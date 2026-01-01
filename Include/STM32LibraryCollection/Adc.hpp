/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_ADC_HPP
#define STM32_ADC_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>

#include "__Internal/__Utility.hpp"

#include "main.h"

#if !defined(HAL_ADC_MODULE_ENABLED) /* module check */
#error "HAL ADC module is not enabled!"
#endif /* module check */

namespace STM32 {

/**
 * @namespace AdcResolution, Tag types for ADC resolution configuration.
 * 
 * These tags specify the ADC hardware resolution at compile time.
 * The resolution determines the maximum raw value the ADC can produce.
 */
namespace AdcResolution {

/**
 * @struct Resolution12Bit, Tag for 12-bit ADC resolution.
 * 
 * Raw ADC values range from 0 to 4095.
 */
struct Resolution12Bit {
    static constexpr double resolution{4095.};
};

/**
 * @struct Resolution10Bit, Tag for 10-bit ADC resolution.
 * 
 * Raw ADC values range from 0 to 1023.
 */
struct Resolution10Bit {
    static constexpr double resolution{1023.};
};

/**
 * @struct Resolution8Bit, Tag for 8-bit ADC resolution.
 * 
 * Raw ADC values range from 0 to 255.
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
 * @example Usage:
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
 * @struct AdcOutputMax, A utility struct to define the output value range for ADC.
 * 
 * @tparam MaxV     Maximum output value. Get() returns values in range [0, MaxV].
 *
 * The ADC reading is linearly mapped from hardware resolution to this range.
 * This allows you to work with meaningful units (e.g., percentage, temperature)
 * instead of raw ADC values.
 *
 * @example Usage:
 * @code {.cpp}
 * // Map ADC to percentage (0-100)
 * using PercentOutput = STM32::AdcOutputMax<100>;
 * 
 * // Map ADC to temperature range (0-330 for 0.0-3.3V at 10mV/°C)
 * using TempOutput = STM32::AdcOutputMax<330>;
 * @endcode
 */
template <std::uint32_t MaxV>
struct AdcOutputMax : __Internal::__Range<double, 0., static_cast<double>(MaxV)> {};

/**
 * @brief IsAdcOutputMax, A concept to check if a type is a AdcOutputMax.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Adc.hpp>
 * 
 * static_assert(STM32::IsAdcOutputMax<STM32::AdcOutputMax<100>>);
 * static_assert(!STM32::IsAdcOutputMax<int>);
 * @endcode
 */
template <typename T>
concept IsAdcOutputMax =
    __Internal::__IsRange<T> &&
    std::same_as<typename T::ValueTypeT, double>;

/**
 * @struct AdcMedianFilterSize, A utility struct to configure median filter size.
 * 
 * @tparam SizeV    Number of samples for median filtering (must be odd and >= 1).
 *
 * The median filter reduces noise by taking multiple ADC readings, sorting them,
 * and returning the middle value. Larger sizes provide better noise rejection
 * but increase read time.
 *
 * @note Use SizeV=1 to disable filtering (single sample).
 * @note Typical values: 3, 5, 7 for good noise rejection.
 *
 * @example Usage:
 * @code {.cpp}
 * using NoFilter = STM32::AdcMedianFilterSize<1>;      // Single sample
 * using LightFilter = STM32::AdcMedianFilterSize<3>;   // 3 samples
 * using HeavyFilter = STM32::AdcMedianFilterSize<7>;   // 7 samples
 * @endcode
 */
template <std::uint32_t SizeV>
struct AdcMedianFilterSize : __Internal::__Constant<std::uint32_t, SizeV> {
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
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Adc.hpp>
 * 
 * static_assert(STM32::IsAdcMedianFilterSize<STM32::AdcMedianFilterSize<5>>);
 * static_assert(!STM32::IsAdcMedianFilterSize<int>);
 * @endcode
 */
template <typename T>
concept IsAdcMedianFilterSize =
    __Internal::__IsConstant<T> &&
    std::same_as<typename T::ValueTypeT, std::uint32_t> &&
    (T::value % 2 != 0) &&
    (1 <= T::value);

/**
 * @struct AdcTimeout, A utility struct to configure ADC conversion timeout.
 * 
 * @tparam TimeoutV    Timeout value in milliseconds.
 *
 * If the ADC conversion does not complete within this time, the operation
 * is aborted and returns 0.
 *
 * @example Usage:
 * @code {.cpp}
 * using FastTimeout = STM32::AdcTimeout<100>;    // 100ms timeout
 * using SlowTimeout = STM32::AdcTimeout<5000>;   // 5 second timeout
 * @endcode
 */
template <std::uint32_t TimeoutV>
struct AdcTimeout : __Internal::__Constant<std::uint32_t, TimeoutV> { };

/**
 * @brief IsAdcTimeout, A concept to check if a type is a AdcTimeout.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Adc.hpp>
 * 
 * static_assert(STM32::IsAdcTimeout<STM32::AdcTimeout<5000>>);
 * static_assert(!STM32::IsAdcTimeout<int>);
 * @endcode
 */
template <typename T>
concept IsAdcTimeout =
    __Internal::__IsConstant<T> &&
    std::same_as<typename T::ValueTypeT, std::uint32_t>;

/**
 * @struct AdcConfig, A utility struct to bundle all ADC configuration parameters.
 * 
 * @tparam AdcOutputMaxT        Output range configuration (0 to MaxV).
 * @tparam AdcResolutionT       Hardware resolution (8, 10, or 12 bit).
 * @tparam AdcMedianFilterSizeT Median filter sample count.
 * @tparam AdcTimeoutT          Conversion timeout in milliseconds.
 *
 * @example Usage:
 * @code {.cpp}
 * // Percentage output with 12-bit resolution, 5-sample filter, 1s timeout
 * using MyAdcConfig = STM32::AdcConfig<
 *     STM32::AdcOutputMax<100>,
 *     STM32::AdcResolution::Resolution12Bit,
 *     STM32::AdcMedianFilterSize<5>,
 *     STM32::AdcTimeout<1000>
 * >;
 * @endcode
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
 * @example Usage:
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
 * @tparam AdcConfigT   ADC configuration type (defaults to 12-bit, 0-100 output,
 *                      5-sample median filter, 5s timeout).
 *
 * @note Adc class is non-copyable and non-movable.
 * @note Uses blocking polling mode for ADC conversion.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Adc.hpp>
 *
 * ADC_HandleTypeDef hadc1; // Assume properly initialized by CubeMX
 *
 * // 1. Use default configuration (0-100 output, 12-bit, 5-sample filter)
 * STM32::Adc<> sensor1{hadc1};
 * auto percent = sensor1.Get();     // Returns 0-100
 *
 * // 2. Custom configuration for temperature sensor
 * using TempAdcConfig = STM32::AdcConfig<
 *     STM32::AdcOutputMax<330>,                    // 0-330 (0.0-3.3V at 10mV/unit)
 *     STM32::AdcResolution::Resolution12Bit,
 *     STM32::AdcMedianFilterSize<7>,               // Heavy filtering for stability
 *     STM32::AdcTimeout<1000>
 * >;
 * STM32::Adc<TempAdcConfig> tempSensor{hadc1};
 * auto temp = tempSensor.Get();     // Returns 0-330
 *
 * // 3. Raw ADC value without filtering
 * auto raw = sensor1.GetRaw();      // Returns 0-4095 (12-bit)
 *
 * // 4. Access underlying HAL handle
 * auto& handle = sensor1.GetHandle();
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
     * @brief Get raw ADC value without filtering or scaling.
     * 
     * Performs a single ADC conversion and returns the raw hardware value.
     * 
     * @returns Raw ADC value (0 to resolution max), or 0 on error.
     * 
     * @note Returns 0 if ADC start or conversion fails.
     * @note Use Get() for filtered and scaled output.
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
     * @brief Get filtered and scaled ADC value.
     * 
     * Takes multiple samples (defined by MedianFilterSizeT), sorts them,
     * and returns the median value scaled to the configured output range.
     * 
     * @returns Scaled ADC value (0 to AdcOutputMax), or 0 on error.
     * 
     * @note Returns 0 if all ADC conversions fail.
     *
     * @note Partial failures are handled gracefully using available samples.
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
