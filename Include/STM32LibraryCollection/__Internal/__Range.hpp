/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_RANGE_HPP
#define STM32_RANGE_HPP

#include <concepts>

namespace STM32 {

namespace __Internal {

/**
 * @struct __Range, A utility struct to define compile-time numeric ranges.
 * 
 * Provides min/max bounds, a default value, and automatically computes the range size.
 * Used as a base for configuration types like PwmInputRange, DacInputMax, etc.
 * 
 * @tparam T                Type of the range values (e.g., int, double, std::uint32_t).
 * @tparam MinValueV        Minimum value of the range (inclusive).
 * @tparam MaxValueV        Maximum value of the range (inclusive).
 * @tparam DefaultValueV    Default value within the range (defaults to MinValueV).
 * 
 * @note MinValueV must be strictly less than MaxValueV.
 * @note DefaultValueV must be within [MinValueV, MaxValueV].
 * @note This is an internal class. Do not use directly in application code.
 * 
 * @example Usage:
 * @code {.cpp}
 * // Integer range 0-100 with default 50
 * using Percentage = STM32::__Internal::__Range<int, 0, 100, 50>;
 * auto min = Percentage::min_value;      // 0
 * auto max = Percentage::max_value;      // 100
 * auto def = Percentage::default_value;  // 50
 * auto size = Percentage::range_size;    // 100
 * 
 * // Double range for duty cycle
 * using DutyCycle = STM32::__Internal::__Range<double, 0.0, 100.0>;
 * @endcode
 */
template<typename T, T MinValueV, T MaxValueV, T DefaultValueV = MinValueV>
struct __Range {
    static_assert(
        MinValueV < MaxValueV,
        "MinValueV must be less than MaxValueV"
    );
    static_assert(
        MinValueV <= DefaultValueV && DefaultValueV <= MaxValueV,
        "DefaultValueV is not in the range [MinValueV, MaxValueV]"
    );
    using ValueTypeT = T;
    static constexpr T min_value{MinValueV};
    static constexpr T max_value{MaxValueV};
    static constexpr T default_value{DefaultValueV};
    static constexpr T range_size{MaxValueV - MinValueV};
};

/**
 * @brief __IsRange, A concept to check if a type satisfies the Range interface.
 * 
 * A type satisfies this concept if it provides:
 * - `min_value`: Minimum bound of the range.
 * - `max_value`: Maximum bound of the range.
 * - `default_value`: Default value within the range.
 * - `range_size`: Computed as (max_value - min_value).
 * - `ValueTypeT`: Type alias for the value type.
 * 
 * Additionally validates that min < max and default is within bounds.
 * 
 * @tparam T        Type to be checked.
 *
 * @note This is an internal concept. Do not use directly in application code.
 */
template <typename T>
concept __IsRange =
    std::same_as<typename T::ValueTypeT, std::remove_cv_t<decltype(T::min_value)>> &&
    std::same_as<typename T::ValueTypeT, std::remove_cv_t<decltype(T::max_value)>> &&
    std::same_as<typename T::ValueTypeT, std::remove_cv_t<decltype(T::default_value)>> &&
    std::same_as<typename T::ValueTypeT, std::remove_cv_t<decltype(T::range_size)>> &&
    T::min_value < T::max_value &&
    T::min_value <= T::default_value && T::default_value <= T::max_value &&
    T::range_size == (T::max_value - T::min_value) &&
    requires {
        T::min_value;
        T::max_value;
        T::default_value;
        T::range_size;
        typename T::ValueTypeT;
    };

} /* namespace __Internal */

} /* namespace STM32 */

#endif /* STM32_RANGE_HPP */