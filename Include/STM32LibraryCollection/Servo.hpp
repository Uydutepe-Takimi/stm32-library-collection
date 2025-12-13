/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_SERVO_HPP
#define STM32_SERVO_HPP

#include "Pwm.hpp"

namespace STM32 {

/**
 * @typedef Servo, A Servo motor control class using PWM.
 * 
 * @example Usage;
 * @code {.cpp}
 * #include <STM32LibraryCollection/Servo.hpp>
 * 
 * TIM_HandleTypeDef htim1; // Assume this is properly initialized elsewhere.
 * STM32::Servo servo{htim1, TIM_CHANNEL_1};
 * servo.Set(90); // Set servo to mid-range position.
 * auto current_position = servo.Get(); // Get current servo position.
 * @endcode
 */
using Servo = Pwm<
    WorkingMode::Blocking,
    PwmDutyCycleRange<2.5, 12.>,
    PwmInputRange<0, 180, 90>,
    PwmInputRangeMax<0, 180>
>;

} /* namespace STM32 */

#endif /* STM32_SERVO_HPP */
