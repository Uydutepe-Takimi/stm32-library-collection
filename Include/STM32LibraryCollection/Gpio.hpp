/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_GPIO_HPP
#define STM32_GPIO_HPP

#include <cstdint>
#include <concepts>

#include "main.h"

#if !defined(HAL_GPIO_MODULE_ENABLED) /* module check */
#error "HAL GPIO module is not enabled!"
#endif /* module check */

namespace STM32 {

/**
 * @enum GpioPinState, States of a GPIO pin.
 */
enum class GpioPinState {
    Low = GPIO_PinState::GPIO_PIN_RESET,
    High = GPIO_PinState::GPIO_PIN_SET
};

/**
 * @namespace GpioType, Types of GPIO pins.
 */
namespace GpioType {
/**
 * @struct Input, tag for Input GPIO pin type.
 */
struct Input {};

/**
 * @struct Output, tag for Output GPIO pin type.
 */
struct Output {};
} /* namespace GpioType */

/**
 * @class Gpio, General Purpose Input/Output pin abstraction.
 * 
 * @tparam GpioTypeT Type of the GPIO pin
 *         (GpioType::Input or GpioType::Output).
 *
 * @note Gpio class is non-copyable and non-movable.
 *
 * @example Write to a GPIO pin.
 * @code{.cpp}
 * #include <STM32LibraryCollection/Gpio.hpp>
 *
 * STM32::GpioOutput led_pin(GPIOA, GPIO_PIN_5);
 * led_pin.Write(STM32::GpioPinState::High);
 * led_pin.Write(STM32::GpioPinState::Low);
 * @endcode
 *
 * @example Toggle a GPIO pin.
 * @code{.cpp}
 * #include <STM32LibraryCollection/Gpio.hpp>
 *
 * STM32::GpioOutput led_pin(GPIOA, GPIO_PIN_5);
 * led_pin.Toogle();
 * led_pin.Toogle();
 * @endcode
 *
 * @example Read the state of a GPIO pin.
 * @code{.cpp}
 * #include <STM32LibraryCollection/Gpio.hpp>
 *
 * STM32::GpioInput pin(GPIOA, GPIO_PIN_5);
 * auto state = pin.Read();
 * @endcode
 */
template <typename GpioTypeT>
requires std::same_as<GpioTypeT, GpioType::Input> ||
         std::same_as<GpioTypeT, GpioType::Output>
class Gpio {
public:

    /**
     * @brief Construct a new Gpio object.
     * 
     * @param gpio_handle   HAL GPIO handle.
     * @param pin           GPIO pin number.
     */
    Gpio(GPIO_TypeDef* gpio_handle, std::uint16_t pin) noexcept
        : m_gpio_handle{gpio_handle}, m_pin{pin}
    { }

    /**
     * @defgroup Deleted copy and move members.
     * @{
     */
    Gpio(const Gpio &) = delete;
    Gpio &operator=(const Gpio &) = delete;
    Gpio(Gpio &&) = delete;
    Gpio &operator=(Gpio &&) = delete;
    /** @} */

    /**
     * @returns HAL GPIO handle.
     */
    [[nodiscard]]
    GPIO_TypeDef* GetHandle() const noexcept
    {
        return m_gpio_handle;
    }

    /**
     * @returns GPIO pin number.
     */
    [[nodiscard]]
    std::uint16_t GetPin() const noexcept
    {
        return m_pin;
    }

    /**
     * @returns State of the GPIO pin.
     */
    [[nodiscard]]
    GpioPinState Read() const noexcept requires std::same_as<GpioTypeT, GpioType::Input>
    {
        return static_cast<GpioPinState>(
            HAL_GPIO_ReadPin(m_gpio_handle, m_pin)
        );
    }

    /**
     * @brief Toggle the GPIO pin state.
     */
    void Toggle() noexcept requires std::same_as<GpioTypeT, GpioType::Output>
    {
        HAL_GPIO_TogglePin(m_gpio_handle, m_pin);
    }

    /**
     * @brief Write the GPIO pin state.
     * 
     * @param pin_state     State to write to the GPIO pin.
     */
    void Write(GpioPinState pin_state) noexcept requires std::same_as<GpioTypeT, GpioType::Output>
    {
        HAL_GPIO_WritePin(
            m_gpio_handle, m_pin,
            static_cast<GPIO_PinState>(pin_state)
        );
    }

private:
    GPIO_TypeDef* m_gpio_handle;
    std::uint16_t m_pin;
};

/**
 * @typedef GpioInput, GPIO pin in input mode.
 */
using GpioInput = Gpio<GpioType::Input>;

/**
 * @typedef GpioOutput, GPIO pin in output mode.
 */ 
using GpioOutput = Gpio<GpioType::Output>;

} /* namespace STM32 */

#endif /* STM32_GPIO_HPP */
