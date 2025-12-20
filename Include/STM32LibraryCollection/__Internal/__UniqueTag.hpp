/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_UNIQUE_TAG_HPP
#define STM32_UNIQUE_TAG_HPP

#include <type_traits>

namespace STM32 {

namespace __Internal {

/**
 * @struct __UniqueTagWrapper, A wrapper struct for unique tag types.
 * 
 * @tparam LambdaT  Unique lambda type (generated via STM32_UNIQUE_TAG macro).
 * 
 * @note Do not use directly. Use STM32_UNIQUE_TAG macro instead.
 */
template <typename LambdaT>
struct __UniqueTagWrapper {
    struct Tag {};

    using LambdaTypeT = LambdaT;

    using TagT = Tag;

    explicit __UniqueTagWrapper(Tag) noexcept
    { }
};

/**
 * @brief __IsUniqueTag, A concept to check if a type is a valid UniqueTag.
 * 
 * @tparam T    Type to be checked.
 * 
 * This concept ensures that only types created via STM32_UNIQUE_TAG
 * can be used as UniqueTagT in peripheral classes.
 */
template <typename T>
concept __IsUniqueTag = 
    requires {
        typename T::LambdaTypeT;
        typename T::Tag;
    } &&
    std::is_empty_v<typename T::LambdaTypeT> &&
    std::is_class_v<typename T::LambdaTypeT> &&
    std::is_constructible_v<T, typename T::TagT>;

} /* namespace __Internal */

} /* namespace STM32 */

/**
 * @def STM32_UNIQUE_TAG
 *
 * @brief Macro to generate unique tag types for template differentiation.
 *
 * Each invocation creates a unique type using a lambda expression.
 *
 * @note Must be used at the instantiation site, not stored in a typedef
 *       and reused (each usage would then be the same type).
 *
 * @example Usage:
 * @code {.cpp}
 * // Correct - each is unique:
 * STM32::Uart<STM32::WorkingMode::Blocking, STM32_UNIQUE_TAG> uart1{huart1};
 * STM32::Uart<STM32::WorkingMode::Blocking, STM32_UNIQUE_TAG> uart2{huart2};
 *
 * // Also correct:
 * using Uart1Tag = STM32_UNIQUE_TAG;
 * using Uart2Tag = STM32_UNIQUE_TAG;
 *
 * // Incorrect - will not compile:
 * STM32::Uart<STM32::WorkingMode::Blocking, int> uart3{huart3}; // Error!
 * STM32::Uart<STM32::WorkingMode::Blocking, void> uart4{huart4}; // Error!
 * @endcode
 */
#define STM32_UNIQUE_TAG    ::STM32::__Internal::__UniqueTagWrapper<decltype([](){})>

#endif /* STM32_UNIQUE_TAG_HPP */
