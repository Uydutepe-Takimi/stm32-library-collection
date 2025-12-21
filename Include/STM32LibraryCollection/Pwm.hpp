/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_PWM_HPP
#define STM32_PWM_HPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <utility>

#include "__Internal/__Utility.hpp"

#include "main.h"

#if !defined(HAL_TIM_MODULE_ENABLED) /* module check */
#error "HAL TIM module is not enabled!"
#endif /* module check */

namespace STM32 {

/**
 * @struct PwmInputRangeMax, A utility struct defining the absolute limits for PWM input values.
 * 
 * @tparam MinV     Absolute minimum value allowed.
 * @tparam MaxV     Absolute maximum value allowed.
 *
 * This defines the hardware or logical limits. PwmInputRange must be within these bounds.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 *
 * // Servo motors typically accept 0-180 degrees
 * using ServoRange = STM32::PwmInputRangeMax<0, 180>;
 *
 * // LED brightness 0-255
 * using LedRange = STM32::PwmInputRangeMax<0, 255>;
 * @endcode
 */
template <std::uint32_t MinV, std::uint32_t MaxV>
struct PwmInputRangeMax : __Internal::__Range<std::uint32_t, MinV, MaxV> {};

/**
 * @struct PwmInputRange, A utility struct defining the operational range for PWM input values.
 * 
 * @tparam MinV         Minimum operational value.
 * @tparam MaxV         Maximum operational value.
 * @tparam DefaultV     Default/initial value (must be within [MinV, MaxV]).
 *
 * This defines the range actually used by Set(). Values are clamped to this range.
 * Must be within the bounds defined by PwmInputRangeMax.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 *
 * // Servo: 0-180 degrees, starts at 90 (center)
 * using ServoInput = STM32::PwmInputRange<0, 180, 90>;
 *
 * // Throttle: 0-100%, starts at 0 (off)
 * using ThrottleInput = STM32::PwmInputRange<0, 100, 0>;
 * @endcode
 */
template <std::uint32_t MinV, std::uint32_t MaxV, std::uint32_t DefaultV>
struct PwmInputRange : __Internal::__Range<std::uint32_t, MinV, MaxV, DefaultV> {};

/**
 * @brief IsPwmInputRange, A concept to check if a type is a PwmInputRange.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 * 
 * static_assert(STM32::IsPwmInputRange<STM32::PwmInputRange<0, 180, 90>>);
 * static_assert(!STM32::IsPwmInputRange<int>);
 * @endcode
 */
template <typename T>
concept IsPwmInputRange =
    __Internal::__IsRange<T> &&
    std::same_as<typename T::ValueTypeT, std::uint32_t>;

/**
 * @struct PwmDutyCycleRange, A utility struct defining the duty cycle range for PWM output.
 *
 * @tparam MinV     Minimum duty cycle percentage (0.0-100.0).
 * @tparam MaxV     Maximum duty cycle percentage (0.0-100.0).
 *
 * The input range is linearly mapped to this duty cycle range.
 * Common use cases:
 * - Servo motors: 2.5% to 12.5% (1ms to 2.5ms pulse at 50Hz)
 * - LED dimming: 0% to 100%
 * - Motor control: 0% to 100%
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 *
 * // Standard servo: 2.5% - 12.5% duty cycle
 * using ServoDuty = STM32::PwmDutyCycleRange<2.5, 12.5>;
 *
 * // Full range LED: 0% - 100% duty cycle
 * using LedDuty = STM32::PwmDutyCycleRange<0.0, 100.0>;
 * @endcode
 */
template <double MinV, double MaxV>
struct PwmDutyCycleRange : __Internal::__Range<double, MinV, MaxV> {
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
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 * 
 * static_assert(STM32::IsPwmDutyCycleRange<STM32::PwmDutyCycleRange<2.5, 12.0>>);
 * static_assert(!STM32::IsPwmDutyCycleRange<int>);
 * @endcode
 */
template <typename T>
concept IsPwmDutyCycleRange =
    __Internal::__IsRange<T> &&
    std::same_as<typename T::ValueTypeT, double> &&
    T::min_value >= 0. &&
    T::max_value <= 100.;

/**
 * @struct PwmConfig, A utility struct bundling all PWM configuration parameters.
 * 
 * @tparam PwmDutyCycleRangeT    Duty cycle output range (in percentage).
 * @tparam PwmInputRangeT        Operational input range with default value.
 * @tparam PwmInputRangeMaxT     Absolute input range limits.
 *
 * @note PwmInputRangeT must be within bounds of PwmInputRangeMaxT.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 *
 * // Servo configuration: 0-180 degrees mapped to 2.5%-12.5% duty
 * using ServoConfig = STM32::PwmConfig<
 *     STM32::PwmDutyCycleRange<2.5, 12.5>,
 *     STM32::PwmInputRange<0, 180, 90>,
 *     STM32::PwmInputRangeMax<0, 180>
 * >;
 *
 * // LED configuration: 0-255 mapped to 0%-100% duty
 * using LedConfig = STM32::PwmConfig<
 *     STM32::PwmDutyCycleRange<0.0, 100.0>,
 *     STM32::PwmInputRange<0, 255, 0>,
 *     STM32::PwmInputRangeMax<0, 255>
 * >;
 * @endcode
 */
template <
    IsPwmDutyCycleRange PwmDutyCycleRangeT,
    IsPwmInputRange PwmInputRangeT,
    IsPwmInputRange PwmInputRangeMaxT
>
struct PwmConfig : PwmDutyCycleRangeT, PwmInputRangeT, PwmInputRangeMaxT {
    using DutyCycleRangeT = PwmDutyCycleRangeT;
    using InputRangeT = PwmInputRangeT;
    using InputRangeMaxT = PwmInputRangeMaxT;
    static_assert(
        InputRangeT::min_value >= InputRangeMaxT::min_value &&
        InputRangeT::max_value <= InputRangeMaxT::max_value,
        "PwmInputRangeT must be within PwmInputRangeMaxT"
    );
};

/**
 * @brief IsPwmConfig, A concept to check if a type is a valid PwmConfig.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 * 
 * using MyPwmConfig = STM32::PwmConfig<
 *     STM32::PwmDutyCycleRange<2.5, 12.0>,
 *     STM32::PwmInputRange<0, 180, 90>,
 *     STM32::PwmInputRangeMax<0, 180>
 * >;
 * 
 * static_assert(STM32::IsPwmConfig<MyPwmConfig>);
 * static_assert(!STM32::IsPwmConfig<int>);
 * @endcode
 */
template <typename T>
concept IsPwmConfig =
    IsPwmDutyCycleRange<typename T::DutyCycleRangeT> &&
    IsPwmInputRange<typename T::InputRangeT> &&
    IsPwmInputRange<typename T::InputRangeMaxT> &&
    T::InputRangeT::min_value >= T::InputRangeMaxT::min_value &&
    T::InputRangeT::max_value <= T::InputRangeMaxT::max_value &&
    requires {
        typename T::DutyCycleRangeT;
        typename T::InputRangeT;
        typename T::InputRangeMaxT;
    };

/**
 * @class Pwm, A class to manage PWM output on STM32 microcontrollers.
 * 
 * @tparam PwmConfigT   PWM configuration type defining input/output ranges.
 *
 * @note Pwm class is non-copyable and non-movable.
 * @note PWM starts automatically upon construction with the default value.
 * @note PWM stops automatically upon destruction.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Pwm.hpp>
 *
 * TIM_HandleTypeDef htim1; // Assume properly initialized by CubeMX
 *
 * // LED brightness control (0-255 input, 0-100% duty)
 * using LedPwm = STM32::Pwm<STM32::PwmConfig<
 *     STM32::PwmDutyCycleRange<0.0, 100.0>,
 *     STM32::PwmInputRange<0, 255, 0>,
 *     STM32::PwmInputRangeMax<0, 255>
 * >>;
 * LedPwm led{htim1, TIM_CHANNEL_1};
 * led.Set(128);                    // Set to ~50% brightness
 * auto brightness = led.Get();     // Read current value
 *
 * // For servo control, use the Servo typedef instead
 * @endcode
 */
template <IsPwmConfig PwmConfigT>
class Pwm {
public:

