/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_MESSAGE_HPP
#define STM32_MESSAGE_HPP

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ranges>

namespace STM32::__Internal {

/**
 * @brief __IsMessage, A concept to check if a type is a valid message buffer.
 * 
 * Valid types must be:
 * - Contiguous range (data stored in continuous memory)
 * - Sized range (size() is available)
 * - Specified value type (e.g., char for UART, std::uint8_t for SPI)
 * 
 * @tparam T            Type to be checked.
 * @tparam ValueTypeT   Expected value type of the range elements.
 * 
 * @note Accepts both fixed-size containers (std::array) and dynamic containers
 *       (std::vector, std::string). Buffer sizes exceeding 65535 bytes will be
 *       silently clamped to 65535 (std::uint16_t max) during transmission/reception.
 * 
 * @example Usage:
 * @code {.cpp}
 * static_assert(__Internal::__IsMessage<std::array<char, 100>, char>);      // OK
 * static_assert(__Internal::__IsMessage<std::vector<std::uint8_t>, std::uint8_t>);  // OK
 * static_assert(!__Internal::__IsMessage<std::array<int, 100>, char>);      // Fails: not char
 * static_assert(!__Internal::__IsMessage<int, char>);                       // Fails: not a range
 * @endcode
 */
template <typename T, typename ValueTypeT>
concept __IsMessage =
    std::ranges::sized_range<T> &&
    std::ranges::contiguous_range<T> &&
    std::same_as<std::ranges::range_value_t<T>, ValueTypeT> &&
    std::same_as<std::ranges::range_size_t<T>, std::size_t>;

/**
 * @brief Clamp size to a target unsigned integer type range.
 * 
 * This utility function clamps a std::size_t value to the valid range
 * of the specified unsigned integer type, which is required by STM32 HAL
 * functions for buffer size parameters.
 * 
 * @tparam TargetT  Target unsigned integer type (e.g., std::uint16_t, std::uint32_t).
 * 
 * @param size  Size to be clamped.
 * 
 * @returns Clamped size as TargetT.
 * 
 * @example Usage:
 * @code {.cpp}
 * auto clamped = __Internal::__ClampMessageLength<std::uint16_t>(buffer.size());
 * // If buffer.size() > 65535, clamped will be 65535
 * 
 * auto clamped32 = __Internal::__ClampMessageLength<std::uint32_t>(buffer.size());
 * // Clamps to std::uint32_t max if exceeded
 * @endcode
 */
template <std::unsigned_integral TargetT>
[[nodiscard]]
constexpr TargetT __ClampMessageLength(std::size_t size) noexcept
{
    return static_cast<TargetT>(
        std::min(
            size,
            static_cast<std::size_t>(std::numeric_limits<TargetT>::max())
        )
    );
}

} /* namespace STM32::__Internal */

#endif /* STM32_MESSAGE_HPP */
