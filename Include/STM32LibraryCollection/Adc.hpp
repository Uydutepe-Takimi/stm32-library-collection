/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_ADC_HPP
#define STM32_ADC_HPP

#include <array>
#include <cmath>
#include <algorithm>

#include "Utility.hpp"

#include "main.h"

#if !defined(HAL_ADC_MODULE_ENABLED) /* module check */
#error "HAL ADC module is not enabled!"
#endif /* module check */

namespace STM32 {

namespace AdcResolution {

/**
 * @struct Resolution12Bit, tag for 12-bit resolution.
 */
struct Resolution12Bit {
    static constexpr double resolution{4095.};
};

/**
 * @struct Resolution10Bit, tag for 10-bit resolution.
 */
struct Resolution10Bit {
    static constexpr double resolution{1023.};
};

/**
 * @struct Resolution8Bit, tag for 8-bit resolution.
 */
struct Resolution8Bit {
    static constexpr double resolution{255.};
};

} /* namespace AdcResolution */

/**
 * @brief IsAdcResolution, A concept to check if a type is a AdcResolution.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Adc.hpp>
 * 
 * static_assert(STM32::IsAdcResolution<STM32::AdcResolution::Resolution12Bit>);
 * static_assert(!STM32::IsAdcResolution<int>);
 * @endcode
 */
template <typename T>
concept IsAdcResolution =
    std::same_as<std::remove_cv_t<decltype(T::resolution)>, double> &&
    requires {
        T::resolution;
    };

/**
 * @brief AdcOutputMax, A utility struct to hold maximum output value for ADC.
 * 
 * @tparam MaxV     Maximum output value (Get() returns 0 to MaxV).
 *
 * @note The ADC reading is mapped from hardware resolution to this range.
 *
 * @example An ADC value of 2048 (12-bit) maps to MaxV/2.
 */
template <std::uint32_t MaxV>
struct AdcOutputMax : Range<double, 0., static_cast<double>(MaxV)> {};

/**
 * @brief IsAdcOutputMax, A concept to check if a type is a AdcOutputMax.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Adc.hpp>
 * 
 * static_assert(STM32::IsAdcOutputMax<STM32::AdcOutputMax<100>>);
 * static_assert(!STM32::IsAdcOutputMax<int>);
 * @endcode
 */
template <typename T>
concept IsAdcOutputMax =
    IsRange<T> &&
    std::same_as<typename T::ValueTypeT, double>;

/**
 * @brief AdcMedianFilterSize, A utility struct to hold ADC median filter size.
 * 
 * @tparam SizeV    Size of the median filter (must be odd and >= 1).
 */
template <std::uint32_t SizeV>
struct AdcMedianFilterSize : Constant<std::uint32_t, SizeV> {
    static_assert(
        SizeV % 2 != 0,
        "Median filter size must be an odd number!"
    );
    static_assert(
        1 <= SizeV,
        "Median filter size must be greater or equal to 1!"
    );
};

/**
 * @brief IsAdcMedianFilterSize, A concept to check if a type is a AdcMedianFilterSize.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Adc.hpp>
 * 
 * static_assert(STM32::IsAdcMedianFilterSize<STM32::AdcMedianFilterSize<5>>);
 * static_assert(!STM32::IsAdcMedianFilterSize<int>);
 * @endcode
 */
template <typename T>
concept IsAdcMedianFilterSize =
    IsConstant<T> &&
    std::same_as<typename T::ValueTypeT, std::uint32_t> &&
    (T::value % 2 != 0) &&
    (1 <= T::value);

/**
 * @brief AdcTimeout, A utility struct to hold ADC timeout value in milliseconds.
 * 
 * @tparam TimeoutV    Timeout value in milliseconds.
 */
template <std::uint32_t TimeoutV>
struct AdcTimeout : Constant<std::uint32_t, TimeoutV> { };

/**
 * @brief IsAdcTimeout, A concept to check if a type is a AdcTimeout.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Adc.hpp>
 * 
 * static_assert(STM32::IsAdcTimeout<STM32::AdcTimeout<5000>>);
 * static_assert(!STM32::IsAdcTimeout<int>);
 * @endcode
 */
template <typename T>
concept IsAdcTimeout =
    IsConstant<T> &&
    std::same_as<typename T::ValueTypeT, std::uint32_t>;

/**
 * @brief AdcConfig, A utility struct to hold ADC configuration.
 * 
 * @tparam AdcOutputMaxT        ADC output maximum value type.
 * @tparam AdcResolutionT       ADC resolution type.
 * @tparam AdcMedianFilterSizeT ADC median filter size type.
 * @tparam AdcTimeoutT          ADC timeout type.
 */
template <
    IsAdcOutputMax AdcOutputMaxT,
    IsAdcResolution AdcResolutionT,
    IsAdcMedianFilterSize AdcMedianFilterSizeT,
    IsAdcTimeout AdcTimeoutT
>
struct AdcConfig : AdcOutputMaxT, AdcResolutionT, AdcMedianFilterSizeT, AdcTimeoutT {
    using OutputRangeT = AdcOutputMaxT;
    using ResolutionT = AdcResolutionT;
    using MedianFilterSizeT = AdcMedianFilterSizeT;
    using TimeoutT = AdcTimeoutT;
};

/**
 * @brief IsAdcConfig, A concept to check if a type is a AdcConfig.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Adc.hpp>
 * 
 * static_assert(STM32::IsAdcConfig<STM32::AdcConfig<
 *     STM32::AdcOutputMax<100>,
 *     STM32::AdcResolution::Resolution12Bit,
 *     STM32::AdcMedianFilterSize<5>,
 *     STM32::AdcTimeout<5000>
 * >>);
 * static_assert(!STM32::IsAdcConfig<int>);
 * @endcode
 */
template <typename T>
concept IsAdcConfig =
    IsAdcOutputMax<typename T::OutputRangeT> &&
    IsAdcResolution<typename T::ResolutionT> &&
    IsAdcMedianFilterSize<typename T::MedianFilterSizeT> &&
    IsAdcTimeout<typename T::TimeoutT> &&
    requires {
        typename T::OutputRangeT;
        typename T::ResolutionT;
        typename T::MedianFilterSizeT;
        typename T::TimeoutT;
        T::OutputRangeT::max_value;
        T::ResolutionT::resolution;
    };

/**
 * @class Adc, A class to manage ADC functionality on STM32 microcontrollers.
 *
 * @tparam AdcConfigT   ADC configuration type.
 *
 * @note Adc class is non-copyable and non-movable.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Adc.hpp>
 *
 * using MyAdc = STM32::Adc<
 *     STM32::AdcConfig<
 *         STM32::AdcOutputMax<100>,
 *         STM32::AdcResolution::Resolution12Bit,
 *         STM32::AdcMedianFilterSize<5>,
 *         STM32::AdcTimeout<5000>
 *     >
 * >;
 * ADC_HandleTypeDef hadc; // Assume this is properly initialized elsewhere.
 * MyAdc sensor{hadc};
 * double value = sensor.Get(); // Get ADC value.
 * @endcode
 */
template <
    IsAdcConfig AdcConfigT =
        AdcConfig<
            AdcOutputMax<100>,
            AdcResolution::Resolution12Bit,
            AdcMedianFilterSize<5>,
            AdcTimeout<5'000>
        >
>
class Adc {
public:

