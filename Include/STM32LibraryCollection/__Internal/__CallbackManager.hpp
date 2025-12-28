/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_CALLBACK_MANAGER_HPP
#define STM32_CALLBACK_MANAGER_HPP

#include "__InplaceFunction.hpp"

namespace STM32 {

/**
 * @typedef CallbackT, Non-allocating callback type for embedded systems.
 */
using CallbackT = __Internal::__InplaceFunction<>;

namespace __Internal {

/**
 * @class __CallbackManager, A self-registering RAII callback manager for STM32 HAL peripherals.
 * 
 * This class automatically registers with HAL on construction and unregisters on destruction,
 * eliminating manual registration boilerplate in peripheral classes.
 * 
 * Each unique combination of template parameters creates independent static callback storage,
 * allowing multiple peripheral instances and callback types.
 * 
 * @tparam HandleT                 Type of the HAL peripheral handle (e.g., UART_HandleTypeDef).
 * @tparam PeripheralUniqueTagT    Unique tag type to differentiate peripheral instances.
 *                                 Must be created via STM32_UNIQUE_TAG macro at instantiation site.
 * @tparam CallbackUniqueTagT      Unique tag type to differentiate callback purposes.
 *                                 Use STM32_UNIQUE_TAG on separate lines within the class.
 * @tparam HalRegisterFunctionT    HAL registration function (e.g., HAL_UART_RegisterCallback).
 * @tparam HalUnregisterFunctionT  HAL unregistration function (e.g., HAL_UART_UnRegisterCallback).
 * @tparam HalCallbackIdV          HAL callback ID constant (e.g., HAL_UART_TX_COMPLETE_CB_ID).
 * 
 * @note This is an internal class. Do not use directly in application code.
 * 
 * @example Usage Pattern:
 * 
 * @code {.cpp}
 * template <IsWorkingMode WorkingModeT, __Internal::__IsUniqueTag UniqueTagT>
 * class MyPeripheral {
 *     // Define auto-registering callbacks - each on its own line for unique types
 *     using TransmitCompleteCallbackT = __Internal::__CallbackManager<
 *         HAL_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
 *         HAL_XXX_RegisterCallback, HAL_XXX_UnRegisterCallback, HAL_XXX_TX_COMPLETE_CB_ID
 *     >;
 *     using ReceiveCompleteCallbackT = __Internal::__CallbackManager<
 *         HAL_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
 *         HAL_XXX_RegisterCallback, HAL_XXX_UnRegisterCallback, HAL_XXX_RX_COMPLETE_CB_ID
 *     >;
 * 
 *     TransmitCompleteCallbackT m_transmit_complete_callback;  // Member - auto registers/unregisters
 *     ReceiveCompleteCallbackT m_receive_complete_callback;
 * 
 * public:
 * 
 *     // Constructor just initializes callback members - no manual HAL calls!
 *     explicit MyPeripheral(HAL_HandleTypeDef& handle) noexcept
 *       : m_handle{handle},
 *         m_transmit_complete_callback{handle},
 *         m_receive_complete_callback{handle}
 *     { }
 * 
 *     // Destructor is trivial - RAII handles cleanup!
 *     ~MyPeripheral() = default;
 * 
 *     bool Transmit(const auto& data, CallbackT&& callback = [](){})
 *     {
 *         m_transmit_complete_callback.Set(std::move(callback));
 *         return HAL_XXX_Transmit_IT(&m_handle, data.data(), data.size()) == HAL_OK;
 *     }
 * 
 *     bool ReceiveTo(auto& buffer, CallbackT&& callback = [](){})
 *     {
 *         m_receive_complete_callback.Set(std::move(callback));
 *         return HAL_XXX_Receive_IT(&m_handle, buffer.data(), buffer.size()) == HAL_OK;
 *     }
 * 
 * private:
 *     HAL_HandleTypeDef& m_handle;
 * };
 * @endcode
 */
template <
    typename HandleT,
    typename PeripheralUniqueTagT,
    typename CallbackUniqueTagT,
    auto HalRegisterFunctionT,
    auto HalUnregisterFunctionT,
    auto HalCallbackIdV
>
class __CallbackManager {
public:

    /**
     * @brief Construct and register with HAL.
     * 
     * @param handle    Reference to the peripheral handle.
     */
    explicit __CallbackManager(HandleT& handle) noexcept
      : m_handle{handle}
    {
        HalRegisterFunctionT(&m_handle, HalCallbackIdV, &__CallbackManager::Invoke);
    }

    /**
     * @brief Destroy and unregister from HAL.
     */
    ~__CallbackManager()
    {
        HalUnregisterFunctionT(&m_handle, HalCallbackIdV);
        s_callback = nullptr;
    }

    /**
     * @defgroup Deleted copy and move members.
     * @{
     */
    __CallbackManager(const __CallbackManager&) = delete;
    __CallbackManager& operator=(const __CallbackManager&) = delete;
    __CallbackManager(__CallbackManager&&) = delete;
    __CallbackManager& operator=(__CallbackManager&&) = delete;
    /** @} */

    /**
     * @brief Set a callback.
     * 
     * @param callback  Callback function to invoke upon event completion.
     */
    void Set(CallbackT&& callback) noexcept
    {
        s_callback = std::move(callback);
    }

    /**
     * @brief Clear the callback.
     */
    void Clear() noexcept
    {
        s_callback = nullptr;
    }

    /**
     * @brief HAL-compatible callback function pointer.
     * 
     * Automatically registered with HAL. Invokes the stored callback if set.
     * 
     * @param handle    Pointer to the peripheral handle (unused).
     */
    static void Invoke([[maybe_unused]] HandleT* handle) noexcept
    {
        if (s_callback) {
            s_callback();
        }
    }

private:
    HandleT& m_handle;
    static inline CallbackT s_callback{};
};

} /* namespace __Internal */

} /* namespace STM32 */

#endif /* STM32_CALLBACK_MANAGER_HPP */
