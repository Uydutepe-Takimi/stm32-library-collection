/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_CRC16_HPP
#define STM32_CRC16_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <ranges>
#include <span>

namespace STM32 {

/**
 * @struct Crc16Polynomial, A compile-time CRC-16 polynomial value.
 * 
 * @tparam PolynomialV  The polynomial value used for CRC calculation.
 * 
 * @note Common polynomials:
 *       - 0x1021: CRC-16-CCITT (Bluetooth, X.25, XMODEM)
 *       - 0x8005: CRC-16-IBM/ANSI (Modbus, USB)
 *       - 0x3D65: CRC-16-DNP
 * 
 * @example Usage:
 * @code {.cpp}
 * using CcittPoly = STM32::Crc16Polynomial<0x1021>;
 * using ModbusPoly = STM32::Crc16Polynomial<0x8005>;
 * @endcode
 */
template <std::uint16_t PolynomialV>
struct Crc16Polynomial {
    static constexpr std::uint16_t value = PolynomialV;
};

/**
 * @struct Crc16InitialValue, A compile-time CRC-16 initial value.
 * 
 * @tparam InitialValueV    The initial CRC value before processing data.
 * 
 * @note Common initial values:
 *       - 0xFFFF: Most common (CCITT-FALSE, X-25)
 *       - 0x0000: XMODEM, KERMIT
 * 
 * @example Usage:
 * @code {.cpp}
 * using InitFFFF = STM32::Crc16InitialValue<0xFFFF>;
 * using Init0000 = STM32::Crc16InitialValue<0x0000>;
 * @endcode
 */
template <std::uint16_t InitialValueV>
struct Crc16InitialValue {
    static constexpr std::uint16_t value = InitialValueV;
};

/**
 * @struct Crc16FinalXor, A compile-time CRC-16 final XOR value.
 * 
 * @tparam FinalXorV    The value to XOR with the final CRC result.
 * 
 * @note Common final XOR values:
 *       - 0x0000: No XOR (CCITT-FALSE, XMODEM)
 *       - 0xFFFF: X-25, CRC-16-IBM
 * 
 * @example Usage:
 * @code {.cpp}
 * using NoFinalXor = STM32::Crc16FinalXor<0x0000>;
 * using XorFFFF = STM32::Crc16FinalXor<0xFFFF>;
 * @endcode
 */
template <std::uint16_t FinalXorV>
struct Crc16FinalXor {
    static constexpr std::uint16_t value = FinalXorV;
};

/**
 * @struct Crc16ReflectInput, A compile-time flag for input byte reflection.
 * 
 * @tparam ReflectV     Whether to bit-reverse each input byte before processing.
 * 
 * @note Reflection means bit-reversing: 0b10110000 -> 0b00001101
 *       - false: CCITT-FALSE, XMODEM (no reflection)
 *       - true: KERMIT, X-25, Modbus (with reflection)
 * 
 * @example Usage:
 * @code {.cpp}
 * using NoReflect = STM32::Crc16ReflectInput<false>;
 * using Reflect = STM32::Crc16ReflectInput<true>;
 * @endcode
 */
template <bool ReflectV>
struct Crc16ReflectInput {
    static constexpr bool value = ReflectV;
};

/**
 * @struct Crc16ReflectOutput, A compile-time flag for output CRC reflection.
 * 
 * @tparam ReflectV     Whether to bit-reverse the final CRC result.
 * 
 * @note Usually matches Crc16ReflectInput setting.
 *       - false: CCITT-FALSE, XMODEM
 *       - true: KERMIT, X-25, Modbus
 * 
 * @example Usage:
 * @code {.cpp}
 * using NoReflect = STM32::Crc16ReflectOutput<false>;
 * using Reflect = STM32::Crc16ReflectOutput<true>;
 * @endcode
 */
template <bool ReflectV>
struct Crc16ReflectOutput {
    static constexpr bool value = ReflectV;
};

namespace __Internal {

/**
 * @brief Reflect (bit-reverse) an 8-bit value.
 */
[[nodiscard]]
constexpr std::uint8_t __Reflect8(std::uint8_t value) noexcept
{
    value = static_cast<std::uint8_t>(((value & 0x55) << 1) | ((value & 0xAA) >> 1));
    value = static_cast<std::uint8_t>(((value & 0x33) << 2) | ((value & 0xCC) >> 2));
    return static_cast<std::uint8_t>((value << 4) | (value >> 4));
}

/**
 * @brief Reflect (bit-reverse) a 16-bit value.
 */
[[nodiscard]]
constexpr std::uint16_t __Reflect16(std::uint16_t value) noexcept
{
    value = static_cast<std::uint16_t>(((value & 0x5555) << 1) | ((value & 0xAAAA) >> 1));
    value = static_cast<std::uint16_t>(((value & 0x3333) << 2) | ((value & 0xCCCC) >> 2));
    value = static_cast<std::uint16_t>(((value & 0x0F0F) << 4) | ((value & 0xF0F0) >> 4));
    return static_cast<std::uint16_t>((value << 8) | (value >> 8));
}

/**
 * @brief Generate CRC-16 lookup table at compile time.
 * 
 * @tparam PolynomialV      The CRC polynomial.
 * @tparam ReflectInputV    Whether input reflection is enabled.
 */
template <std::uint16_t PolynomialV, bool ReflectInputV>
[[nodiscard]]
consteval std::array<std::uint16_t, 256> __GenerateCrc16Table() noexcept
{
    std::array<std::uint16_t, 256> table{};
    
    for (std::size_t i = 0; i < 256; ++i) {
        std::uint16_t crc{};
        
        if constexpr (ReflectInputV) {
            crc = static_cast<std::uint16_t>(i);
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x0001) {
                    crc = static_cast<std::uint16_t>((crc >> 1) ^ __Reflect16(PolynomialV));
                } else {
                    crc >>= 1;
                }
            }
        } else {
            crc = static_cast<std::uint16_t>(i << 8);
            for (int j = 0; j < 8; ++j) {
                if (crc & 0x8000) {
                    crc = static_cast<std::uint16_t>((crc << 1) ^ PolynomialV);
                } else {
                    crc <<= 1;
                }
            }
        }
        
