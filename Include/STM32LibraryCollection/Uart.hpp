/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_UART_HPP
#define STM32_UART_HPP

#include <array>
#include <cstdint>

#include "main.h"

#if !defined(HAL_UART_MODULE_ENABLED) /* module check */
static_assert(false, "HAL UART module is not enabled!");
#endif /* module check */

#if !defined(HAL_DMA_MODULE_ENABLED) /* module check */
static_assert(false, "HAL DMA module is not enabled!");
#endif /* module check */

static_assert(__cplusplus >= 201703L, "C++17 required!");

namespace  stm32 {

template <std::uint32_t TransmitTimeout = 100>
class uart {
public:
	uart(UART_HandleTypeDef& uart_handle) noexcept
	: m_uart_handle{&uart_handle}
	{ }

	uart(const uart&) = delete;
	uart& operator=(const uart&) = delete;
	uart(uart&&) = delete;
	uart& operator=(uart&&) = delete;

	[[nodiscard]]
	UART_HandleTypeDef* get_uart_handle() const noexcept
	{
		return m_uart_handle;
	}

	bool abort_receive() noexcept
	{
		return HAL_UART_AbortReceive(m_uart_handle) == HAL_OK;
	}

	bool abort_transmit() noexcept
	{
		return HAL_UART_AbortTransmit(m_uart_handle) == HAL_OK;
	}

	template <std::size_t MessageLength>
	bool receive_to_dma(
		std::array<char, MessageLength>& rx_message,
		std::size_t size = MessageLength
	) noexcept
	{
		return (HAL_OK == HAL_UART_Receive_DMA(
			m_uart_handle,
			reinterpret_cast<std::uint8_t*>(rx_message.data()),
			static_cast<std::uint16_t>(size)
		));
	}

	template <std::size_t MessageLength>
	bool transmit(
		const std::array<char, MessageLength>& tx_message,
		std::size_t size = MessageLength
	) noexcept
	{
		return (HAL_OK == HAL_UART_Transmit(
			m_uart_handle,
			reinterpret_cast<std::uint8_t*>(
				const_cast<char *>(
					tx_message.data()
				)
			),
			static_cast<std::uint16_t>(size),
			TransmitTimeout
		));
	}

private:
	UART_HandleTypeDef* m_uart_handle;
};

template <std::uint32_t TransmitTimeout = 100>
using hc05 = uart<TransmitTimeout>;

template <std::uint32_t TransmitTimeout = 100>
using hc06 = uart<TransmitTimeout>;
/**
  * Example;
  *
  * std::array<char, 5> rx_message{};
  * std::array<char, 5> tx_message{'1','3','3','2','1'};
  * stm32::uart raspberrypi(huart3);
  * raspberrypi.transmit(tx_message);
  * raspberrypi.receive_to_dma(rx_message);
  */

} /* namespace STM32 */

#endif /* STM32_UART_HPP */
