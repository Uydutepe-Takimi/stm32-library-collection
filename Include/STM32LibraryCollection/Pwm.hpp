/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_PWM_HPP
#define STM32_PWM_HPP

#include <cmath>
#include <cstdint>
#include <algorithm>

#include "main.h"

#if !defined(HAL_TIM_MODULE_ENABLED) /* module check */
static_assert(false, "HAL TIM module is not enabled!");
#endif /* module check */

static_assert(__cplusplus > 201703L, "C++20 required!");

namespace STM32 {
#if __cpp_nontype_template_args < 201911L
/* the compiler does not support double as a non-type template parameter */

/**
  * @brief A helper class to use double as a non-type template parameter
  *
  */
struct stm32_double {
	consteval stm32_double(double value) noexcept
        : m_power_of_ten{
            [](double d){
                if (d < 0){
                	d *= -1;
                }
                std::int32_t exponent{};
                while (true){
                    auto difference = d - static_cast<int>(d);
                    if (difference == 0.0){
                        break;
                    }
                    d *= ten;
                    ++exponent;
                }
                return exponent;
            }(value)
        },
		m_value{static_cast<std::int32_t>(
			value * power(ten, m_power_of_ten)
        )}
    { }

    consteval operator double() const noexcept
    {
        return m_value/power(ten, m_power_of_ten);
    }

    static consteval double power(double base, double exponent) noexcept
    {
        while (exponent--){
            base *= base;
        }
        return base;
    }

    std::int32_t m_power_of_ten;
    std::int32_t m_value;
    static constexpr double ten = 10.;
};

/**
  * @brief A pwm class for STM32 development boards
  *
  * Note: A pwm class object must be defined
  * 	  after all the initializing operations,
  * 	  for example, in the USER CODE 2 section.
  */
template <std::uint32_t InputRangeBegin, std::uint32_t InputRangeEnd,
		  std::uint32_t MinInput, std::uint32_t MaxInput, std::uint32_t DefaultInput,
		  stm32_double MinPWMDuty, stm32_double MaxPWMDuty>
#else
/* the compiler supports double as a non-type template parameter  */

template <std::uint32_t InputRangeBegin, std::uint32_t InputRangeEnd,
		  std::uint32_t MinInput, std::uint32_t MaxInput, std::uint32_t DefaultInput,
		  double MinPWMDuty, double MaxPWMDuty>

#endif /* non-type template parameter check */
class pwm {
public:
	static_assert(
		InputRangeBegin <= MinInput,
		"the minimum input cannot be less than the input range begin!"
	);
	static_assert(
		MaxInput <= InputRangeEnd,
		"the maximum input cannot be greater than the input range end!"
	);
	static_assert(
		InputRangeBegin <= InputRangeEnd,
		"the input range begin cannot be greater than the input range end!"
	);
	static_assert(
		MinInput <= MaxInput,
		"the minimum input cannot be greater than the maximum input!"
	);
	static_assert(
		MinInput <= DefaultInput && DefaultInput <= MaxInput,
		"the default input must be in the range between the minimum and the maximum inputs!"
	);
	static_assert(
		0 <= MinPWMDuty,
		"the minimum pwm duty cycle percentage cannot be negative!"
	);
	static_assert(
		MaxPWMDuty <= 100,
		"the maximum pwm duty cycle percentage cannot be greater than 100!"
	);

	pwm(TIM_HandleTypeDef& timer_handle, std::uint32_t timer_channel) noexcept
	: m_timer_handle{&timer_handle},
	  m_timer_channel{timer_channel},
	  m_pwm_resolution{m_timer_handle->Init.Period + 1}
	{
		HAL_TIM_PWM_Start(m_timer_handle, m_timer_channel);
		set(DefaultInput);
	}

	pwm(const pwm&) = delete;
	pwm& operator=(const pwm&) = delete;
	pwm(pwm&&) = delete;
	pwm& operator=(pwm&&) = delete;

	~pwm()
	{
		HAL_TIM_PWM_Stop(m_timer_handle, m_timer_channel);
	}

	[[nodiscard]]
	TIM_HandleTypeDef* get_timer_handle() const noexcept
	{
		return m_timer_handle;
	}

	[[nodiscard]]
	std::uint32_t get_timer_channel() const noexcept
	{
		return m_timer_channel;
	}

	[[nodiscard]]
	std::uint32_t get() const noexcept
	{
		return convert_to_input(
			__HAL_TIM_GET_COMPARE(m_timer_handle, m_timer_channel)
		);
	}

	void set(double input) noexcept
	{
		input = std::clamp(
			input,
			static_cast<double>(MinInput),
			static_cast<double>(MaxInput)
		);
		__HAL_TIM_SET_COMPARE(
			m_timer_handle,
			m_timer_channel,
			convert_to_pwm(input)
		);
	}

private:
	TIM_HandleTypeDef* m_timer_handle;
	std::uint32_t m_timer_channel;
	std::uint32_t m_pwm_resolution;
	double m_min_pwm_value{
		(m_pwm_resolution * MinPWMDuty) / 100.
	};
	double m_max_pwm_value{
		(m_pwm_resolution * MaxPWMDuty) / 100.
	};
	double m_pwm_value_resolution{
		m_max_pwm_value - m_min_pwm_value
	};

	constexpr int convert_to_pwm(double input) const noexcept
	{
		return static_cast<int>(
			m_min_pwm_value +
			((input - InputRangeBegin) / (InputRangeEnd - InputRangeBegin)) *
			m_pwm_value_resolution
		);
	}

	constexpr auto convert_to_input(int pwm_value) const noexcept
	{
		return static_cast<std::uint32_t>(std::round(
			(InputRangeEnd - InputRangeBegin) *
			((pwm_value - m_min_pwm_value) / m_pwm_value_resolution)) +
			InputRangeBegin
		);
	}
};

/**
  * @brief type alias for servo motors
  *
  */
using servo = pwm<0, 180, 0, 180, 90, 2.5, 12.>;

/**
  * Example;
  *
  * stm32::servo vertical_servo(htim2, TIM_CHANNEL_1);
  * vertical_servo.set(125);
  */

/**
  * TIM2 specs for STM32F4-DISC at 168 MHz for servo motors;
  * peripheral_frequency = 84'000'000;
  * prescaler            = 27;
  * counter_period		 = 59'999;
  */

/**
  * TIM2 specs for NUCLEO-F446RE at 180 MHz for servo motors;
  * peripheral_frequency = 90'000'000;
  * prescaler            = 29;
  * counter_period		 = 59'999;
  */

} /* namespace STM32 */

#endif /* STM32_PWM_HPP */
