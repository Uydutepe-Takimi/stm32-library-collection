/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_GPIO_HPP
#define STM32_GPIO_HPP

#include <concepts>
#include <cstdint>
#include <utility>

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
 * @namespace GpioType, Tag types for GPIO pin direction.
 * 
 * These tags are used as template parameters to specify whether
 * a GPIO pin is configured as input or output at compile time.
 */
namespace GpioType {

/**
 * @struct Input, Tag for input GPIO pin type.
 * 
 * Use this tag when creating a GPIO pin that reads external signals.
 */
struct Input {};

/**
 * @struct Output, Tag for output GPIO pin type.
 * 
 * Use this tag when creating a GPIO pin that drives external signals.
 */
struct Output {};

} /* namespace GpioType */

/**
 * @brief IsGpioType, A concept to check if a type is a GpioType.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Gpio.hpp>
 * 
 * static_assert(STM32::IsGpioType<STM32::GpioType::Input>);
 * static_assert(!STM32::IsGpioType<int>);
 * @endcode
 */
template <typename T>
concept IsGpioType =
    std::same_as<T, GpioType::Input> ||
    std::same_as<T, GpioType::Output>;

/**
 * @class Gpio, General Purpose Input/Output pin abstraction.
 * 
 * @tparam GpioTypeT Type of the GPIO pin
 *         (GpioType::Input or GpioType::Output).
 *
 * @note Gpio class is non-copyable and non-movable.
 * @note Use the type aliases GpioInput and GpioOutput for convenience.
 *
 * @example Usage:
 * @code{.cpp}
 * #include <STM32LibraryCollection/Gpio.hpp>
 *
 * // Output pin example - LED control
 * STM32::GpioOutput led{GPIOA, GPIO_PIN_5};
 * led.High();                              // Turn LED on
 * led.Low();                               // Turn LED off
 * led.Toggle();                            // Toggle LED state
 * led.Write(STM32::GpioPinState::High);    // Write specific state
 * led = STM32::GpioPinState::Low;          // Assignment operator
 *
 * // Input pin example - Button reading
 * STM32::GpioInput button{GPIOC, GPIO_PIN_13};
 * auto state = button.Read();              // Read pin state
 * if (button.IsHigh()) {
 *      // pressed
 * }
 * if (button.IsLow()) {
 *      // released
 * }
 * STM32::GpioPinState s = button;          // Implicit conversion
 * @endcode
 */
template <IsGpioType GpioTypeT>
class Gpio {
public:

    /**
     * @brief Construct a new Gpio object.
     * 
     * @param handle        HAL GPIO handle.
     * @param pin           GPIO pin number.
     */
    Gpio(GPIO_TypeDef* handle, std::uint16_t pin) noexcept
        : m_handle{handle}, m_pin{pin}
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
    auto&& GetHandle(this auto&& self) noexcept
    {
        return std::forward<decltype(self)>(self).m_handle;
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
    GpioPinState Read() const noexcept
    requires std::same_as<GpioTypeT, GpioType::Input>
    {
        return static_cast<GpioPinState>(
            HAL_GPIO_ReadPin(m_handle, m_pin)
        );
    }

    /**
     * @brief Implicit conversion operator to GpioPinState.
     * 
     * Allows the GPIO input pin to be used directly in conditionals
     * or assigned to a GpioPinState variable.
     * 
     * @returns State of the GPIO pin.
     */
    operator GpioPinState() const noexcept
    requires std::same_as<GpioTypeT, GpioType::Input>
    {
        return Read();
    }

    /**
     * @brief Toggle the GPIO pin state.
     */
    void Toggle() noexcept
    requires std::same_as<GpioTypeT, GpioType::Output>
    {
        HAL_GPIO_TogglePin(m_handle, m_pin);
    }

    /**
     * @brief Write the GPIO pin state.
     * 
     * @param pin_state     State to write to the GPIO pin.
     */
    void Write(GpioPinState pin_state) noexcept
    requires std::same_as<GpioTypeT, GpioType::Output>
    {
        HAL_GPIO_WritePin(
            m_handle, m_pin,
            static_cast<GPIO_PinState>(pin_state)
        );
    }

    /**
     * @brief Assignment operator to write the GPIO pin state.
     * 
     * @param state     State to write to the GPIO pin.
     * 
     * @returns Reference to the current Gpio object.
     */
    Gpio& operator=(GpioPinState state) noexcept
    requires std::same_as<GpioTypeT, GpioType::Output>
    {
        Write(state);
        return *this;
    }

    /**
     * @brief Set the GPIO pin to High state.
     */
    void High() noexcept
    requires std::same_as<GpioTypeT, GpioType::Output>
    {
        Write(GpioPinState::High);
    }

    /**
     * @brief Set the GPIO pin to Low state.
     */
    void Low() noexcept
    requires std::same_as<GpioTypeT, GpioType::Output>
    {
        Write(GpioPinState::Low);
    }

    /**
     * @returns True if the GPIO pin is in High state.
     */
    [[nodiscard]]
    bool IsHigh() const noexcept
    requires std::same_as<GpioTypeT, GpioType::Input>
    {
        return Read() == GpioPinState::High;
    }

    /**
     * @returns True if the GPIO pin is in Low state.
     */
    [[nodiscard]]
    bool IsLow() const noexcept
    requires std::same_as<GpioTypeT, GpioType::Input>
    {
        return Read() == GpioPinState::Low;
    }

private:
    GPIO_TypeDef* m_handle;
    std::uint16_t m_pin;
};

/**
 * @typedef GpioInput, Convenience alias for GPIO pin in input mode.
 * 
 * @example Usage:
 * @code{.cpp}
 * STM32::GpioInput button{GPIOC, GPIO_PIN_13};
 * @endcode
 */
using GpioInput = Gpio<GpioType::Input>;

/**
 * @typedef GpioOutput, Convenience alias for GPIO pin in output mode.
 * 
 * @example Usage:
 * @code{.cpp}
 * STM32::GpioOutput led{GPIOA, GPIO_PIN_5};
 * @endcode
 */ 
using GpioOutput = Gpio<GpioType::Output>;

} /* namespace STM32 */

#endif /* STM32_GPIO_HPP */
