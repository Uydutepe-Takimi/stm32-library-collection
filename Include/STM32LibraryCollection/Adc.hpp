/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_ADC_HPP
#define STM32_ADC_HPP

#include <array>
#include <cmath>
#include <algorithm>

#include "main.h"

#if !defined(HAL_ADC_MODULE_ENABLED) /* module check */
static_assert(false, "HAL ADC module is not enabled!");
#endif /* module check */

static_assert(__cplusplus >= 201703L, "C++17 required!");

namespace STM32 {

template <std::uint32_t MinInput = 0, std::uint32_t MaxInput = 100,
		std::uint32_t MedianFilterSize = 1,std::uint32_t TimeoutMs = 5'000>
class adc {
public:
	adc(ADC_HandleTypeDef& adc_handle,
		double adc_resolution = 4095.) noexcept
	: m_adc_handle{&adc_handle},
	  m_adc_resolution{adc_resolution}
	{
		static_assert(
			MinInput <= MaxInput,
			"the minimum input cannot be greater than the maximum input!"
		);
		static_assert(
			1 <= MedianFilterSize,
			"the median filter size must be greater or equal to 1!"
		);
		static_assert(
			MedianFilterSize % 2 != 0,
			"the median filter size must be an odd number!"
		);
	}

	adc(const adc&) = delete;
	adc& operator=(const adc&) = delete;
	adc(adc&&) = delete;
	adc& operator=(adc&&) = delete;

	[[nodiscard]]
	ADC_HandleTypeDef* get_adc_handle() const noexcept
	{
		return m_adc_handle;
	}

	[[nodiscard]]
	double get() const noexcept
	{
		static std::array<double, MedianFilterSize> adc_values{};
		for (std::uint32_t i{}; i < MedianFilterSize; ++i){
			HAL_ADC_Start(m_adc_handle);
			HAL_ADC_PollForConversion(m_adc_handle , TimeoutMs);
			std::uint32_t input = HAL_ADC_GetValue(m_adc_handle);
			HAL_ADC_Stop(m_adc_handle);
			adc_values[i] = std::round(
				(input / m_adc_resolution) * (MaxInput - MinInput)
			) + MinInput;
		}
		std::sort(std::begin(adc_values), std::end(adc_values));
		auto value = adc_values[MedianFilterSize/2];
		return MaxInput <=  value ? MaxInput : value;
	}

private:
	ADC_HandleTypeDef* m_adc_handle;
	double m_adc_resolution;
};

/**
  * Example;
  *
  * stm32::adc temperature_sensor(hadc1);
  * auto temperature = temperature_sensor.get();
  */

} /* namespace STM32 */

#endif /* STM32_ADC_HPP */
