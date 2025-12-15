/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_UTILITY_HPP
#define STM32_UTILITY_HPP

#include <concepts>

namespace STM32 {

/**
 * @struct Constant, A utility struct to hold compile-time constant values.
 * 
 * @tparam T        Type of the constant value.
 * @tparam ValueV   The constant value.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Utility.hpp>
 * 
 * auto value = STM32::Constant<int, 42>::value; // value is 42.
 * @endcode
 */
template <typename T, T ValueV>
struct Constant {
    using ValueTypeT = T;
    static constexpr T value{ValueV};
};

/**
 * @brief IsConstant, A concept to check if a type is a Constant.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Utility.hpp>
 * 
 * static_assert(STM32::IsConstant<STM32::Constant<int, 42>>);
 * static_assert(!STM32::IsConstant<int>);
 * @endcode
 */
template <typename T>
concept IsConstant =
    std::same_as<typename T::ValueTypeT, std::remove_cv_t<decltype(T::value)>> &&
    requires {
        T::value;
        typename T::ValueTypeT;
    };

/**
 * @struct Range, A utility struct to hold compile-time range values.
 * 
 * @tparam T                Type of the range values.
 * @tparam MinValueV        Minimum value of the range.
 * @tparam MaxValueV        Maximum value of the range.
 * @tparam DefaultValueV    Default value within the range.
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Utility.hpp>
 * 
 * using int_range = STM32::Range<int, 0, 100, 50>; // Range from 0 to 100 with default 50.
 * @endcode
 */
template<typename T, T MinValueV, T MaxValueV, T DefaultValueV = MinValueV>
struct Range {
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
 * @brief IsRange, A concept to check if a type is a Range.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Utility.hpp>
 * 
 * static_assert(STM32::IsRange<STM32::Range<int, 0, 100, 50>>);
 * static_assert(!STM32::IsRange<int>);
 * @endcode
 */
template <typename T>
concept IsRange =
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

} /* namespace STM32 */

#endif /* STM32_UTILITY_HPP */