        table[i] = crc;
    }
    
    return table;
}

} /* namespace __Internal */

/**
 * @concept IsCrc16Polynomial
 * @brief Checks if a type is a valid CRC-16 polynomial.
 */
template <typename T>
concept IsCrc16Polynomial = requires {
    { T::value } -> std::convertible_to<std::uint16_t>;
};

/**
 * @concept IsCrc16InitialValue
 * @brief Checks if a type is a valid CRC-16 initial value.
 */
template <typename T>
concept IsCrc16InitialValue = requires {
    { T::value } -> std::convertible_to<std::uint16_t>;
};

/**
 * @concept IsCrc16FinalXor
 * @brief Checks if a type is a valid CRC-16 final XOR value.
 */
template <typename T>
concept IsCrc16FinalXor = requires {
    { T::value } -> std::convertible_to<std::uint16_t>;
};

/**
 * @concept IsCrc16ReflectInput
 * @brief Checks if a type is a valid CRC-16 input reflection flag.
 */
template <typename T>
concept IsCrc16ReflectInput = requires {
    { T::value } -> std::convertible_to<bool>;
};

/**
 * @concept IsCrc16ReflectOutput
 * @brief Checks if a type is a valid CRC-16 output reflection flag.
 */
template <typename T>
concept IsCrc16ReflectOutput = requires {
    { T::value } -> std::convertible_to<bool>;
};

/**
 * @concept IsCrc16Data
 * @brief Checks if a type is a valid data source for CRC calculation.
 */
template <typename T>
concept IsCrc16Data = 
    std::ranges::contiguous_range<T> &&
    std::ranges::sized_range<T> &&
    std::same_as<std::ranges::range_value_t<T>, std::uint8_t>;

/**
 * @class Crc16, A compile-time configurable CRC-16 calculator.
 * 
 * All CRC parameters are template arguments for self-documentation and
 * compile-time validation. Uses an optimized lookup table generated at
 * compile time for fast byte-at-a-time processing.
 * 
 * @tparam PolynomialT      CRC polynomial (e.g., Crc16Polynomial<0x1021>).
 * @tparam InitialValueT    Initial CRC value (e.g., Crc16InitialValue<0xFFFF>).
 * @tparam FinalXorT        Final XOR value (e.g., Crc16FinalXor<0x0000>).
 * @tparam ReflectInputT    Input reflection flag (e.g., Crc16ReflectInput<false>).
 * @tparam ReflectOutputT   Output reflection flag (e.g., Crc16ReflectOutput<false>).
 * 
 * @note The lookup table is generated at compile time (consteval).
 * @note All Calculate methods are constexpr and can be evaluated at compile time.
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Crc16.hpp>
 * 
 * // Define CRC-16-CCITT-FALSE configuration
 * using MyCrc = STM32::Crc16<
 *     STM32::Crc16Polynomial<0x1021>,
 *     STM32::Crc16InitialValue<0xFFFF>,
 *     STM32::Crc16FinalXor<0x0000>,
 *     STM32::Crc16ReflectInput<false>,
 *     STM32::Crc16ReflectOutput<false>
 * >;
 * 
 * // Or use a predefined alias
 * using MyPredefinedCrc = STM32::Crc16CcittFalse;
 * 
 * std::array<std::uint8_t, 10> data{0x01, 0x02, 0x03};
 * auto crc = MyCrc::Calculate(data);
 * 
 * // Compile-time calculation
 * constexpr std::array<std::uint8_t, 4> test_data{0x31, 0x32, 0x33, 0x34};
 * constexpr auto compile_time_crc = MyCrc::Calculate(test_data);
 * @endcode
 */
