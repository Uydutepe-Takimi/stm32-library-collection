/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_UTILITY_HPP
#define STM32_UTILITY_HPP

/**
 * @file __Utility.hpp
 * @brief Aggregate header for internal utility components.
 * 
 * This header provides a convenient single include for all internal utilities:
 * - __CallbackManager: Bridge between C++ callbacks and HAL C-style callbacks.
 * - __Constant: Compile-time constant value wrapper.
 * - __FixedCallback: Non-allocating callable wrapper for embedded systems.
 * - __Range: Compile-time numeric range definition.
 * - __UniqueTag: Unique type generation for template differentiation.
 * 
 * @note These are internal utilities. Application code should not include
 *       this header directly. Use the public peripheral headers instead.
 */

#include "__CallbackManager.hpp"
#include "__Constant.hpp"
#include "__FixedCallback.hpp"
#include "__Range.hpp"
#include "__UniqueTag.hpp"

#endif /* STM32_UTILITY_HPP */
