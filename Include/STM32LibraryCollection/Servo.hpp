/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_SERVO_HPP
#define STM32_SERVO_HPP

#include "Pwm.hpp"

namespace STM32 {

/**
 * @typedef Servo, Pre-configured PWM class for standard servo motor control.
 * 
 * Configured for standard hobby servos:
 * - Input range: 0-180 degrees (default position: 90 degrees)
 * - Duty cycle: 2.5% - 12% (1ms - 2.4ms pulse at 50Hz)
 * 
 * @note Requires timer configured for 50Hz (20ms period).
 * @note Servo class is non-copyable and non-movable.
 * @note Servo starts at 90 degrees (center position) upon construction.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Servo.hpp>
 * 
 * TIM_HandleTypeDef htim1; // Assume properly initialized for 50Hz PWM
 * 
 * STM32::Servo servo{htim1, TIM_CHANNEL_1};
 * servo.Set(0);        // Move to 0 degrees
 * servo.Set(90);       // Move to center (90 degrees)
 * servo.Set(180);      // Move to 180 degrees
 * auto pos = servo.Get();  // Read current position
 * @endcode
 */
using Servo = Pwm<
    PwmConfig<
        PwmDutyCycleRange<2.5, 12.>,
        PwmInputRange<0, 180, 90>,
        PwmInputRangeMax<0, 180>
    >
>;

} /* namespace STM32 */

#endif /* STM32_SERVO_HPP */
