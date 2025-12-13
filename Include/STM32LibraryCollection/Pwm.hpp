/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_PWM_HPP
#define STM32_PWM_HPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <span>

#include "Config.hpp"
#include "Utility.hpp"

#include "main.h"

#if !defined(HAL_TIM_MODULE_ENABLED) /* module check */
#error "HAL TIM module is not enabled!"
#endif /* module check */

namespace STM32 {
/**
 * @struct PwmInputRangeMax, A utility struct to hold maximum input range values for PWM.
 * 
 * @tparam MinV     Minimum value of the range.
 * @tparam MaxV     Maximum value of the range.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 *
 * using MyPwmInputRangeMax = STM32::PwmInputRangeMax<0, 180>;
 * auto min_value = MyPwmInputRangeMax::min_value; // min_value is 0.
 * auto max_value = MyPwmInputRangeMax::max_value; // max_value is 180.
 * @endcode
 */
template <std::uint32_t MinV, std::uint32_t MaxV>
struct PwmInputRangeMax : Range<std::uint32_t, MinV, MaxV> {};

/**
 * @struct PwmInputRange, A utility struct to hold input range values for PWM. 
 * 
 * @tparam MinV         Minimum value of the range.
 * @tparam MaxV         Maximum value of the range.
 * @tparam DefaultV     Default value within the range.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 *
 * using MyPwmInputRange = STM32::PwmInputRange<0, 180, 90>;
 * auto min_value = MyPwmInputRange::min_value; // min_value is 0.
 * auto max_value = MyPwmInputRange::max_value; // max_value is 180.
 * auto default_value = MyPwmInputRange::default_value; // default_value is 90.
 * @endcode
 */
template <std::uint32_t MinV, std::uint32_t MaxV, std::uint32_t DefaultV>
struct PwmInputRange : Range<std::uint32_t, MinV, MaxV, DefaultV> {};

/**
 * @brief IsPwmInputRange, A concept to check if a type is a PwmInputRange.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 * 
 * static_assert(STM32::IsPwmInputRange<STM32::PwmInputRange<0, 180, 90>>);
 * static_assert(!STM32::IsPwmInputRange<int>);
 * @endcode
 */
template <typename T>
concept IsPwmInputRange =
    STM32::IsRange<T> &&
    std::same_as<typename T::ValueTypeT, std::uint32_t>;

/**
 * @struct PwmDutyCycleRange, A utility struct to hold duty cycle range values for PWM.
 *
 * @tparam MinV     Minimum value of the duty cycle (in percentage).
 * @tparam MaxV     Maximum value of the duty cycle (in percentage).
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 *
 * using MyPwmDutyCycleRange = STM32::PwmDutyCycleRange<2.5, 12.>;
 * auto min_value = MyPwmDutyCycleRange::min_value; // min_value is 2.5.
 * auto max_value = MyPwmDutyCycleRange::max_value; // max_value is 12.0.
 * @endcode
 */
template <double MinV, double MaxV>
struct PwmDutyCycleRange : Range<double, MinV, MaxV> {
    static_assert(
        MinV >= 0.0 && MaxV <= 100.0,
        "Duty cycle values must be in the range [0.0, 100.0]"
    );
};

/**
 * @brief IsPwmDutyCycleRange, A concept to check if a type is a PwmDutyCycleRange.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 * 
 * static_assert(STM32::IsPwmDutyCycleRange<STM32::PwmDutyCycleRange<2.5, 12.0>>);
 * static_assert(!STM32::IsPwmDutyCycleRange<int>);
 * @endcode
 */
template <typename T>
concept IsPwmDutyCycleRange =
    STM32::IsRange<T> &&
    std::same_as<typename T::ValueTypeT, double> &&
    T::min_value >= 0. &&
    T::max_value <= 100.;

/**
 * @class Pwm, A class to manage PWM functionality on STM32 microcontrollers.
 * 
 * @tparam WorkingModeT        Working mode of the PWM
 *                             (WorkingMode::Blocking, WorkingMode::Interrupt, WorkingMode::DMA).
 * @tparam DutyCycleRangeT     Duty cycle range for the PWM.
 * @tparam InputRangeT         Input range for the PWM.
 * @tparam InputRangeMaxT      Maximum input range for the PWM.
 *
 * @note Pwm class is non-copyable and non-movable.
 *
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 * 
 * using MyPwm = STM32::Pwm<
 *     STM32::WorkingMode::Blocking,
 *     STM32::PwmDutyCycleRange<0.0, 100.0>,
 *     STM32::PwmInputRange<0, 255, 128>,
 *     STM32::PwmInputRangeMax<0, 255>
 * >;
 *
 * TIM_HandleTypeDef htim1; // Assume this is properly initialized elsewhere.
 * MyPwm pwm{htim1, TIM_CHANNEL_1};
 * pwm.Set(128); // Set PWM to mid-range value.
 * auto current_value = pwm.Get(); // Get current PWM input value.
 * @endcode
 *
 * @example Usage with DMA;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 * #include <array>
 *
 * using MyPwmDma = STM32::Pwm<
 *     STM32::WorkingMode::DMA,
 *     STM32::PwmDutyCycleRange<0.0, 100.0>,
 *     STM32::PwmInputRange<0, 255, 128>,
 *     STM32::PwmInputRangeMax<0, 255>
 * >;
 *
 * TIM_HandleTypeDef htim1; // Assume this is properly initialized elsewhere.
 * std::array<std::uint32_t, 256> dma_buffer{}; // DMA buffer for PWM values.
 * MyPwmDma pwm_dma{htim1, TIM_CHANNEL_1, dma_buffer};
 * pwm_dma.Set(128); // Set PWM to mid-range value.
 * auto current_value_dma = pwm_dma.Get(); // Get current PWM input value.
 * @endcode
 */
template <
    IsWorkingMode WorkingModeT,
    IsPwmDutyCycleRange DutyCycleRangeT,
    IsPwmInputRange InputRangeT,
    IsPwmInputRange InputRangeMaxT
>
class Pwm {
    static_assert(
        InputRangeT::min_value >= InputRangeMaxT::min_value &&
        InputRangeT::max_value <= InputRangeMaxT::max_value,
        "InputRangeT must be within InputRangeMaxT"
    );
public:
    /**
     * @brief Construct Pwm class, without DMA support.
     * 
     * @param timer_handle      Reference to the TIM handle.
     * @param timer_channel     Timer channel for PWM output.
     *
     * @note Pwm starts automatically upon construction.
     */
    Pwm(TIM_HandleTypeDef& timer_handle, std::uint32_t timer_channel) noexcept
      : m_timer_handle{timer_handle},
        m_timer_channel{timer_channel}
    {
        StartPwm();
        Set(InputRangeT::default_value);
    }

