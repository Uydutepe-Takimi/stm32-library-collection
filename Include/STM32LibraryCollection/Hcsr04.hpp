/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_HCSR04_HPP
#define STM32_HCSR04_HPP

#include <Gpio.hpp>
#include <Timer.hpp>

#if !defined(HCSR04_OUTPUT_VOLTAGE_REDUCED)
static_assert(false,
	"the output voltage of the sensor is 5V! "
	"reduce the output voltage by using voltage divider or "
	"use a 5V tolerant pin as an input pin. "
	"define HCSR04_OUTPUT_VOLTAGE_REDUCED to skip this error."
);
#endif

namespace STM32 {

class hcsr04 {
public:
	hcsr04(
		timer& us_timer,
		GpioInput& input_pin,
		GpioOutput& output_pin) noexcept
	: m_us_timer{&us_timer},
	  m_input_pin{&input_pin},
	  m_output_pin{&output_pin}
	{ }

	hcsr04(const hcsr04&) = delete;
	hcsr04& operator=(const hcsr04&) = delete;
	hcsr04(hcsr04&&) = delete;
	hcsr04& operator=(hcsr04&&) = delete;

	[[nodiscard]]
	std::uint16_t get_distance() const noexcept
	{
		m_output_pin->Write(GpioPinState::High);
		m_us_timer->sleep_for(initial_delay);
		m_output_pin->Write(GpioPinState::Low);
		m_us_timer->reset();
		while (m_input_pin->Read() != GpioPinState::High){
			if (m_us_timer->get() >= max_counter_value){
				return max_distance;
			}
		}
		m_us_timer->reset();
		while (m_input_pin->Read() != GpioPinState::Low){
			if (m_us_timer->get() >= max_counter_value){
				return max_distance;
			}
		}
		auto distance = static_cast<std::uint16_t>(
			m_us_timer->get() / coefficient
		);
		m_us_timer->sleep_for(new_measurement_delay/2);
		m_us_timer->sleep_for(new_measurement_delay/2);
		return distance;
	}

private:
	timer* m_us_timer;
	GpioInput* m_input_pin;
	GpioOutput* m_output_pin;

	static constexpr auto coefficient          = 58.;
	static constexpr int max_distance          = 400; 		/* cm */
	static constexpr int max_counter_value     = 30'000;
	static constexpr int initial_delay         = 10;		/* us */
	static constexpr int new_measurement_delay = 100'000;	/* us */
};

} /* namespace STM32 */

#endif /* STM32_HCSR04_HPP */
