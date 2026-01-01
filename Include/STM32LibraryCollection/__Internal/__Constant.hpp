/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_CONSTANT_HPP
#define STM32_CONSTANT_HPP

#include <concepts>

namespace STM32 {

namespace __Internal {

/**
 * @struct __Constant, A utility struct to wrap compile-time constant values as types.
 * 
 * This enables using constant values as template parameters and provides
 * a uniform interface for accessing the value and its type.
 * 
 * @tparam T        Type of the constant value.
 * @tparam ValueV   The constant value itself.
 *
 * @note This is an internal class. Do not use directly in application code.
 *
 * @example Usage:
 * @code {.cpp}
 * // Define a timeout constant
 * using Timeout100ms = STM32::__Internal::__Constant<std::uint32_t, 100>;
 * 
 * // Access the value
 * auto timeout = Timeout100ms::value;  // 100
 * 
 * // Use in templates
 * template <__IsConstant TimeoutT>
 * void WaitFor() {
 *     HAL_Delay(TimeoutT::value);
 * }
 * @endcode
 */
template <typename T, T ValueV>
struct __Constant {
    using ValueTypeT = T;
    static constexpr T value{ValueV};
};

/**
 * @brief __IsConstant, A concept to check if a type satisfies the Constant interface.
 * 
 * A type satisfies this concept if it provides:
 * - `value`: A static constexpr member holding the constant.
 * - `ValueTypeT`: A type alias for the value's type.
 * 
 * @tparam T        Type to be checked.
 *
 * @note This is an internal concept. Do not use directly in application code.
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