/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
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
 * @class Timer, A class to manage Blocking Mode Timer functionality on STM32 microcontrollers.
 * 
 * @note Timer class is non-copyable and non-movable.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Timer.hpp>
 *
 * TIM_HandleTypeDef htim1; // Assume this is properly initialized elsewhere.
 * STM32::Timer timer{htim1};
 * timer.SleepFor(1000); // Sleep for 1000 timer ticks.
 * auto time_point = timer.Get(); // Get current timer value.
 * timer.Reset(); // Reset timer counter to 0.
 * timer.Set(500); // Set timer counter to 500.
 * timer.SleepUntil(1000); // Sleep until timer counter reaches 1000.
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
     * @returns Current timer counter value.
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
     * @brief Reset the timer counter and sleep for a specific duration in timer ticks.
     * 
     * @param duration      Duration in timer ticks to sleep.
     */
    void SleepFor(std::uint32_t duration) noexcept
    {
        Reset();
        while(Get() < duration);
    }

    /**
     * @brief Sleep until the timer counter reaches a specific value.
     * 
     * @param time_point     Timer counter value to sleep until.
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
