/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_I2C_HPP
#define STM32_I2C_HPP

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ranges>
#include <utility>

#include "Config.hpp"
#include "__Internal/__Utility.hpp"

#include "main.h"

#if !defined(HAL_I2C_MODULE_ENABLED) /* module check */
#error "HAL I2C module is not enabled!"
#endif /* module check */

#if !defined(HAL_DMA_MODULE_ENABLED) /* module check */
#error "HAL DMA module is not enabled!"
#endif /* module check */

#if (USE_HAL_I2C_REGISTER_CALLBACKS != 1) /* module check */
#error "HAL I2C callbacks are not enabled!"
#endif /* module check */

namespace STM32 {

/**
 * @enum I2cMemoryAddressSize, Size of memory/register address for I2C memory operations.
 */
enum class I2cMemoryAddressSize {
    Bits8 = I2C_MEMADD_SIZE_8BIT,   /**< 8-bit memory address (most sensors) */
    Bits16 = I2C_MEMADD_SIZE_16BIT  /**< 16-bit memory address (EEPROMs) */
};

/**
 * @struct I2cTimeout, A utility struct to hold the I2C operation timeout value.
 * 
 * @tparam TimeoutV   I2C operation timeout value in milliseconds.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/I2c.hpp>
 *
 * using MyI2cTimeout = STM32::I2cTimeout<200>;
 * auto timeout = MyI2cTimeout::value; // timeout is 200ms.
 * @endcode
 */
template <std::uint32_t TimeoutV>
struct I2cTimeout : __Internal::__Constant<std::uint32_t, TimeoutV> {
    static_assert(
        TimeoutV > 0,
        "Timeout must be greater than zero"
    );
};

/**
 * @brief IsI2cTimeout, A concept to check if a type is a I2cTimeout.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/I2c.hpp>
 * 
 * static_assert(STM32::IsI2cTimeout<STM32::I2cTimeout<100>>);
 * static_assert(!STM32::IsI2cTimeout<int>);
 * @endcode
 */
template <typename T>
concept IsI2cTimeout =
    __Internal::__IsConstant<T> &&
    std::same_as<typename T::ValueTypeT, std::uint32_t> &&
    T::value > 0;

/**
 * @struct I2cDeviceAddress, A compile-time I2C device address.
 * 
 * @tparam DeviceAddressV   7-bit I2C device address (0x00-0x7F).
 *                          This is the address as specified in the device datasheet.
 *                          The left-shift for I2C protocol is handled internally.
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/I2c.hpp>
 * 
 * // BMP280 sensor at address 0x76 (from datasheet)
 * using Bmp280Address = STM32::I2cDeviceAddress<0x76>;
 * 
 * // MPU6050 sensor at address 0x68 (from datasheet)
 * using Mpu6050Address = STM32::I2cDeviceAddress<0x68>;
 * 
 * // Use in I2C operations
 * i2c.Transmit<Mpu6050Address>(tx_data);
 * @endcode
 */
template <std::uint16_t DeviceAddressV>
struct I2cDeviceAddress {
    static_assert(
        DeviceAddressV <= 0x7F,
        "I2C device address must be a 7-bit address (0x00-0x7F)"
    );
    static constexpr std::uint16_t value = DeviceAddressV << 1;
};

/**
 * @concept IsI2cDeviceAddress
 * @brief Checks if a type is a valid I2C device address.
 * 
 * @tparam T    Type to be checked.
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/I2c.hpp>
 * 
 * static_assert(STM32::IsI2cDeviceAddress<STM32::I2cDeviceAddress<0x68>>); // OK
 * static_assert(!STM32::IsI2cDeviceAddress<int>);                          // Fails
 * @endcode
 */
template <typename T>
concept IsI2cDeviceAddress = requires {
    { T::value } -> std::convertible_to<std::uint16_t>;
} && (T::value <= (0x7F << 1));

/**
 * @struct I2cMemoryAddress, A compile-time memory/register address for I2C memory operations.
 * 
 * @tparam MemoryAddressV       Memory or register address value.
 * @tparam MemoryAddressSizeV   Size of the address (I2cMemoryAddressSize::Bits8 or I2cMemoryAddressSize::Bits16).
 * 
 * @note For 8-bit address size, the address must be <= 0xFF.
 *       For 16-bit address size, the address must be <= 0xFFFF.
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/I2c.hpp>
 * 
 * // 8-bit register address (common for sensors)
 * using WhoAmIReg = STM32::I2cMemoryAddress<0x75, STM32::I2cMemoryAddressSize::Bits8>;
 * 
 * // 16-bit memory address (common for EEPROMs)
 * using EepromAddr = STM32::I2cMemoryAddress<0x0100, STM32::I2cMemoryAddressSize::Bits16>;
 * 
 * // Use in I2C operations
 * i2c.MemoryReadTo<Mpu6050Address, WhoAmIReg>(rx_buffer);
 * @endcode
 */
template <std::uint16_t MemoryAddressV, I2cMemoryAddressSize MemoryAddressSizeV>
struct I2cMemoryAddress {
    static constexpr auto address{MemoryAddressV};
    static constexpr auto address_size{MemoryAddressSizeV};
    static_assert(
        (MemoryAddressSizeV == I2cMemoryAddressSize::Bits8 && MemoryAddressV <= 0xFF) ||
        (MemoryAddressSizeV == I2cMemoryAddressSize::Bits16 && MemoryAddressV <= 0xFFFF),
        "Memory address value exceeds the specified address size"
    );
};

/**
 * @concept IsI2cMemoryAddress
 * @brief Checks if a type is a valid I2C memory address.
 * 
 * @tparam T    Type to be checked.
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/I2c.hpp>
 * 
 * using MyReg = STM32::I2cMemoryAddress<0x20, STM32::I2cMemoryAddressSize::Bits8>;
 * static_assert(STM32::IsI2cMemoryAddress<MyReg>);  // OK
 * static_assert(!STM32::IsI2cMemoryAddress<int>);   // Fails
 * @endcode
 */
template <typename T>
concept IsI2cMemoryAddress =
    requires {
        { T::address } -> std::convertible_to<std::uint16_t>;
        { T::address_size } -> std::convertible_to<I2cMemoryAddressSize>;
    } &&
    ((T::address_size == I2cMemoryAddressSize::Bits8 && T::address <= 0xFF) ||
     (T::address_size == I2cMemoryAddressSize::Bits16 && T::address <= 0xFFFF));

/**
 * @struct I2cMaxAttempts, A compile-time maximum attempts value for I2C operations.
 * 
 * @tparam AttemptsV    Maximum number of attempts (must be > 0).
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/I2c.hpp>
 * 
 * // Check device with 5 attempts
 * bool ready = i2c.IsDeviceReady<Bmp280Address, I2cTimeout<100>, I2cMaxAttempts<5>>();
 * @endcode
 */
template <std::size_t AttemptsV>
struct I2cMaxAttempts : __Internal::__Constant<std::size_t, AttemptsV> {
    static_assert(
        AttemptsV > 0,
        "Max attempts must be greater than zero"
    );
};

/**
 * @concept IsI2cMaxAttempts
 * @brief Checks if a type is a valid I2C max attempts value.
 * 
 * @tparam T    Type to be checked.
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/I2c.hpp>
 * 
 * static_assert(STM32::IsI2cMaxAttempts<STM32::I2cMaxAttempts<3>>); // OK
 * static_assert(!STM32::IsI2cMaxAttempts<int>);                     // Fails
 * @endcode
 */
template <typename T>
concept IsI2cMaxAttempts =
    __Internal::__IsConstant<T> &&
    std::same_as<typename T::ValueTypeT, std::size_t> &&
    T::value > 0;

/**
 * @brief IsI2cMessage, A concept to check if a type is a valid I2C message buffer.
 * 
 * Valid types must be:
 * - Contiguous range (data stored in continuous memory)
 * - Sized range (size() is available)
 * - std::uint8_t value type (I2C operates on bytes)
 * 
 * @tparam T        Type to be checked.
 * 
 * @note Accepts both fixed-size containers (std::array) and dynamic containers
 *       (std::vector). Buffer sizes exceeding 65535 bytes will be silently
 *       clamped to 65535 (std::uint16_t max) during transmission/reception.
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/I2c.hpp>
 * static_assert(STM32::IsI2cMessage<std::array<std::uint8_t, 100>>);  // OK
 * static_assert(STM32::IsI2cMessage<std::vector<std::uint8_t>>);      // OK
 * static_assert(!STM32::IsI2cMessage<std::array<int, 100>>);          // Fails: not uint8_t
 * static_assert(!STM32::IsI2cMessage<int>);                           // Fails: not a range
 * @endcode
 */
template <typename T>
concept IsI2cMessage = __Internal::__IsMessage<T, std::uint8_t>;

/**
 * @class I2c, A class to manage I2C functionality on STM32 microcontrollers.
 * 
 * I2C (Inter-Integrated Circuit) is a synchronous, multi-master, multi-slave,
 * half-duplex serial protocol. This class operates in master mode and supports:
 * - Master transmit (Transmit)
 * - Master receive (ReceiveTo)
 * - Memory/register write (MemoryWrite)
 * - Memory/register read (MemoryReadTo)
 * 
 * @tparam WorkingModeT         Working mode of the I2C
 *                              (WorkingMode::Blocking, WorkingMode::Interrupt, WorkingMode::DMA).
 * @tparam UniqueTagT           Unique tag type to differentiate multiple I2c instances.
 *                              UniqueTagT must be STM32_UNIQUE_TAG.
 *
 * @note I2c class is non-copyable and non-movable.
 * @note This class operates in master mode only.
 * @note Device addresses should be 7-bit left-shifted (or 8-bit with R/W bit cleared).
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/I2c.hpp>
 *
 * I2C_HandleTypeDef hi2c1; // Assume properly initialized by CubeMX
 *
 * // Define device addresses (compile-time, 7-bit address from datasheet)
 * using Mpu6050Address = STM32::I2cDeviceAddress<0x68>;
 * using Bmp280Address = STM32::I2cDeviceAddress<0x76>;
 *
 * // Define register addresses (compile-time)
 * using WhoAmIReg = STM32::I2cMemoryAddress<0x75, STM32::I2cMemoryAddressSize::Bits8>;
 * using ConfigReg = STM32::I2cMemoryAddress<0x1A, STM32::I2cMemoryAddressSize::Bits8>;
 *
 * // Create I2C with DMA as default mode
 * STM32::I2c<STM32::WorkingMode::DMA, STM32_UNIQUE_TAG> i2c{hi2c1};
 *
 * std::array<std::uint8_t, 10> tx_data{0x01, 0x02, 0x03};
 * std::array<std::uint8_t, 10> rx_data{};
 *
 * // 1. Master transmit (e.g., sending configuration to sensor)
 * i2c.Transmit<Mpu6050Address>(tx_data, [](){
 *     // TX complete callback
 * });
 *
 * // 2. Master receive (e.g., reading data from sensor)
 * i2c.ReceiveTo<Mpu6050Address>(rx_data, [](){
 *     // RX complete - process rx_data
 * });
 *
 * // 3. Memory/register write (e.g., writing to sensor register)
 * i2c.MemoryWrite<Mpu6050Address, ConfigReg>(tx_data, [](){
 *     // Memory write complete
 * });
 *
 * // 4. Memory/register read (e.g., reading WHO_AM_I register)
 * i2c.MemoryReadTo<Mpu6050Address, WhoAmIReg>(rx_data, [](){
 *     // Memory read complete - rx_data[0] contains WHO_AM_I value
 * });
 *
 * // 5. Override mode per-operation (blocking transmit)
 * i2c.Transmit<Mpu6050Address, STM32::WorkingMode::Blocking>(tx_data);
 *
 * // 6. Blocking mode with custom timeout
 * i2c.ReceiveTo<Bmp280Address, STM32::WorkingMode::Blocking, STM32::I2cTimeout<500>>(rx_data);
 *
 * // 7. Check if device is ready
 * if (i2c.IsDeviceReady<Mpu6050Address>()) {
 *     // Device is responding
 * }
 * @endcode
 */
template <IsWorkingMode WorkingModeT, __Internal::__IsUniqueTag UniqueTagT>
class I2c {
    using MasterTransmitCompleteCallbackT = __Internal::__CallbackManager<
        I2C_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
        HAL_I2C_RegisterCallback, HAL_I2C_UnRegisterCallback, HAL_I2C_MASTER_TX_COMPLETE_CB_ID
    >;
    using MasterReceiveCompleteCallbackT = __Internal::__CallbackManager<
        I2C_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
        HAL_I2C_RegisterCallback, HAL_I2C_UnRegisterCallback, HAL_I2C_MASTER_RX_COMPLETE_CB_ID
    >;
    using MemoryTransmitCompleteCallbackT = __Internal::__CallbackManager<
        I2C_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
        HAL_I2C_RegisterCallback, HAL_I2C_UnRegisterCallback, HAL_I2C_MEM_TX_COMPLETE_CB_ID
    >;
    using MemoryReceiveCompleteCallbackT = __Internal::__CallbackManager<
        I2C_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
        HAL_I2C_RegisterCallback, HAL_I2C_UnRegisterCallback, HAL_I2C_MEM_RX_COMPLETE_CB_ID
    >;
public:

