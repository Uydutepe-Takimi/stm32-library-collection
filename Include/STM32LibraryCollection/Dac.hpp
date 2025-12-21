/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_DAC_HPP
#define STM32_DAC_HPP

#include <algorithm>
#include <cstdint>
#include <utility>

#include "Config.hpp"
#include "__Internal/__Utility.hpp"

#include "main.h"

#if !defined(HAL_DAC_MODULE_ENABLED) /* module check */
#error "HAL DAC module is not enabled!"
#endif /* module check */

namespace STM32 {

/**
 * @namespace DacAlignment, Tag types for DAC data alignment configuration.
 * 
 * These tags specify how the DAC data is aligned in the data register.
 * The alignment affects both the resolution and how data is written.
 */
namespace DacAlignment {

/**
 * @struct Align12BRight, Tag for 12-bit right-aligned data.
 * 
 * Data bits occupy positions 0-11. Most common alignment for 12-bit DAC.
 * Output values range from 0 to 4095.
 */
struct Align12BRight {
    static constexpr std::uint32_t alignment{DAC_ALIGN_12B_R};
    static constexpr double resolution{4095.0};
};

/**
 * @struct Align12BLeft, Tag for 12-bit left-aligned data.
 * 
 * Data bits occupy positions 4-15. Useful for certain audio applications.
 * Output values range from 0 to 4095.
 */
struct Align12BLeft {
    static constexpr std::uint32_t alignment{DAC_ALIGN_12B_L};
    static constexpr double resolution{4095.0};
};

/**
 * @struct Align8BRight, Tag for 8-bit right-aligned data.
 * 
 * Data bits occupy positions 0-7. Lower resolution but faster.
 * Output values range from 0 to 255.
 */
struct Align8BRight {
    static constexpr std::uint32_t alignment{DAC_ALIGN_8B_R};
    static constexpr double resolution{255.0};
};

} /* namespace DacAlignment */

/**
 * @enum DacChannel, Enumeration for DAC output channels.
 */
enum class DacChannel : std::uint32_t {
    Channel1 = DAC_CHANNEL_1,
#if defined(DAC_CHANNEL2_SUPPORT)
    Channel2 = DAC_CHANNEL_2,
#endif /* DAC_CHANNEL2_SUPPORT */
};

/**
 * @brief IsDacAlignment, A concept to check if a type is a DacAlignment.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Dac.hpp>
 * 
 * static_assert(STM32::IsDacAlignment<STM32::DacAlignment::Align12BRight>);
 * static_assert(!STM32::IsDacAlignment<int>);
 * @endcode
 */
template <typename T>
concept IsDacAlignment = 
    std::same_as<std::remove_cv_t<decltype(T::alignment)>, std::uint32_t> &&
    std::same_as<std::remove_cv_t<decltype(T::resolution)>, double> &&
    requires {
        T::alignment;
        T::resolution;
    };

/**
 * @struct DacInputMax, A utility struct to define the input value range for DAC.
 * 
 * @tparam MaxV     Maximum input value. Set() accepts values in range [0, MaxV].
 *
 * The input value is linearly mapped from this range to the DAC resolution.
 * This allows you to work with meaningful units instead of raw DAC values.
 *
 * @example Usage:
 * @code {.cpp}
 * // Map 0-100 (percentage) to DAC output
 * using PercentInput = STM32::DacInputMax<100>;
 * 
 * // Map 0-255 (8-bit) to DAC output
 * using ByteInput = STM32::DacInputMax<255>;
 * @endcode
 */
template <std::uint32_t MaxV>
struct DacInputMax : __Internal::__Range<double, 0., static_cast<double>(MaxV)> {};

/**
 * @brief IsDacInputMax, A concept to check if a type is a DacInputMax.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Dac.hpp>
 * 
 * static_assert(STM32::IsDacInputMax<STM32::DacInputMax<100>>);
 * static_assert(!STM32::IsDacInputMax<int>);
 * @endcode
 */
template <typename T>
concept IsDacInputMax =
    __Internal::__IsRange<T> &&
    std::same_as<typename T::ValueTypeT, double>;

/**
 * @struct DacConfig, A utility struct to bundle DAC configuration parameters.
 * 
 * @tparam DacInputMaxT    Input range configuration (0 to MaxV).
 * @tparam DacAlignmentT   Data alignment (8-bit or 12-bit, left or right).
 *
 * @example Usage:
 * @code {.cpp}
 * // 12-bit right-aligned with 0-100 input range
 * using MyDacConfig = STM32::DacConfig<
 *     STM32::DacInputMax<100>,
 *     STM32::DacAlignment::Align12BRight
 * >;
 * @endcode
 */
template <IsDacInputMax DacInputMaxT, IsDacAlignment DacAlignmentT>
struct DacConfig : DacInputMaxT, DacAlignmentT {
    using InputRangeT = DacInputMaxT;
    using AlignmentT = DacAlignmentT;
};

/**
 * @brief IsDacConfig, A concept to check if a type is a DacConfig.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Dac.hpp>
 * 
 * static_assert(STM32::IsDacConfig<STM32::DacConfig<
 *     STM32::DacInputMax<100>,
 *     STM32::DacAlignment::Align12BRight
 * >>);
 * static_assert(!STM32::IsDacConfig<int>);
 * @endcode
 */
template <typename T>
concept IsDacConfig =
    IsDacInputMax<typename T::InputRangeT> &&
    IsDacAlignment<typename T::AlignmentT> &&
    requires {
        typename T::InputRangeT;
        typename T::AlignmentT;
        T::InputRangeT::max_value;
        T::AlignmentT::resolution;
    };

/**
 * @class Dac, A class to manage DAC functionality on STM32 microcontrollers.
 *
 * @tparam DacChannelV  DAC channel to use (Channel1 or Channel2).
 * @tparam DacConfigT   DAC configuration type (defaults to 0-100 input, 12-bit right-aligned).
 *
 * @note Dac class is non-copyable and non-movable.
 * @note DAC is started automatically upon construction and stopped on destruction.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Dac.hpp>
 *
 * DAC_HandleTypeDef hdac; // Assume properly initialized by CubeMX
 *
 * // 1. Use default configuration (0-100 input, 12-bit right-aligned)
 * STM32::Dac<STM32::DacChannel::Channel1> dac1{hdac};
 * dac1.Set(50);  // Set to 50% output voltage
 *
 * // 2. Custom configuration for LED brightness (0-255 input)
 * using LedDacConfig = STM32::DacConfig<
 *     STM32::DacInputMax<255>,
 *     STM32::DacAlignment::Align12BRight
 * >;
 * STM32::Dac<STM32::DacChannel::Channel1, LedDacConfig> led{hdac};
 * led.Set(128);  // Set to ~50% brightness
 *
 * // 3. Access configuration info
 * auto channel = dac1.GetChannel();
 * auto alignment = dac1.GetAlignment();
 * @endcode
 */
template <
    DacChannel DacChannelV,
    IsDacConfig DacConfigT =
        DacConfig<DacInputMax<100>, DacAlignment::Align12BRight>
>
class Dac {
public:

