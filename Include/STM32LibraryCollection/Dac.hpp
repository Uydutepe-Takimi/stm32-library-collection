/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_DAC_HPP
#define STM32_DAC_HPP

#include <algorithm>
#include <cstdint>
#include <utility>

#include "Config.hpp"
#include "Utility.hpp"

#include "main.h"

#if !defined(HAL_DAC_MODULE_ENABLED) /* module check */
#error "HAL DAC module is not enabled!"
#endif /* module check */

namespace STM32 {

namespace DacAlignment {
/**
 * @struct Align12BRight, tag for 12-bit right alignment.
 */
struct Align12BRight {
    static constexpr std::uint32_t alignment{DAC_ALIGN_12B_R};
    static constexpr double resolution{4095.0};
};

/**
 * @struct Align12BLeft, tag for 12-bit left alignment.
 */
struct Align12BLeft {
    static constexpr std::uint32_t alignment{DAC_ALIGN_12B_L};
    static constexpr double resolution{4095.0};
};

/**
 * @struct Align8BRight, tag for 8-bit right alignment.
 */
struct Align8BRight {
    static constexpr std::uint32_t alignment{DAC_ALIGN_8B_R};
    static constexpr double resolution{255.0};
};
} /* namespace DacAlignment */

/**
 * @brief DacChannel, Enumeration for DAC channels.
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
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Dac.hpp>
 * 
 * static_assert(STM32::IsDacAlignment<STM32::DacAlignment::Align12BRight>);
 * static_assert(!STM32::IsDacAlignment<int>);
 * @endcode
 */
template <typename T>
concept IsDacAlignment = 
    std::same_as<typename std::remove_cv_t<decltype(T::alignment)>, std::uint32_t> &&
    std::same_as<typename std::remove_cv_t<decltype(T::resolution)>, double> &&
    requires {
        T::alignment;
        T::resolution;
    };

/**
 * @brief DacInputMax, A utility struct to hold maximum input value for DAC.
 * 
 * @tparam MaxV     Maximum input value for DAC.
 */
template <std::uint32_t MaxV>
struct DacInputMax : Range<double, 0., static_cast<double>(MaxV)> {};

/**
 * @brief IsDacInputMax, A concept to check if a type is a DacInputMax.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Dac.hpp>
 * 
 * static_assert(STM32::IsDacInputMax<STM32::DacInputMax<100>>);
 * static_assert(!STM32::IsDacInputMax<int>);
 * @endcode
 */
template <typename T>
concept IsDacInputMax =
    IsRange<T> &&
    std::same_as<typename T::ValueTypeT, double>;

/**
 * @brief DacConfig, A utility struct to hold DAC configuration.
 * 
 * @tparam DacInputMaxT    DAC input maximum value type.
 * @tparam DacAlignmentT   DAC alignment type.
 */
template <IsDacInputMax DacInputMaxT, IsDacAlignment DacAlignmentT>
struct DacConfig : DacInputMaxT, DacAlignmentT {
    using InputRangeT = DacInputMaxT;
    using AlignmentT = DacAlignmentT;
    static_assert(
        InputRangeT::max_value <= AlignmentT::resolution,
        "DacInputMaxT must be less than or equal to DacAlignmentT resolution!"
    );
};

/**
 * @brief IsDacConfig, A concept to check if a type is a DacConfig.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage;
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
    T::InputRangeT::max_value <= T::AlignmentT::resolution &&
    requires {
        typename T::InputRangeT;
        typename T::AlignmentT;
        T::InputRangeT::max_value;
        T::AlignmentT::resolution;
    };

/**
 * @class Dac, A class to manage DAC functionality on STM32 microcontrollers.
 *
 * @tparam DacConfigT   DAC configuration type.
 *
 * @note Dac class is non-copyable and non-movable.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Dac.hpp>
 *
 * using MyDac = STM32::Dac<
 *     STM32::DacConfig<
 *         STM32::DacInputMax<100>,
 *         STM32::DacAlignment::Align12BRight
 *     >
 * >;
 * DAC_HandleTypeDef hdac; // Assume this is properly initialized elsewhere.
 * MyDac led{hdac, STM32::DacChannel::Channel1};
 * led.Set(50); // Set DAC output to mid-range value.
 * @endcode
 */
template <
    IsDacConfig DacConfigT =
        DacConfig<DacInputMax<100>, DacAlignment::Align12BRight>
>
class Dac {
public:
    /**
     * @brief Construct Dac class.
     * 
     * @param handle        Reference to the DAC handle.
     * @param channel       DAC channel to be used.
     *
     * @note DAC is started automatically upon construction.
     */
    Dac(
        DAC_HandleTypeDef& handle,
        DacChannel channel
    ) noexcept
      : m_handle{handle},
        m_channel{channel}
    {
        HAL_DAC_Start(&m_handle, std::to_underlying(m_channel));
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
        HAL_DAC_Stop(&m_handle, std::to_underlying(m_channel));
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
    DacChannel GetChannel() const noexcept
    {
        return m_channel;
    }

    /**
     * @returns DAC alignment value.
     */
    [[nodiscard]]
    constexpr auto GetDacAlignment() const noexcept
    {
        return DacConfigT::AlignmentT::alignment;
    }

    /**
     * @returns Get current DAC output value.
     */
    [[nodiscard]]
    std::uint32_t Get() const noexcept
    {
        return HAL_DAC_GetValue(
            &m_handle,
            std::to_underlying(m_channel)
        );
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
            std::to_underlying(m_channel),
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
    DacChannel m_channel;

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