    /**
     * @brief Construct I2c class.
     * 
     * @param handle        Reference to the I2C handle.
     * 
     * @note HAL callbacks are automatically registered via RAII.
     */
    explicit I2c(I2C_HandleTypeDef& handle) noexcept
      : m_handle{handle},
        m_master_transmit_complete_callback{handle},
        m_master_receive_complete_callback{handle},
        m_memory_transmit_complete_callback{handle},
        m_memory_receive_complete_callback{handle}
    { }

    /**
     * @defgroup Deleted copy and move members.
     * @{
     */
    I2c(const I2c&) = delete;
    I2c& operator=(const I2c&) = delete;
    I2c(I2c&&) = delete;
    I2c& operator=(I2c&&) = delete;
    /** @} */

    /**
     * @brief Destroy I2c class.
     * 
     * @note Callbacks are automatically unregistered via RAII.
     */
    ~I2c() = default;

    /**
     * @returns I2C handle reference.
     */
    [[nodiscard]]
    auto&& GetHandle(this auto&& self) noexcept
    {
        return std::forward<decltype(self)>(self).m_handle;
    }

    /* ==================== Master Receive Operations ==================== */

    /**
     * @brief Receive data from a slave device in blocking mode.
     * 
     * @tparam DeviceAddressT   I2C device address (must satisfy IsI2cDeviceAddress).
     * @tparam RxWorkingModeT   Working mode for receiving (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param rx_message        A contiguous range to store the received data.
     * 
     * @returns True on success, false otherwise.
     *
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsI2cDeviceAddress DeviceAddressT,
        IsWorkingMode RxWorkingModeT = WorkingModeT,
        IsI2cTimeout TimeoutV = I2cTimeout<100>
    >
    bool ReceiveTo(
        IsI2cMessage auto& rx_message
    ) noexcept
    requires std::same_as<RxWorkingModeT, WorkingMode::Blocking>
    {
        return (HAL_OK == HAL_I2C_Master_Receive(
            &m_handle,
            DeviceAddressT::value,
            std::ranges::data(rx_message),
            __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message)),
            TimeoutV::value
        ));
    }

    /**
     * @brief Receive data from a slave device in non-blocking mode.
     * 
     * @tparam DeviceAddressT   I2C device address (must satisfy IsI2cDeviceAddress).
     * @tparam RxWorkingModeT   Working mode for receiving (default is WorkingModeT).
     * 
     * @param rx_message        A contiguous range to store the received data.
     * @param complete_callback Callback function to be called upon completion.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsI2cDeviceAddress DeviceAddressT,
        IsWorkingMode RxWorkingModeT = WorkingModeT
    >
    bool ReceiveTo(
        IsI2cMessage auto& rx_message,
        CallbackT&& complete_callback = [](){}
    ) noexcept
    requires (!std::same_as<RxWorkingModeT, WorkingMode::Blocking>)
    {
        m_master_receive_complete_callback.Set(
            std::move(complete_callback)
        );
        if constexpr (std::same_as<RxWorkingModeT, WorkingMode::Interrupt>) {
            return (HAL_OK == HAL_I2C_Master_Receive_IT(
                &m_handle,
                DeviceAddressT::value,
                std::ranges::data(rx_message),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message))
            ));
        } else if constexpr (std::same_as<RxWorkingModeT, WorkingMode::DMA>) {
            return (HAL_OK == HAL_I2C_Master_Receive_DMA(
                &m_handle,
                DeviceAddressT::value,
                std::ranges::data(rx_message),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message))
            ));
        }
    }

    /* ==================== Master Transmit Operations ==================== */