    /**
     * @brief Construct Dac class.
     * 
     * @param handle        Reference to the DAC handle.
     *
     * @note DAC is started automatically upon construction.
     */
    explicit Dac(DAC_HandleTypeDef& handle) noexcept
      : m_handle{handle}
    {
        HAL_DAC_Start(&m_handle, std::to_underlying(DacChannelV));
        Set(DacConfigT::InputRangeT::min_value);
    }

    /**
     * @defgroup Deleted copy and move members.
     * @{
     */
    Dac(const Dac&) = delete;
    Dac& operator=(const Dac&) = delete;
    Dac(Dac&&) = delete;
    Dac& operator=(Dac&&) = delete;
    /** @} */

    /**
     * @brief Destroy the Dac object, stops DAC.
     */
    ~Dac()
    {
        HAL_DAC_Stop(&m_handle, std::to_underlying(DacChannelV));
    }

    /**
     * @returns DAC handle reference.
     */
    [[nodiscard]]
    auto&& GetHandle(this auto&& self) noexcept
    {
        return std::forward<decltype(self)>(self).m_handle;
    }

    /**
     * @returns DAC channel.
     */
    [[nodiscard]]
    constexpr auto GetChannel() const noexcept
    {
        return DacChannelV;
    }

    /**
     * @returns DAC alignment value.
     */
    [[nodiscard]]
    constexpr auto GetAlignment() const noexcept
    {
        return DacConfigT::AlignmentT::alignment;
    }

    /**
     * @brief Set DAC output value.
     * 
     * @param output        Output value to be set.
     */
    void Set(std::uint32_t output) noexcept
    {
        HAL_DAC_SetValue(
            &m_handle,
            std::to_underlying(DacChannelV),
            DacConfigT::AlignmentT::alignment,
            ConvertToDac(
                std::clamp(
                    static_cast<double>(output),
                    DacConfigT::InputRangeT::min_value,
                    DacConfigT::InputRangeT::max_value
                )
            )
        );
    }

private:
    DAC_HandleTypeDef& m_handle;

    /**
     * @brief Convert output value to DAC value based on alignment.
     * 
     * @param output        Output value to convert.
     * 
     * @returns Corresponding DAC value.
     */
    constexpr std::uint32_t ConvertToDac(double output) const noexcept
    {
        auto normalized{
            (output - DacConfigT::InputRangeT::min_value) /
            DacConfigT::InputRangeT::range_size
        };
        return static_cast<std::uint32_t>(
            normalized * DacConfigT::AlignmentT::resolution
        );
    }
};

} /* namespace STM32 */

#endif /* STM32_DAC_HPP */
