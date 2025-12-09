/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_TIMER_HPP
#define STM32_TIMER_HPP

#include <cstdint>

#include "main.h"

#if !defined(HAL_TIM_MODULE_ENABLED) /* module check */
static_assert(false, "HAL TIM module is not enabled!");
#endif /* module check */

static_assert(__cplusplus >= 201703L, "C++17 required!");

namespace STM32 {

class timer {
public:
	timer(TIM_HandleTypeDef& timer_handle) noexcept
	: m_timer_handle{&timer_handle}
	{
		HAL_TIM_Base_Start(m_timer_handle);
	}

	timer(const timer&) = delete;
	timer& operator=(const timer&) = delete;
	timer(timer&&) = delete;
	timer& operator=(timer&&) = delete;

	~timer()
	{
		HAL_TIM_Base_Stop(m_timer_handle);
	}

	[[nodiscard]]
	std::uint32_t get() const noexcept
	{
		return __HAL_TIM_GET_COUNTER(m_timer_handle);
	}

	[[nodiscard]]
	TIM_HandleTypeDef* get_timer_handle() const noexcept
	{
		return m_timer_handle;
	}

	void reset() noexcept
	{
		set(0);
	}

	void set(std::uint32_t time_point) noexcept
	{
		__HAL_TIM_SET_COUNTER(m_timer_handle, time_point);
	}

	void sleep_for(std::uint32_t duration) noexcept
	{
		reset();
		while(get() < duration);
	}

private:
	TIM_HandleTypeDef* m_timer_handle;
};

/**
  * Example;
  *
  * stm32::timer us_timer(htim1);
  * us_timer.sleep_for(123);
  */

/*
 * TIM5 specs for STM32F4-DISC;
 * timer frequency must be 1'000'000 Hz for us_timer;
 * peripheral_frequency     = 84'000'000;
 * prescaler                = 83;
 * timer_frequency          = 1'000'000;
 * counter_period = 4'294'967'295;
 */

/*
 * TIM1 specs for NUCLEO-F446RE;
 * timer frequency must be 1'000'000 Hz for us_timer;
 * peripheral_frequency     = 90'000'000;
 * prescaler                = 89;
 * timer_frequency          = 1'000'000;
 * counter_period = 65'535;
 */

} /* namespace STM32 */

#endif /* STM32_TIMER_HPP */