    /**
     * @brief Transmit data to a slave device in blocking mode.
     * 
     * @tparam DeviceAddressT   I2C device address (must satisfy IsI2cDeviceAddress).
     * @tparam TxWorkingModeT   Working mode for transmitting (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param tx_message        A contiguous range containing the data to transmit.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsI2cDeviceAddress DeviceAddressT,
        IsWorkingMode TxWorkingModeT = WorkingModeT,
        IsI2cTimeout TimeoutV = I2cTimeout<100>
    >
    bool Transmit(
        const IsI2cMessage auto& tx_message
    ) noexcept
    requires std::same_as<TxWorkingModeT, WorkingMode::Blocking>
    {
        return (HAL_OK == HAL_I2C_Master_Transmit(
            &m_handle,
            DeviceAddressT::value,
            const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
            __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message)),
            TimeoutV::value
        ));
    }

    /**
     * @brief Transmit data to a slave device in non-blocking mode.
     * 
     * @tparam DeviceAddressT    I2C device address (must satisfy IsI2cDeviceAddress).
     * @tparam TxWorkingModeT    Working mode for transmitting (default is WorkingModeT).
     * 
     * @param tx_message         A contiguous range containing the data to transmit.
     * @param complete_callback  Callback function to be called upon completion.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsI2cDeviceAddress DeviceAddressT,
        IsWorkingMode TxWorkingModeT = WorkingModeT
    >
    bool Transmit(
        const IsI2cMessage auto& tx_message,
        CallbackT&& complete_callback = [](){}
    ) noexcept
    requires (!std::same_as<TxWorkingModeT, WorkingMode::Blocking>)
    {
        m_master_transmit_complete_callback.Set(
            std::move(complete_callback)
        );
        if constexpr (std::same_as<TxWorkingModeT, WorkingMode::Interrupt>) {
            return (HAL_OK == HAL_I2C_Master_Transmit_IT(
                &m_handle,
                DeviceAddressT::value,
                const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message))
            ));
        } else if constexpr (std::same_as<TxWorkingModeT, WorkingMode::DMA>) {
            return (HAL_OK == HAL_I2C_Master_Transmit_DMA(
                &m_handle,
                DeviceAddressT::value,
                const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message))
            ));
        }
    }

    /* ==================== Memory Read Operations ==================== */

