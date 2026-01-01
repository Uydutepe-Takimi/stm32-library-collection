/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_CONFIG_HPP
#define STM32_CONFIG_HPP

#include <concepts>

namespace STM32 {

/**
 * @namespace WorkingMode, Tag types for peripheral operation modes.
 * 
 * These tags are used as template parameters to specify the operation mode
 * of peripherals (UART, SPI, I2C, etc.) at compile time.
 */
namespace WorkingMode {

/**
 * @struct Blocking, Tag for blocking (polling) working mode.
 * 
 * In blocking mode, operations wait until completion before returning.
 * Simple but blocks the CPU during the entire operation.
 */
struct Blocking {};

/**
 * @struct Interrupt, Tag for interrupt-driven working mode.
 * 
 * In interrupt mode, operations return immediately and trigger
 * a callback when complete. More efficient than blocking.
 */
struct Interrupt {};

/**
 * @struct DMA, Tag for Direct Memory Access working mode.
 * 
 * In DMA mode, data transfer is handled by the DMA controller,
 * freeing the CPU for other tasks. Most efficient for large transfers.
 */
struct DMA {};

} /* namespace WorkingMode */

/**
 * @brief IsWorkingMode, A concept to check if a type is a WorkingMode.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Config.hpp>
 * 
 * static_assert(STM32::IsWorkingMode<STM32::WorkingMode::Blocking>);
 * static_assert(!STM32::IsWorkingMode<int>);
 * @endcode
 */
template <typename T>
concept IsWorkingMode = std::same_as<T, WorkingMode::Blocking> ||
                        std::same_as<T, WorkingMode::Interrupt> ||
                        std::same_as<T, WorkingMode::DMA>;

} /* namespace STM32 */

#endif /* STM32_CONFIG_HPP */