    /**
     * @brief Construct Pwm class with DMA support.
     * 
     * @tparam BufferLengthV    Length of the DMA buffer.
     * 
     * @param timer_handle      Reference to the TIM handle.
     * @param timer_channel     Timer channel for PWM output.
     * @param dma_buffer        Span representing the DMA buffer.
     *
     * @note Pwm starts automatically upon construction.
     */
    template <std::uint16_t BufferLengthV>
    Pwm(
        TIM_HandleTypeDef& timer_handle,
        std::uint32_t timer_channel,
        std::span<std::uint32_t, BufferLengthV> dma_buffer
    ) noexcept requires std::same_as<WorkingModeT, WorkingMode::DMA>
      : m_timer_handle{timer_handle},
        m_timer_channel{timer_channel},
        m_dma_buffer{dma_buffer}
    {
        StartPwm();
        Set(InputRangeT::default_value);
    }

    /**
     * @brief Destroy the Pwm object, stops PWM.
     */
    ~Pwm()
    {
        StopPwm();
    }

    /**
     * @returns TIM handle reference.
     */
    [[nodiscard]]
    TIM_HandleTypeDef& GetTimerHandle() const noexcept
    {
        return m_timer_handle;
    }

    /**
     * @returns Timer channel.
     */
    [[nodiscard]]
    std::uint32_t GetTimerChannel() const noexcept
    {
        return m_timer_channel;
    }