    /**
     * @brief Read data from a memory/register address on a slave device in blocking mode.
     * 
     * @tparam DeviceAddressT   I2C device address (must satisfy IsI2cDeviceAddress).
     * @tparam MemoryAddressT   Memory/register address (must satisfy IsI2cMemoryAddress).
     * @tparam RxWorkingModeT   Working mode for reading (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param rx_message        A contiguous range to store the received data.
     * 
     * @returns True on success, false otherwise.
     *
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsI2cDeviceAddress DeviceAddressT,
        IsI2cMemoryAddress MemoryAddressT,
        IsWorkingMode RxWorkingModeT = WorkingModeT,
        IsI2cTimeout TimeoutV = I2cTimeout<100>
    >
    bool MemoryReadTo(
        IsI2cMessage auto& rx_message
    ) noexcept
    requires std::same_as<RxWorkingModeT, WorkingMode::Blocking>
    {
        return (HAL_OK == HAL_I2C_Mem_Read(
            &m_handle,
            DeviceAddressT::value,
            MemoryAddressT::address,
            std::to_underlying(MemoryAddressT::address_size),
            std::ranges::data(rx_message),
            __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message)),
            TimeoutV::value
        ));
    }

    /**
     * @brief Read data from a memory/register address on a slave device in non-blocking mode.
     * 
     * @tparam DeviceAddressT   I2C device address (must satisfy IsI2cDeviceAddress).
     * @tparam MemoryAddressT   Memory/register address (must satisfy IsI2cMemoryAddress).
     * @tparam RxWorkingModeT   Working mode for reading (default is WorkingModeT).
     * 
     * @param rx_message        A contiguous range to store the received data.
     * @param complete_callback Callback function to be called upon completion.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsI2cDeviceAddress DeviceAddressT,
        IsI2cMemoryAddress MemoryAddressT,
        IsWorkingMode RxWorkingModeT = WorkingModeT
    >
    bool MemoryReadTo(
        IsI2cMessage auto& rx_message,
        CallbackT&& complete_callback = [](){}
    ) noexcept
    requires (!std::same_as<RxWorkingModeT, WorkingMode::Blocking>)
    {
        m_memory_receive_complete_callback.Set(
            std::move(complete_callback)
        );
        if constexpr (std::same_as<RxWorkingModeT, WorkingMode::Interrupt>) {
            return (HAL_OK == HAL_I2C_Mem_Read_IT(
                &m_handle,
                DeviceAddressT::value,
                MemoryAddressT::address,
                std::to_underlying(MemoryAddressT::address_size),
                std::ranges::data(rx_message),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message))
            ));
        } else if constexpr (std::same_as<RxWorkingModeT, WorkingMode::DMA>) {
            return (HAL_OK == HAL_I2C_Mem_Read_DMA(
                &m_handle,
                DeviceAddressT::value,
                MemoryAddressT::address,
                std::to_underlying(MemoryAddressT::address_size),
                std::ranges::data(rx_message),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message))
            ));
        }
    }

    /* ==================== Memory Write Operations ==================== */

