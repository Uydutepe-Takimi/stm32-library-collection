/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_TIMER_HPP
#define STM32_TIMER_HPP

#include <cstdint>
#include <utility>

#include "main.h"

#if !defined(HAL_TIM_MODULE_ENABLED) /* module check */
#error "HAL TIM module is not enabled!"
#endif /* module check */

namespace STM32 {

/**
 * @class Timer, A class to manage hardware timer functionality on STM32 microcontrollers.
 * 
 * Provides a simple interface for time measurement and delays using hardware timers.
 * The time unit depends on the timer's clock configuration (prescaler and clock source).
 * 
 * @note Timer class is non-copyable and non-movable.
 * @note Timer starts automatically upon construction and stops on destruction.
 * @note SleepFor() and SleepUntil() are blocking operations.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Timer.hpp>
 *
 * TIM_HandleTypeDef htim2; // Assume properly initialized (e.g., 1MHz = 1us ticks)
 * 
 * STM32::Timer timer{htim2};
 * 
 * // Time measurement
 * timer.Reset();             // Start timing
 * // ... do something ...
 * auto elapsed = timer.Get(); // Get elapsed ticks
 * 
 * // Blocking delays
 * timer.SleepFor(1000);      // Wait for 1000 ticks (1ms at 1MHz)
 * timer.SleepUntil(5000);    // Wait until counter reaches 5000
 * 
 * // Direct counter manipulation
 * timer.Set(0);              // Set counter to specific value
 * timer.Reset();             // Shorthand for Set(0)
 * @endcode
 */
class Timer {
public:

    /**
     * @brief Construct Timer class.
     * 
     * @param handle    Reference to the TIM handle.
     *
     * @note Timer starts automatically upon construction.
     */
    explicit Timer(TIM_HandleTypeDef& handle) noexcept
      : m_handle{handle}
    {
        HAL_TIM_Base_Start(&m_handle);
    }

    /**
     * @defgroup Deleted copy and move members.
     * @{
     */
    Timer(const Timer&) = delete;
    Timer& operator=(const Timer&) = delete;
    Timer(Timer&&) = delete;
    Timer& operator=(Timer&&) = delete;
    /** @} */

    /**
     * @brief Destroy the Timer object, stops the timer.
     */
    ~Timer()
    {
        HAL_TIM_Base_Stop(&m_handle);
    }

    /**
     * @brief Get current timer counter value.
     * 
     * @returns Current counter value in timer ticks.
     */
    [[nodiscard]]
    std::uint32_t Get() const noexcept
    {
        return __HAL_TIM_GET_COUNTER(&m_handle);
    }

    /**
     * @returns TIM handle reference.
     */
    [[nodiscard]]
    auto&& GetHandle(this auto&& self) noexcept
    {
        return std::forward<decltype(self)>(self).m_handle;
    }

    /**
     * @brief Reset the timer counter to zero.
     * 
     * Equivalent to Set(0).
     */
    void Reset() noexcept
    {
        Set(0);
    }

    /**
     * @brief Set the timer counter to a specific value.
     * 
     * @param time_point     Value to set the timer counter to.
     */
    void Set(std::uint32_t time_point) noexcept
    {
        __HAL_TIM_SET_COUNTER(&m_handle, time_point);
    }

    /**
     * @brief Reset the timer and block for a specific duration.
     * 
     * Resets the counter to 0, then blocks until the counter reaches the
     * specified duration.
     * 
     * @param duration      Number of timer ticks to wait.
     * 
     * @warning This is a blocking operation.
     */
    void SleepFor(std::uint32_t duration) noexcept
    {
        Reset();
        while(Get() < duration);
    }

    /**
     * @brief Block until the timer counter reaches a specific value.
     * 
     * If the counter has already passed the target, returns immediately.
     * 
     * @param time_point     Timer counter value to wait for.
     * 
     * @warning This is a blocking operation.
     */
    void SleepUntil(std::uint32_t time_point) noexcept
    {
        const auto current = Get();
        if (time_point <= current){
            return;
        }
        if (time_point < current){
            while (Get() >= current);
        }
        while (Get() < time_point);
    }

private:
    TIM_HandleTypeDef& m_handle;
};

} /* namespace STM32 */

#endif /* STM32_TIMER_HPP */