    /**
     * @brief Constructor for Adc class.
     * 
     * @param handle        Reference to ADC handle.
     */
    explicit Adc(ADC_HandleTypeDef& handle) noexcept
    : m_handle{handle}
    { }

    /**
     * @defgroup Deleted copy and move members.
     * @{
     */
    Adc(const Adc&) = delete;
    Adc& operator=(const Adc&) = delete;
    Adc(Adc&&) = delete;
    Adc& operator=(Adc&&) = delete;
    /** @} */

    /**
     * @returns ADC handle reference.
     */
    [[nodiscard]]
    auto&& GetHandle(this auto&& self) noexcept
    {
        return std::forward<decltype(self)>(self).m_handle;
    }

    /**
     * @returns Raw ADC output value without filtering.
     */
    [[nodiscard]]
    std::uint32_t GetRaw() const noexcept
    {
        if (HAL_ADC_Start(&m_handle) != HAL_OK){
            return 0;
        }
        if (HAL_ADC_PollForConversion(&m_handle , AdcConfigT::TimeoutT::value) != HAL_OK){
            HAL_ADC_Stop(&m_handle);
            return 0;
        }
        const auto adc_value = HAL_ADC_GetValue(&m_handle);
        HAL_ADC_Stop(&m_handle);
        return adc_value;
    }

    /**
     * @returns Current ADC output value, applying median filter.
     */
    [[nodiscard]]
    std::uint32_t Get() const noexcept
    {
        std::array<std::uint32_t, AdcConfigT::MedianFilterSizeT::value> adc_values{};
        std::size_t filled_size{};
        for (std::size_t i{}; i < adc_values.size(); ++i){
            if (HAL_ADC_Start(&m_handle) != HAL_OK){
                continue;
            }
            if (HAL_ADC_PollForConversion(&m_handle , AdcConfigT::TimeoutT::value) == HAL_OK){
                adc_values[filled_size++] = ConvertToOutput(HAL_ADC_GetValue(&m_handle));
            }
            HAL_ADC_Stop(&m_handle);
        }
        if (filled_size == 0){
            return 0;
        }
        std::ranges::sort(adc_values.begin(), adc_values.begin() + filled_size);
        return adc_values[filled_size / 2];
    }

private:
    ADC_HandleTypeDef& m_handle;

    /**
     * @brief ConvertToOutput, Converts raw ADC value to configured output range.
     * 
     * @param adc_value     Raw ADC value.
     *
     * @returns Converted ADC output value.
     */
    constexpr auto ConvertToOutput(double adc_value) const noexcept
    {
        const double scaled{
            (adc_value * AdcConfigT::OutputRangeT::range_size) /
            AdcConfigT::ResolutionT::resolution
        };
        return static_cast<std::uint32_t>(
            std::round(scaled) + AdcConfigT::OutputRangeT::min_value
        );
    }
};

} /* namespace STM32 */

#endif /* STM32_ADC_HPP */