    /**
     * @brief Write data to a memory/register address on a slave device in blocking mode.
     * 
     * @tparam DeviceAddressT   I2C device address (must satisfy IsI2cDeviceAddress).
     * @tparam MemoryAddressT   Memory/register address (must satisfy IsI2cMemoryAddress).
     * @tparam TxWorkingModeT   Working mode for writing (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param tx_message        A contiguous range containing the data to write.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsI2cDeviceAddress DeviceAddressT,
        IsI2cMemoryAddress MemoryAddressT,
        IsWorkingMode TxWorkingModeT = WorkingModeT,
        IsI2cTimeout TimeoutV = I2cTimeout<100>
    >
    bool MemoryWrite(
        const IsI2cMessage auto& tx_message
    ) noexcept
    requires std::same_as<TxWorkingModeT, WorkingMode::Blocking>
    {
        return (HAL_OK == HAL_I2C_Mem_Write(
            &m_handle,
            DeviceAddressT::value,
            MemoryAddressT::address,
            std::to_underlying(MemoryAddressT::address_size),
            const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
            __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message)),
            TimeoutV::value
        ));
    }

    /**
     * @brief Write data to a memory/register address on a slave device in non-blocking mode.
     * 
     * @tparam DeviceAddressT    I2C device address (must satisfy IsI2cDeviceAddress).
     * @tparam MemoryAddressT    Memory/register address (must satisfy IsI2cMemoryAddress).
     * @tparam TxWorkingModeT    Working mode for writing (default is WorkingModeT).
     * 
     * @param tx_message         A contiguous range containing the data to write.
     * @param complete_callback  Callback function to be called upon completion.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsI2cDeviceAddress DeviceAddressT,
        IsI2cMemoryAddress MemoryAddressT,
        IsWorkingMode TxWorkingModeT = WorkingModeT
    >
    bool MemoryWrite(
        const IsI2cMessage auto& tx_message,
        CallbackT&& complete_callback = [](){}
    ) noexcept
    requires (!std::same_as<TxWorkingModeT, WorkingMode::Blocking>)
    {
        m_memory_transmit_complete_callback.Set(
            std::move(complete_callback)
        );
        if constexpr (std::same_as<TxWorkingModeT, WorkingMode::Interrupt>) {
            return (HAL_OK == HAL_I2C_Mem_Write_IT(
                &m_handle,
                DeviceAddressT::value,
                MemoryAddressT::address,
                std::to_underlying(MemoryAddressT::address_size),
                const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message))
            ));
        } else if constexpr (std::same_as<TxWorkingModeT, WorkingMode::DMA>) {
            return (HAL_OK == HAL_I2C_Mem_Write_DMA(
                &m_handle,
                DeviceAddressT::value,
                MemoryAddressT::address,
                std::to_underlying(MemoryAddressT::address_size),
                const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message))
            ));
        }
    }

    /* ==================== Utility Operations ==================== */

