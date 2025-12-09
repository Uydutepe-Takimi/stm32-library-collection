/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_L298N_HPP
#define STM32_L298N_HPP

#include <Gpio.hpp>
#include <Timer.hpp>

namespace STM32 {

class l298n_linear_motor {
public:
	l298n_linear_motor(
		gpio_output& forward_pin,
		gpio_output& backward_pin,
		timer& us_timer) noexcept
	: m_forward_pin{&forward_pin},
	  m_backward_pin{&backward_pin},
	  m_us_timer{&us_timer}
	{
		stop();
	}

	l298n_linear_motor(const l298n_linear_motor&) = delete;
	l298n_linear_motor& operator=(const l298n_linear_motor&) = delete;
	l298n_linear_motor(l298n_linear_motor&&) = delete;
	l298n_linear_motor& operator=(l298n_linear_motor&&) = delete;

	~l298n_linear_motor()
	{
		stop();
	}

	void backward() noexcept
	{
		m_forward_pin->write(Low);
		m_backward_pin->write(High);
	}

	void backward_for(std::uint32_t duration) noexcept
	{
		m_forward_pin->write(Low);
		m_backward_pin->write(High);
		m_us_timer->sleep_for(duration);
		stop();
	}

	void forward() noexcept
	{
		m_forward_pin->write(High);
		m_backward_pin->write(Low);
	}

	void forward_for(std::uint32_t duration) noexcept
	{
		m_forward_pin->write(High);
		m_backward_pin->write(Low);
		m_us_timer->sleep_for(duration);
		stop();
	}

	void stop() noexcept
	{
		m_forward_pin->write(Low);
		m_backward_pin->write(Low);
	}

private:
	gpio_output* m_forward_pin;
	gpio_output* m_backward_pin;
	timer* m_us_timer;
};

} /* namespace STM32 */

#endif /* STM32_L298N_HPP */
