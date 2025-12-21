/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_CONSTANT_HPP
#define STM32_CONSTANT_HPP

#include <concepts>

namespace STM32 {

namespace __Internal {

/**
 * @struct __Constant, A utility struct to hold compile-time constant values.
 * 
 * @tparam T        Type of the constant value.
 * @tparam ValueV   The constant value.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/__Internal/__Constant.hpp>
 * 
 * auto value = STM32::__Internal::__Constant<int, 42>::value; // value is 42.
 * @endcode
 */
template <typename T, T ValueV>
struct __Constant {
    using ValueTypeT = T;
    static constexpr T value{ValueV};
};

/**
 * @brief __IsConstant, A concept to check if a type is a Constant.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/__Internal/__Constant.hpp>
 * 
 * static_assert(STM32::__Internal::__IsConstant<STM32::__Internal::__Constant<int, 42>>);
 * static_assert(!STM32::__Internal::__IsConstant<int>);
 * @endcode
 */
template <typename T>
concept __IsConstant =
    std::same_as<typename T::ValueTypeT, std::remove_cv_t<decltype(T::value)>> &&
    requires {
        T::value;
        typename T::ValueTypeT;
    };

} /* namespace __Internal */

} /* namespace STM32 */

#endif /* STM32_CONSTANT_HPP */