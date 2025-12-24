/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_CALLBACK_MANAGER_HPP
#define STM32_CALLBACK_MANAGER_HPP

#include "__InplaceFunction.hpp"

namespace STM32 {

namespace __Internal {

/**
 * @class __CallbackManager, A static callback manager for STM32 HAL peripherals.
 * 
 * This class provides a bridge between user-defined C++ callbacks (lambdas,
 * functors) and STM32 HAL C-style callback functions. Uses __FixedCallback
 * internally to avoid heap allocation. Each unique instantiation (via UniqueTagT)
 * creates independent static storage for callbacks, allowing multiple peripheral
 * instances to have separate callbacks.
 * 
 * @tparam HandleT      Type of the HAL peripheral handle (e.g., UART_HandleTypeDef).
 * @tparam UniqueTagT   Unique tag type to differentiate multiple instances.
 *                      Must be created via STM32_UNIQUE_TAG macro.
 * 
 * @note This is an internal class. Do not use directly in application code.
 * 
 * @warning Each peripheral instance MUST use a unique UniqueTagT to prevent
 *          callback collisions. Always use STM32_UNIQUE_TAG macro.
 * 
 * ## Integration Guide for Peripheral Wrapper Developers
 * 
 * ### Step 1: Define Private Type Alias
 * 
 * @code {.cpp}
 * template <IsWorkingMode WorkingModeT, __Internal::__IsUniqueTag UniqueTagT>
 * class MyPeripheral {
 *     using CallbackManagerT = __Internal::__CallbackManager<
 *         HAL_HandleTypeDef,             // HAL handle type
 *         UniqueTagT                     // Unique tag type
 *     >;
 * @endcode
 * 
 * ### Step 2: Expose CallbackT to Users
 * 
 * @code {.cpp}
 * public:
 *     using CallbackT = CallbackManagerT::CallbackT;
 * @endcode
 * 
 * ### Step 3: Register HAL Callbacks in Constructor
 * 
 * @code {.cpp}
 *     explicit MyPeripheral(HAL_HandleTypeDef& handle) noexcept
 *       : m_handle{handle}
 *     {
 *         HAL_XXX_RegisterCallback(
 *             &m_handle,
 *             HAL_XXX_TX_COMPLETE_CB_ID,
 *             CallbackManagerT::TransmitCompleteCallback
 *         );
 *         HAL_XXX_RegisterCallback(
 *             &m_handle,
 *             HAL_XXX_RX_COMPLETE_CB_ID,
 *             CallbackManagerT::ReceiveCompleteCallback
 *         );
 *     }
 * @endcode
 * 
 * ### Step 4: Unregister HAL Callbacks in Destructor
 * 
 * @code {.cpp}
 *     ~MyPeripheral()
 *     {
 *         HAL_XXX_UnRegisterCallback(&m_handle, HAL_XXX_TX_COMPLETE_CB_ID);
 *         HAL_XXX_UnRegisterCallback(&m_handle, HAL_XXX_RX_COMPLETE_CB_ID);
 *         CallbackManagerT::UnRegisterCallbacks();
 *     }
 * @endcode
 * 
 * ### Step 5: Register User Callbacks Before HAL Operations
 * 
 * @code {.cpp}
 *     bool Transmit(const auto& data, CallbackT&& callback = [](){})
 *     {
 *         CallbackManagerT::RegisterTransmitCompleteCallback(
 *             std::move(callback)
 *         );
 *         return HAL_XXX_Transmit_IT(&m_handle, data.data(), data.size()) == HAL_OK;
 *     }
 * 
 *     bool ReceiveTo(auto& buffer, CallbackT&& callback = [](){})
 *     {
 *         CallbackManagerT::RegisterReceiveCompleteCallback(
 *             std::move(callback)
 *         );
 *         return HAL_XXX_Receive_IT(&m_handle, buffer.data(), buffer.size()) == HAL_OK;
 *     }
 * @endcode
 * 
 * ## Callback Flow Diagram
 * 
 * @verbatim
 * ┌─────────────┐    ┌──────────────────┐    ┌─────────────────┐    ┌─────────┐
 * │ User Code   │    │ Peripheral Class │    │ CallbackManager │    │ HAL/ISR │
 * └──────┬──────┘    └────────┬─────────┘    └────────┬────────┘    └────┬────┘
 *        │                    │                       │                  │
 *        │ Transmit(data, λ)  │                       │                  │
 *        │───────────────────>│                       │                  │
 *        │                    │ RegisterTxCallback(λ) │                  │
 *        │                    │──────────────────────>│                  │
 *        │                    │                       │ (stores λ)       │
 *        │                    │                       │                  │
 *        │                    │ HAL_XXX_Transmit_IT() │                  │
 *        │                    │─────────────────────────────────────────>│
 *        │                    │                       │                  │
 *        │                    │                       │    ISR fires     │
 *        │                    │                       │<─────────────────│
 *        │                    │                       │ TxCompleteCallback()
 *        │                    │                       │                  │
 *        │<───────────────────│───────────────────────│ λ() invoked      │
 *        │                    │                       │                  │
 * @endverbatim
 * 
 * ## Complete Example
 * 
 * @code {.cpp}
 * #include <STM32LibraryCollection/Uart.hpp>
 * 
 * UART_HandleTypeDef huart1; // Assume this is properly initialized elsewhere.
 * UART_HandleTypeDef huart2; // Assume this is properly initialized elsewhere.
 * 
 * // Each instance has independent callbacks due to unique tags
 * STM32::Uart<STM32::WorkingMode::Interrupt, STM32_UNIQUE_TAG> uart1{huart1};
 * STM32::Uart<STM32::WorkingMode::DMA, STM32_UNIQUE_TAG> uart2{huart2};
 * 
 * std::array<char, 64> buffer1{};
 * std::array<char, 64> buffer2{};
 * 
 * // These callbacks won't interfere with each other
 * uart1.ReceiveTo(buffer1, [](){ 
 *     // Process data received by uart1
 * });
 * uart2.ReceiveTo(buffer2, [](){
 *     // Process data received by uart2
 * });
 * @endcode
 */
template <typename HandleT, typename UniqueTagT>
class __CallbackManager {
public:

    /**
     * @typedef CallbackT, Non-allocating callback type for embedded systems.
     */
    using CallbackT = __InplaceFunction<>;

    /**
     * @brief Register a callback for receive completion.
     * 
     * @param callback  Callback function to invoke upon receive completion.
     */
    static void RegisterReceiveCompleteCallback(CallbackT&& callback) noexcept
    {
        m_receive_complete_callback = std::move(callback);
    }

    /**
     * @brief Register a callback for transmit completion.
     * 
     * @param callback  Callback function to invoke upon transmit completion.
     */
    static void RegisterTransmitCompleteCallback(CallbackT&& callback) noexcept
    {
        m_transmit_complete_callback = std::move(callback);
    }

    /**
     * @brief HAL callback for receive completion.
     * 
     * @param handle    Pointer to the peripheral handle (unused).
     */
    static void ReceiveCompleteCallback([[maybe_unused]] HandleT* handle) noexcept
    {
        if (m_receive_complete_callback) {
            m_receive_complete_callback();
        }
    }

    /**
     * @brief HAL callback for transmit completion.
     * 
     * @param handle    Pointer to the peripheral handle (unused).
     */
    static void TransmitCompleteCallback([[maybe_unused]] HandleT* handle) noexcept
    {
        if (m_transmit_complete_callback) {
            m_transmit_complete_callback();
        }
    }

    /**
     * @brief Unregister all callbacks.
     */
    static void UnRegisterCallbacks() noexcept
    {
        m_receive_complete_callback = nullptr;
        m_transmit_complete_callback = nullptr;
    }

private:
    static inline CallbackT m_receive_complete_callback{};
    static inline CallbackT m_transmit_complete_callback{};
};

} /* namespace __Internal */

} /* namespace STM32 */

#endif /* STM32_CALLBACK_MANAGER_HPP */
