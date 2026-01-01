/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_UNIQUE_TAG_HPP
#define STM32_UNIQUE_TAG_HPP

#include <type_traits>

namespace STM32 {

namespace __Internal {

/**
 * @struct __UniqueTagWrapper, A wrapper that creates unique types from lambda expressions.
 * 
 * This struct exploits the fact that each lambda expression in C++ has a unique type.
 * By wrapping a lambda type, we create a unique tag that can be used to differentiate
 * template instantiations, enabling each peripheral instance to have independent
 * static storage for callbacks.
 * 
 * @tparam LambdaT  Unique lambda type (automatically generated via STM32_UNIQUE_TAG macro).
 * 
 * @note Do not use directly. Always use the STM32_UNIQUE_TAG macro.
 * 
 * ## How It Works
 * 
 * @code {.cpp}
 * // Each lambda has a unique type
 * auto lambda1 = [](){};
 * auto lambda2 = [](){};
 * static_assert(!std::is_same_v<decltype(lambda1), decltype(lambda2)>);
 * 
 * // STM32_UNIQUE_TAG expands to:
 * // ::STM32::__Internal::__UniqueTagWrapper<decltype([](){})>
 * // 
 * // Each expansion creates a new lambda, thus a new unique type
 * @endcode
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
 * @brief __IsUniqueTag, A concept to validate that a type is a proper unique tag.
 * 
 * @tparam T    Type to be checked.
 * 
 * This concept ensures that:
 * - The type has a `LambdaTypeT` member (the wrapped lambda type).
 * - The type has a `Tag` member type.
 * - The lambda type is empty (stateless lambda).
 * - The lambda type is a class type.
 * - The wrapper is constructible from its Tag type.
 * 
 * This prevents users from passing arbitrary types like `int` or `void`
 * as UniqueTagT to peripheral classes.
 * 
 * @note This is an internal concept. Do not use directly in application code.
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
 * Each invocation of this macro creates a unique type by leveraging the fact
 * that every lambda expression in C++ has a distinct type. This enables
 * multiple instances of the same peripheral class to have independent
 * static callback storage.
 *
 * ## Why This Is Needed
 * 
 * The __CallbackManager uses static members to store callbacks (required for
 * C-style HAL callback functions). Without unique tags, all instances would
 * share the same static storage, causing callback collisions.
 *
 * ## Usage Rules
 * 
 * 1. **Always use at instantiation site** - Each usage generates a new unique type.
 * 2. **Never reuse a typedef** - Storing in a typedef and reusing creates the same type.
 * 3. **One tag per instance** - Each peripheral instance needs its own unique tag.
 *
 * @example Correct Usage:
 * @code {.cpp}
 * // Each STM32_UNIQUE_TAG creates a distinct type
 * STM32::Uart<STM32::WorkingMode::Interrupt, STM32_UNIQUE_TAG> uart1{huart1};
 * STM32::Uart<STM32::WorkingMode::Interrupt, STM32_UNIQUE_TAG> uart2{huart2};
 * 
 * // Also correct - separate typedefs
 * using Uart1Tag = STM32_UNIQUE_TAG;  // Unique type A
 * using Uart2Tag = STM32_UNIQUE_TAG;  // Unique type B (different from A)
 * STM32::Uart<STM32::WorkingMode::DMA, Uart1Tag> uart3{huart3};
 * STM32::Uart<STM32::WorkingMode::DMA, Uart2Tag> uart4{huart4};
 * @endcode
 * 
 * @example Incorrect Usage:
 * @code {.cpp}
 * // WRONG - reusing the same typedef creates shared callbacks!
 * using SharedTag = STM32_UNIQUE_TAG;
 * STM32::Uart<STM32::WorkingMode::Interrupt, SharedTag> uart1{huart1};
 * STM32::Uart<STM32::WorkingMode::Interrupt, SharedTag> uart2{huart2};  // Shares callbacks with uart1!
 * 
 * // WRONG - arbitrary types are rejected by __IsUniqueTag concept
 * STM32::Uart<STM32::WorkingMode::Interrupt, int> uart3{huart3};   // Compile error!
 * STM32::Uart<STM32::WorkingMode::Interrupt, void> uart4{huart4};  // Compile error!
 * @endcode
 */
#define STM32_UNIQUE_TAG    ::STM32::__Internal::__UniqueTagWrapper<decltype([](){})>

#endif /* STM32_UNIQUE_TAG_HPP */