    /**
     * @brief Check if a device is ready on the I2C bus.
     * 
     * @tparam DeviceAddressT   I2C device address (must satisfy IsI2cDeviceAddress).
     * @tparam TimeoutT         Timeout for each attempt (default is 100ms).
     * @tparam MaxAttemptsT     Maximum number of attempts (default is 3).
     * 
     * @returns True if device is ready (ACK received), false otherwise.
     * 
     * @note This function can be used to scan the I2C bus for connected devices.
     */
    template <
        IsI2cDeviceAddress DeviceAddressT,
        IsI2cTimeout TimeoutT = I2cTimeout<100>,
        IsI2cMaxAttempts MaxAttemptsT = I2cMaxAttempts<3>
    >
    [[nodiscard]]
    bool IsDeviceReady() noexcept
    {
        return (HAL_OK == HAL_I2C_IsDeviceReady(
            &m_handle,
            DeviceAddressT::value,
            MaxAttemptsT::value,
            TimeoutT::value
        ));
    }

private:
    I2C_HandleTypeDef& m_handle;
    MasterTransmitCompleteCallbackT m_master_transmit_complete_callback;
    MasterReceiveCompleteCallbackT m_master_receive_complete_callback;
    MemoryTransmitCompleteCallbackT m_memory_transmit_complete_callback;
    MemoryReceiveCompleteCallbackT m_memory_receive_complete_callback;
};

} /* namespace STM32 */

#endif /* STM32_I2C_HPP */