template <
    IsCrc16Polynomial PolynomialT,
    IsCrc16InitialValue InitialValueT,
    IsCrc16FinalXor FinalXorT,
    IsCrc16ReflectInput ReflectInputT,
    IsCrc16ReflectOutput ReflectOutputT
>
class Crc16 {
public:
    /** @brief The CRC polynomial used. */
    static constexpr std::uint16_t polynomial = PolynomialT::value;
    
    /** @brief The initial CRC value. */
    static constexpr std::uint16_t initial_value = InitialValueT::value;
    
    /** @brief The final XOR value. */
    static constexpr std::uint16_t final_xor = FinalXorT::value;
    
    /** @brief Whether input bytes are reflected. */
    static constexpr bool reflect_input = ReflectInputT::value;
    
    /** @brief Whether output CRC is reflected. */
    static constexpr bool reflect_output = ReflectOutputT::value;

    /**
     * @brief Calculate CRC-16 of a contiguous byte range.
     * 
     * @param data  A contiguous range of bytes (std::array, std::vector, std::span, etc.).
     * 
     * @returns The calculated CRC-16 value.
     */
    [[nodiscard]]
    static constexpr std::uint16_t Calculate(IsCrc16Data auto const& data) noexcept
    {
        return Calculate(std::ranges::data(data), std::ranges::size(data));
    }

    /**
     * @brief Calculate CRC-16 of a byte array.
     * 
     * @param data      Pointer to the data bytes.
     * @param length    Number of bytes to process.
     * 
     * @returns The calculated CRC-16 value.
     */
    [[nodiscard]]
    static constexpr std::uint16_t Calculate(
        const std::uint8_t* data,
        std::size_t length
    ) noexcept
    {
        std::uint16_t crc = initial_value;
        
        if constexpr (reflect_input) {
            // Reflected algorithm (LSB first)
            for (std::size_t i = 0; i < length; ++i) {
                std::uint8_t index = static_cast<std::uint8_t>(crc ^ data[i]);
                crc = static_cast<std::uint16_t>((crc >> 8) ^ s_table[index]);
            }
        } else {
            // Non-reflected algorithm (MSB first)
            for (std::size_t i = 0; i < length; ++i) {
                std::uint8_t index = static_cast<std::uint8_t>((crc >> 8) ^ data[i]);
                crc = static_cast<std::uint16_t>((crc << 8) ^ s_table[index]);
            }
        }
        
        // Apply output reflection if different from input reflection
        if constexpr (reflect_output != reflect_input) {
            crc = __Internal::__Reflect16(crc);
        }
        
        return crc ^ final_xor;
    }

    /**
     * @brief Update an existing CRC with additional data (for streaming).
     * 
     * @param crc       The current CRC value to update.
     * @param data      A contiguous range of bytes to process.
     * 
     * @returns The updated CRC-16 value.
     * 
     * @note To finalize, call Finalize() with the updated CRC value.
     */
    [[nodiscard]]
    static constexpr std::uint16_t Update(
        std::uint16_t crc,
        IsCrc16Data auto const& data
    ) noexcept
    {
        if constexpr (reflect_input) {
            for (const auto byte : data) {
                std::uint8_t index = static_cast<std::uint8_t>(crc ^ byte);
                crc = static_cast<std::uint16_t>((crc >> 8) ^ s_table[index]);
            }
        } else {
            for (const auto byte : data) {
                std::uint8_t index = static_cast<std::uint8_t>((crc >> 8) ^ byte);
                crc = static_cast<std::uint16_t>((crc << 8) ^ s_table[index]);
            }
        }
        return crc;
    }

    /**
     * @brief Finalize a streaming CRC calculation.
     * 
     * @param crc   The CRC value after all Update() calls.
     * 
     * @returns The finalized CRC-16 value.
     */
    [[nodiscard]]
    static constexpr std::uint16_t Finalize(std::uint16_t crc) noexcept
    {
        if constexpr (reflect_output != reflect_input) {
            crc = __Internal::__Reflect16(crc);
        }
        return crc ^ final_xor;
    }

