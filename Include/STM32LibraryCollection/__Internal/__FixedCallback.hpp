/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_FIXED_CALLBACK_HPP
#define STM32_FIXED_CALLBACK_HPP

#include <concepts>
#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace STM32 {

namespace __Internal {

/**
 * @class __FixedCallback, A non-allocating callable wrapper for embedded systems.
 * 
 * This class provides std::function-like type erasure for callables (lambdas,
 * function pointers, functors) without heap allocation. The callable is stored
 * in a fixed-size internal buffer.
 * 
 * @tparam CapacityV     Size of the internal buffer in bytes (default: 64).
 *                       Must be large enough to hold the callable and its captures.
 * @tparam AlignmentV    AlignmentV of the internal buffer (default: max alignment).
 * 
 * If a callable exceeds the capacity, a static_assert will trigger at compile time.
 * 
 * @note This is an internal class. Do not use directly in application code.
 * @note This class is move-only (non-copyable) to prevent unintended duplication.
 * 
 * ## Typical Capture Sizes
 * 
 * - Empty lambda `[](){}`: ~1 byte
 * - One pointer capture `[ptr](){}`: ~8 bytes
 * - Two pointer captures `[a, b](){}`: ~16 bytes
 * - Reference capture (pointer internally): ~8 bytes per reference
 * 
 * The default 64-byte capacity handles most common embedded callback patterns.
 * 
 * @example Usage:
 * @code {.cpp}
 * __FixedCallback<> cb1 = []() { doSomething(); };  // OK - empty lambda
 * 
 * int* ptr = &some_value;
 * __FixedCallback<> cb2 = [ptr]() { *ptr = 42; };  // OK - single pointer
 * 
 * // Large capture - increase capacity or reduce captures
 * std::array<int, 100> big_data{};
 * __FixedCallback<512> cb3 = [big_data]() { useBigData(big_data); };  // OK with larger capacity
 * // __FixedCallback<32> cb4 = [big_data]() {};  // Compile error! Too large
 * @endcode
 */
template <std::size_t CapacityV = 64, std::size_t AlignmentV = alignof(std::max_align_t)>
class __FixedCallback {
public:

    /**
     * @brief Default constructor, creates an empty (null) callback.
     */
    __FixedCallback() noexcept = default;

    /**
     * @brief Construct from nullptr.
     */
    __FixedCallback(std::nullptr_t) noexcept
       : __FixedCallback{}
    { }

    /**
     * @brief Construct from a callable (lambda, functor, function pointer).
     * 
     * @tparam F    Callable type (must be invocable with no arguments).
     * @param f     The callable to store.
     * 
     * @note Fails at compile time if sizeof(F) > CapacityV.
     */
    template <typename F>
    __FixedCallback(F&& f) noexcept
    requires std::invocable<F> && 
        (!std::same_as<std::decay_t<F>, __FixedCallback>)
    {
        using DecayedF = std::decay_t<F>;
        static_assert(
            sizeof(DecayedF) <= CapacityV,
            "Callback captures too large for fixed buffer! "
            "Reduce captures or increase __FixedCallback capacity."
        );
        static_assert(
            alignof(DecayedF) <= AlignmentV,
            "Callback alignment requirement exceeds buffer alignment!"
        );
        static_assert(
            std::is_nothrow_move_constructible_v<DecayedF>,
            "Callback must be nothrow move constructible for embedded safety."
        );
        std::construct_at(
            reinterpret_cast<DecayedF*>(m_storage),
            std::forward<F>(f)
        );
        m_invoke = [](void* ptr) noexcept {
            (*static_cast<DecayedF*>(ptr))();
        };
        m_destroy = [](void* ptr) noexcept {
            std::destroy_at(static_cast<DecayedF*>(ptr));
        };
        m_move = [](void* dst, void* src) noexcept {
            std::construct_at(
                static_cast<DecayedF*>(dst),
                std::move(*static_cast<DecayedF*>(src))
            );
            std::destroy_at(static_cast<DecayedF*>(src));
        };
    }

    /**
     * @brief Destructor, destroys the stored callable if any.
     */
    ~__FixedCallback()
    {
        Reset();
    }

    /**
     * @brief Move constructor.
     * 
     * @param other     Callback to move from (becomes null after move).
     */
    __FixedCallback(__FixedCallback&& other) noexcept
      : m_invoke{other.m_invoke},
        m_destroy{other.m_destroy},
        m_move{other.m_move}
    {
        if (m_move) {
            m_move(m_storage, other.m_storage);
        }
        other.m_invoke = nullptr;
        other.m_destroy = nullptr;
        other.m_move = nullptr;
    }

    /**
     * @brief Move assignment operator.
     * 
     * @param other     Callback to move from (becomes null after move).
     *
     * @returns         Reference to this.
     */
    __FixedCallback& operator=(__FixedCallback&& other) noexcept
    {
        if (this != &other) {
            Reset();
            m_invoke = other.m_invoke;
            m_destroy = other.m_destroy;
            m_move = other.m_move;
            if (m_move) {
                m_move(m_storage, other.m_storage);
            }
            other.m_invoke = nullptr;
            other.m_destroy = nullptr;
            other.m_move = nullptr;
        }
        return *this;
    }

    /**
     * @defgroup Deleted copy members to prevent unintended duplication.
     * @{
     */
    __FixedCallback(const __FixedCallback&) = delete;
    __FixedCallback& operator=(const __FixedCallback&) = delete;
    /** @} */

    /**
     * @brief Assign nullptr, clearing the callback.
     * 
     * @returns     Reference to this.
     */
    __FixedCallback& operator=(std::nullptr_t) noexcept
    {
        Reset();
        return *this;
    }

    /**
     * @brief Assign a new callable.
     * 
     * @tparam F    Callable type.
     * @param f     The callable to store.
     *
     * @returns     Reference to this.
     */
    template <typename F>
    __FixedCallback& operator=(F&& f) noexcept
    requires std::invocable<F> && 
             (!std::same_as<std::decay_t<F>, __FixedCallback>)
    {
        Reset();
        *this = __FixedCallback(std::forward<F>(f));
        return *this;
    }

    /**
     * @brief Invoke the stored callable.
     * 
     * @note Does nothing if the callback is null (safe to call on empty callback).
     */
    void operator()() const noexcept
    {
        if (m_invoke) {
            m_invoke(const_cast<void*>(static_cast<const void*>(m_storage)));
        }
    }

    /**
     * @brief Check if a callable is stored.
     * 
     * @returns     true if a callable is stored, false if null.
     */
    explicit operator bool() const noexcept
    {
        return m_invoke != nullptr;
    }

    /**
     * @brief Clear the stored callable.
     */
    void Reset() noexcept
    {
        if (m_destroy) {
            m_destroy(m_storage);
        }
        m_invoke = nullptr;
        m_destroy = nullptr;
        m_move = nullptr;
    }

private:
    alignas(AlignmentV) std::byte m_storage[CapacityV]{};
    void (*m_invoke)(void*) noexcept = nullptr;
    void (*m_destroy)(void*) noexcept = nullptr;
    void (*m_move)(void*, void*) noexcept = nullptr;
};

} /* namespace __Internal */

} /* namespace STM32 */

#endif /* STM32_FIXED_CALLBACK_HPP */
