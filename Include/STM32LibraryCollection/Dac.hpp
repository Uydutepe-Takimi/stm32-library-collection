/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_DAC_HPP
#define STM32_DAC_HPP

#include <cstdint>
#include <algorithm>

#include "main.h"

#if !defined(HAL_DAC_MODULE_ENABLED) /* module check */
static_assert(false, "HAL DAC module is not enabled!");
#endif /* module check */

static_assert(__cplusplus >= 201703L, "C++17 required!");

namespace STM32 {

template <std::uint32_t MinOutput = 0, std::uint32_t MaxOutput = 100,
		unsigned int DacAlignment = DAC_ALIGN_12B_R>
class dac {
public:
	dac(DAC_HandleTypeDef& dac_handle,
		std::uint32_t dac_channel,
		double dac_resolution  = 4095.) noexcept
	: m_dac_handle{&dac_handle},
	  m_dac_channel{dac_channel},
	  m_dac_resolution{dac_resolution}
	{
		static_assert(
			MinOutput <= MaxOutput,
			"the minimum output cannot be greater than the maximum output!"
		);
		HAL_DAC_Start(m_dac_handle, m_dac_channel);
		set(MinOutput);
	}

	dac(const dac&) = delete;
	dac& operator=(const dac&) = delete;
	dac(dac&&) = delete;
	dac& operator=(dac&&) = delete;

	~dac()
	{
		HAL_DAC_Stop(m_dac_handle, m_dac_channel);
	}

	[[nodiscard]]
	DAC_HandleTypeDef* get_dac_handle() const noexcept
	{
		return m_dac_handle;
	}

	[[nodiscard]]
	std::uint32_t get_dac_channel() const noexcept
	{
		return m_dac_channel;
	}

	[[nodiscard]]
	unsigned int get_dac_alignment() const noexcept
	{
		return DacAlignment;
	}

	void set(double output) noexcept
	{
		output = std::clamp(
			output,
			static_cast<double>(MinOutput),
			static_cast<double>(MaxOutput)
		);
		HAL_DAC_SetValue(
			m_dac_handle,
			m_dac_channel,
			DacAlignment,
			convert_to_dac(output)
		);
	}

private:
	DAC_HandleTypeDef* m_dac_handle;
	std::uint32_t m_dac_channel;
	double m_dac_resolution;

	constexpr int convert_to_dac(double output) const noexcept
	{
		return static_cast<int>(
			(output / (MaxOutput - MinOutput)) * m_dac_resolution
		);
	}
};

/**
  * Example;
  *
  * stm32::dac led(hdac, DAC_CHANNEL_1);
  * led.set(100);
  */

} /* namespace STM32 */

#endif /* STM32_DAC_HPP */