    /**
     * @returns Current input value corresponding to the PWM duty cycle.
     */
    [[nodiscard]]
    std::uint32_t Get() const noexcept
    {
        return ConvertToInput(
            __HAL_TIM_GET_COMPARE(&m_timer_handle, m_timer_channel)
        );
    }

    /**
     * @brief Set the PWM duty cycle based on the input value.
     * 
     * @param input     Input value to set the PWM duty cycle.
     */
    void Set(double input) noexcept
    {
        __HAL_TIM_SET_COMPARE(
            &m_timer_handle,
            m_timer_channel,
            ConvertToPwm(
                std::clamp(
                    input,
                    static_cast<double>(InputRangeT::min_value),
                    static_cast<double>(InputRangeT::max_value)
                )
            )
        );
    }

private:
    TIM_HandleTypeDef& m_timer_handle;
    std::uint32_t m_timer_channel;
    std::span<std::uint32_t> m_dma_buffer{};
    std::uint32_t m_pwm_resolution{
        m_timer_handle.Init.Period + 1
    };
    double m_min_pwm_value{
        (m_pwm_resolution * DutyCycleRangeT::min_value) / 100.
    };
    double m_max_pwm_value{
        (m_pwm_resolution * DutyCycleRangeT::max_value) / 100.
    };
    double m_pwm_value_resolution{
        m_max_pwm_value - m_min_pwm_value
    };

    /**
     * @brief Convert input value to PWM value.
     * 
     * @param input     Input value to convert.
     * 
     * @returns Corresponding PWM value.
     */
    constexpr auto ConvertToPwm(double input) const noexcept
    {
        return static_cast<std::uint32_t>(
            m_min_pwm_value +
            (
                (input - InputRangeMaxT::min_value) / 
                (InputRangeMaxT::max_value - InputRangeMaxT::min_value)
            ) *
            m_pwm_value_resolution
        );
    }

    /**
     * @brief Convert PWM value to input value.
     * 
     * @param pwm_value     PWM value to convert.
     * 
     * @returns Corresponding input value.
     */
    constexpr auto ConvertToInput(int pwm_value) const noexcept
    {
        return static_cast<std::uint32_t>(std::round(
            (InputRangeMaxT::max_value - InputRangeMaxT::min_value) *
            ((pwm_value - m_min_pwm_value) / m_pwm_value_resolution)) +
            InputRangeMaxT::min_value
        );
    }

    /**
     * @brief Start PWM based on the working mode.
     */
    void StartPwm() noexcept
    {
        if constexpr (std::same_as<WorkingModeT, WorkingMode::Blocking>){
            HAL_TIM_PWM_Start(&m_timer_handle, m_timer_channel);
        } else if constexpr (std::same_as<WorkingModeT, WorkingMode::Interrupt>){
            HAL_TIM_PWM_Start_IT(&m_timer_handle, m_timer_channel);
        } else if constexpr (std::same_as<WorkingModeT, WorkingMode::DMA>){
            HAL_TIM_PWM_Start_DMA(
                &m_timer_handle, m_timer_channel, m_dma_buffer.data(), m_dma_buffer.size()
            );
        }
    }

    /**
     * @brief Stop PWM based on the working mode.
     */
    void StopPwm() noexcept
    {
        if constexpr (std::same_as<WorkingModeT, WorkingMode::Blocking>){
            HAL_TIM_PWM_Stop(&m_timer_handle, m_timer_channel);
        } else if constexpr (std::same_as<WorkingModeT, WorkingMode::Interrupt>){
            HAL_TIM_PWM_Stop_IT(&m_timer_handle, m_timer_channel);
        } else if constexpr (std::same_as<WorkingModeT, WorkingMode::DMA>){
            HAL_TIM_PWM_Stop_DMA(&m_timer_handle, m_timer_channel);
        }
    }
};

} /* namespace STM32 */

#endif /* STM32_PWM_HPP */