    /**
     * @brief Construct Pwm class.
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
        HAL_TIM_PWM_Start(&m_timer_handle, m_timer_channel);
        Set(PwmConfigT::InputRangeT::default_value);
    }

    /**
     * @defgroup Deleted copy and move members.
     * @{
     */
    Pwm(const Pwm&) = delete;
    Pwm& operator=(const Pwm&) = delete;
    Pwm(Pwm&&) = delete;
    Pwm& operator=(Pwm&&) = delete;
    /** @} */

    /**
     * @brief Destroy the Pwm object, stops PWM.
     */
    ~Pwm()
    {
        HAL_TIM_PWM_Stop(&m_timer_handle, m_timer_channel);
    }

    /**
     * @returns TIM handle reference.
     */
    [[nodiscard]]
    auto&& GetTimerHandle(this auto&& self) noexcept
    {
        return std::forward<decltype(self)>(self).m_timer_handle;
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
                    static_cast<double>(PwmConfigT::InputRangeT::min_value),
                    static_cast<double>(PwmConfigT::InputRangeT::max_value)
                )
            )
        );
    }

private:
    TIM_HandleTypeDef& m_timer_handle;
    std::uint32_t m_timer_channel;
    std::uint32_t m_pwm_resolution{
        m_timer_handle.Init.Period + 1
    };
    double m_min_pwm_value{
        (m_pwm_resolution * PwmConfigT::DutyCycleRangeT::min_value) / 100.
    };
    double m_max_pwm_value{
        (m_pwm_resolution * PwmConfigT::DutyCycleRangeT::max_value) / 100.
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
                (input - PwmConfigT::InputRangeMaxT::min_value) / 
                (PwmConfigT::InputRangeMaxT::range_size)
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
            (PwmConfigT::InputRangeMaxT::range_size) *
            ((pwm_value - m_min_pwm_value) / m_pwm_value_resolution)) +
            PwmConfigT::InputRangeMaxT::min_value
        );
    }
};

} /* namespace STM32 */

#endif /* STM32_PWM_HPP */
