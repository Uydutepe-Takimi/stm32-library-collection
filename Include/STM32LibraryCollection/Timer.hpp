/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_TIMER_HPP
#define STM32_TIMER_HPP

#include <cstdint>
#include <span>

#include "Config.hpp"

#include "main.h"

#if !defined(HAL_TIM_MODULE_ENABLED) /* module check */
#error "HAL TIM module is not enabled!"
#endif /* module check */

namespace STM32 {
/**
 * @class Timer, A class to manage Timer functionality on STM32 microcontrollers.
 * 
 * @tparam WorkingModeT        Working mode of the Timer
 *                             (WorkingMode::Blocking, WorkingMode::Interrupt, WorkingMode::DMA).
 *
 * @note Timer class is non-copyable and non-movable.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Timer.hpp>
 *
 * using MyTimer = STM32::Timer<STM32::WorkingMode::Blocking>;
 * 
 * TIM_HandleTypeDef htim1; // Assume this is properly initialized elsewhere.
 * MyTimer timer{htim1};
 * timer.SleepFor(1000); // Sleep for 1000 timer ticks.
 * auto time_point = timer.Get(); // Get current timer value.
 * timer.Reset(); // Reset timer counter to 0.
 * timer.Set(500); // Set timer counter to 500.
 * timer.SleepUntil(1000); // Sleep until timer counter reaches 1000.
 * @endcode
 *
 * @example Usage with DMA;
 * @code {.cpp}
 * #include <array>
 * #include <STM32LibraryCollection/Timer.hpp>
 *
 * using MyTimerDma = STM32::Timer<STM32::WorkingMode::DMA>;
 * 
 * TIM_HandleTypeDef htim1; // Assume this is properly initialized elsewhere.
 * std::array<std::uint32_t, 256> dma_buffer{}; // DMA buffer for timer values.
 * MyTimerDma timer_dma{htim1, dma_buffer};
 * timer_dma.SleepFor(1000); // Sleep for 1000 timer ticks.
 * @endcode
 */
template <IsWorkingMode WorkingModeT>
class Timer {
public:
    /**
     * @brief Construct Timer class, without DMA support.
     * 
     * @param handle    Reference to the TIM handle.
     *
     * @note Timer starts automatically upon construction.
     */
    explicit Timer(TIM_HandleTypeDef& handle) noexcept
      : m_handle{handle}
    {
        StartTimer();
    }

    /**
     * @brief Construct Timer class with DMA support.
     * 
     * @tparam BufferLengthV    Length of the DMA buffer.
     * 
     * @param handle        Reference to the TIM handle.
     * @param dma_buffer    Span representing the DMA buffer.
     *
     * @note Timer starts automatically upon construction.
     */
    template <std::uint16_t BufferLengthV>
    Timer(TIM_HandleTypeDef& handle, std::span<std::uint32_t, BufferLengthV> dma_buffer)
    noexcept requires std::same_as<WorkingModeT, WorkingMode::DMA>
      : m_handle{handle},
        m_dma_buffer{dma_buffer}
    {
        StartTimer();
    }

    /**
     * @brief Destroy the Timer object, stops the timer.
     */
    ~Timer()
    {
        StopTimer();
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
    TIM_HandleTypeDef& GetHandle() const noexcept
    {
        return m_handle;
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
        while(Get() < time_point);
    }

private:
    TIM_HandleTypeDef& m_handle;
    std::span<std::uint32_t> m_dma_buffer{};

    /**
     * @brief Start Timer based on the working mode.
     */
    void StartTimer() noexcept
    {
        if constexpr (std::same_as<WorkingModeT, WorkingMode::Blocking>){
            HAL_TIM_Base_Start(&m_handle);
        } else if constexpr (std::same_as<WorkingModeT, WorkingMode::Interrupt>){
            HAL_TIM_Base_Start_IT(&m_handle);
        } else if constexpr (std::same_as<WorkingModeT, WorkingMode::DMA>){
            HAL_TIM_Base_Start_DMA(&m_handle, m_dma_buffer.data(), m_dma_buffer.size());
        }
    }

    /**
     * @brief Stop Timer based on the working mode.
     */
    void StopTimer() noexcept
    {
        if constexpr (std::same_as<WorkingModeT, WorkingMode::Blocking>){
            HAL_TIM_Base_Stop(&m_handle);
        } else if constexpr (std::same_as<WorkingModeT, WorkingMode::Interrupt>){
            HAL_TIM_Base_Stop_IT(&m_handle);
        } else if constexpr (std::same_as<WorkingModeT, WorkingMode::DMA>){
            HAL_TIM_Base_Stop_DMA(&m_handle);
        }
    }
};

} /* namespace STM32 */

#endif /* STM32_TIMER_HPP */
