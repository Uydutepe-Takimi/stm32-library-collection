/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_CONFIG_HPP
#define STM32_CONFIG_HPP

#include <concepts>

namespace STM32 {

/**
 * @namespace WorkingMode, Working modes for peripherals.
 */
namespace WorkingMode {

/**
 * @struct Blocking, tag for Blocking working mode.
 */
struct Blocking {};

/**
 * @struct Interrupt, tag for Interrupt working mode.
 */
struct Interrupt {};

/**
 * @struct DMA, tag for Direct Memory Access working mode.
 */
struct DMA {};

} /* namespace WorkingMode */

/**
 * @brief IsWorkingMode, A concept to check if a type is a WorkingMode.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage;
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