    /**
     * @brief Get the initial value for streaming calculations.
     * 
     * @returns The initial CRC value.
     */
    [[nodiscard]]
    static constexpr std::uint16_t Init() noexcept
    {
        return initial_value;
    }

    /**
     * @brief Access the precomputed lookup table.
     * 
     * @returns Reference to the 256-entry lookup table.
     */
    [[nodiscard]]
    static constexpr const std::array<std::uint16_t, 256>& Table() noexcept
    {
        return s_table;
    }

private:
    /** @brief Compile-time generated lookup table. */
    static constexpr std::array<std::uint16_t, 256> s_table = 
        __Internal::__GenerateCrc16Table<polynomial, reflect_input>();
};

/* ==================== Predefined CRC-16 Variants ==================== */

/**
 * @brief CRC-16/CCITT-FALSE - Most common variant.
 * 
 * Used in: Bluetooth, many embedded systems.
 * Poly: 0x1021, Init: 0xFFFF, No reflection, No final XOR.
 */
using Crc16CcittFalse = Crc16<
    Crc16Polynomial<0x1021>,
    Crc16InitialValue<0xFFFF>,
    Crc16FinalXor<0x0000>,
    Crc16ReflectInput<false>,
    Crc16ReflectOutput<false>
>;

/**
 * @brief CRC-16/XMODEM - Used in XMODEM protocol.
 * 
 * Poly: 0x1021, Init: 0x0000, No reflection, No final XOR.
 */
using Crc16Xmodem = Crc16<
    Crc16Polynomial<0x1021>,
    Crc16InitialValue<0x0000>,
    Crc16FinalXor<0x0000>,
    Crc16ReflectInput<false>,
    Crc16ReflectOutput<false>
>;

/**
 * @brief CRC-16/KERMIT - Used in Kermit protocol.
 * 
 * Also known as CRC-16/CCITT-TRUE.
 * Poly: 0x1021, Init: 0x0000, With reflection, No final XOR.
 */
using Crc16Kermit = Crc16<
    Crc16Polynomial<0x1021>,
    Crc16InitialValue<0x0000>,
    Crc16FinalXor<0x0000>,
    Crc16ReflectInput<true>,
    Crc16ReflectOutput<true>
>;

/**
 * @brief CRC-16/X-25 - Used in X.25, HDLC, SDLC.
 * 
 * Poly: 0x1021, Init: 0xFFFF, With reflection, Final XOR 0xFFFF.
 */
using Crc16X25 = Crc16<
    Crc16Polynomial<0x1021>,
    Crc16InitialValue<0xFFFF>,
    Crc16FinalXor<0xFFFF>,
    Crc16ReflectInput<true>,
    Crc16ReflectOutput<true>
>;

/**
 * @brief CRC-16/MODBUS - Used in Modbus RTU protocol.
 * 
 * Poly: 0x8005, Init: 0xFFFF, With reflection, No final XOR.
 */
using Crc16Modbus = Crc16<
    Crc16Polynomial<0x8005>,
    Crc16InitialValue<0xFFFF>,
    Crc16FinalXor<0x0000>,
    Crc16ReflectInput<true>,
    Crc16ReflectOutput<true>
>;

/**
 * @brief CRC-16/USB - Used in USB protocol.
 * 
 * Poly: 0x8005, Init: 0xFFFF, With reflection, Final XOR 0xFFFF.
 */
using Crc16Usb = Crc16<
    Crc16Polynomial<0x8005>,
    Crc16InitialValue<0xFFFF>,
    Crc16FinalXor<0xFFFF>,
    Crc16ReflectInput<true>,
    Crc16ReflectOutput<true>
>;

/**
 * @brief CRC-16/IBM - Also known as CRC-16/ARC, CRC-16/LHA.
 * 
 * Poly: 0x8005, Init: 0x0000, With reflection, No final XOR.
 */
using Crc16Ibm = Crc16<
    Crc16Polynomial<0x8005>,
    Crc16InitialValue<0x0000>,
    Crc16FinalXor<0x0000>,
    Crc16ReflectInput<true>,
    Crc16ReflectOutput<true>
>;

/**
 * @brief CRC-16/DNP - Used in DNP3 (Distributed Network Protocol).
 * 
 * Poly: 0x3D65, Init: 0x0000, With reflection, Final XOR 0xFFFF.
 */
using Crc16Dnp = Crc16<
    Crc16Polynomial<0x3D65>,
    Crc16InitialValue<0x0000>,
    Crc16FinalXor<0xFFFF>,
    Crc16ReflectInput<true>,
    Crc16ReflectOutput<true>
>;

} /* namespace STM32 */

#endif /* STM32_CRC16_HPP */
