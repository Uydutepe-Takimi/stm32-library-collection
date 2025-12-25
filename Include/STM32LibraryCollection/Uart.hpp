/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_UART_HPP
#define STM32_UART_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ranges>

#include "Config.hpp"
#include "__Internal/__Utility.hpp"

#include "main.h"

#if !defined(HAL_UART_MODULE_ENABLED) /* module check */
#error "HAL UART module is not enabled!"
#endif /* module check */

#if !defined(HAL_DMA_MODULE_ENABLED) /* module check */
#error "HAL DMA module is not enabled!"
#endif /* module check */

#if (USE_HAL_UART_REGISTER_CALLBACKS != 1) /* module check */
#error "HAL UART callbacks are not enabled!"
#endif /* module check */

namespace STM32 {

/**
 * @struct UartTimeout, A utility struct to hold the UART operation timeout value.
 * 
 * @tparam TimeoutV   UART operation timeout value in milliseconds.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Uart.hpp>
 *
 * using MyUartTimeout = STM32::UartTimeout<200>;
 * auto timeout = MyUartTimeout::value; // timeout is 200ms.
 * @endcode
 */
template <std::uint32_t TimeoutV>
struct UartTimeout : __Internal::__Constant<std::uint32_t, TimeoutV> {
    static_assert(
        TimeoutV > 0,
        "Timeout must be greater than zero"
    );
};

/**
 * @brief IsUartTimeout, A concept to check if a type is a UartTimeout.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Uart.hpp>
 * 
 * static_assert(STM32::IsUartTimeout<STM32::UartTimeout<100>>);
 * static_assert(!STM32::IsUartTimeout<int>);
 * @endcode
 */
template <typename T>
concept IsUartTimeout =
    __Internal::__IsConstant<T> &&
    std::same_as<typename T::ValueTypeT, std::uint32_t> &&
    T::value > 0;

/**
 * @brief IsUartMessage, A concept to check if a type is a valid UART message buffer.
 * 
 * Valid types must be:
 * - Contiguous range (data stored in continuous memory)
 * - Sized range (size() is available)
 * - char value type
 * 
 * @tparam T        Type to be checked.
 * 
 * @note Accepts both fixed-size containers (std::array) and dynamic containers
 *       (std::vector, std::string). Buffer sizes exceeding 65535 bytes will be
 *       silently clamped to 65535 (std::uint16_t max) during transmission/reception.
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Uart.hpp>
 * static_assert(STM32::IsUartMessage<std::array<char, 100>>);   // OK
 * static_assert(STM32::IsUartMessage<std::vector<char>>);       // OK
 * static_assert(STM32::IsUartMessage<std::string>);             // OK
 * static_assert(!STM32::IsUartMessage<std::array<int, 100>>);   // Fails: not char
 * static_assert(!STM32::IsUartMessage<int>);                    // Fails: not a range
 * @endcode
 */
template <typename T>
concept IsUartMessage = __Internal::__IsMessage<T, char>;

/**
 * @class Uart, A class to manage UART functionality on STM32 microcontrollers.
 * 
 * @tparam WorkingModeT         Working mode of the UART
 *                              (WorkingMode::Blocking, WorkingMode::Interrupt, WorkingMode::DMA).
 * @tparam UniqueTagT           Unique tag type to differentiate multiple Uart instances.
 *                              UniqueTagT must be STM32_UNIQUE_TAG.
 *
 * @note Uart class is non-copyable and non-movable.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Uart.hpp>
 *
 * UART_HandleTypeDef huart1; // Assume this is properly initialized elsewhere.
 * UART_HandleTypeDef huart2; // Assume this is properly initialized elsewhere.
 *
 * // Create UART with DMA as default mode
 * STM32::Uart<STM32::WorkingMode::DMA, STM32_UNIQUE_TAG> uart1{huart1};
 *
 * // Create another UART with Interrupt as default mode (independent callbacks)
 * STM32::Uart<STM32::WorkingMode::Interrupt, STM32_UNIQUE_TAG> uart2{huart2};
 *
 * std::array<char, 100> tx_message{};
 * std::array<char, 100> rx_message{};
 *
 * // 1. Use default mode (DMA) with callback
 * uart1.Transmit(tx_message, [](){
 *     // TX complete callback
 * });
 *
 * uart1.ReceiveTo(rx_message, [](){
 *     // RX complete callback - parse received data here
 * });
 *
 * // 2. Override mode per-operation: use Blocking for TX, keep DMA for RX
 * uart1.Transmit<STM32::WorkingMode::Blocking>(tx_message);
 *
 * // 3. Blocking mode with custom timeout (500ms)
 * uart1.Transmit<STM32::WorkingMode::Blocking, STM32::UartTimeout<500>>(tx_message);
 * uart1.ReceiveTo<STM32::WorkingMode::Blocking, STM32::UartTimeout<500>>(rx_message);
 *
 * // 4. Access underlying HAL handle if needed
 * auto& handle = uart1.GetHandle();
 * @endcode
 */
template <IsWorkingMode WorkingModeT, __Internal::__IsUniqueTag UniqueTagT>
class Uart {
    using TransmitCompleteCallbackT = __Internal::__CallbackManager<
        UART_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
        HAL_UART_RegisterCallback, HAL_UART_UnRegisterCallback, HAL_UART_TX_COMPLETE_CB_ID
    >;
    using ReceiveCompleteCallbackT = __Internal::__CallbackManager<
        UART_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
        HAL_UART_RegisterCallback, HAL_UART_UnRegisterCallback, HAL_UART_RX_COMPLETE_CB_ID
    >;
public:

    /**
     * @typedef CallbackT, Type alias for callback functions.
     */
    using CallbackT = TransmitCompleteCallbackT::CallbackT;

    /**
     * @brief Construct Uart class.
     * 
     * @param handle        Reference to the UART handle.
     */
    explicit Uart(UART_HandleTypeDef& handle) noexcept
      : m_handle{handle},
        m_transmit_complete_callback{handle},
        m_receive_complete_callback{handle}
    { }

    /**
     * @defgroup Deleted copy and move members.
     * @{
     */
    Uart(const Uart&) = delete;
    Uart& operator=(const Uart&) = delete;
    Uart(Uart&&) = delete;
    Uart& operator=(Uart&&) = delete;
    /** @} */

    /**
     * @brief Destroy Uart class.
     * 
     * @note Callbacks are automatically unregistered via RAII.
     */
    ~Uart() = default;

    /**
     * @returns UART handle reference.
     */
    [[nodiscard]]
    auto&& GetHandle(this auto&& self) noexcept
    {
        return std::forward<decltype(self)>(self).m_handle;
    }

    /**
     * @brief Receive data into the provided message buffer in blocking mode.
     * 
     * @tparam RxWorkingModeT   Working mode for receiving (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param rx_message        A contiguous range to store the received message.
     * 
     * @returns True on success, false otherwise.
     *
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     *          Only the first 65535 bytes will be received for oversized buffers.
     */
    template <
        IsWorkingMode RxWorkingModeT = WorkingModeT,
        IsUartTimeout TimeoutV = UartTimeout<100>
    >
    bool ReceiveTo(
        IsUartMessage auto& rx_message
    ) noexcept
    requires std::same_as<RxWorkingModeT, WorkingMode::Blocking>
    {
        return (HAL_OK == HAL_UART_Receive(
            &m_handle,
            reinterpret_cast<std::uint8_t*>(std::ranges::data(rx_message)),
            __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message)),
            TimeoutV::value
        ));
    }

    /**
     * @brief Receive data into the provided message buffer in non-blocking mode.
     * 
     * @tparam RxWorkingModeT   Working mode for receiving (default is WorkingModeT).
     * 
     * @param rx_message        A contiguous range to store the received message.
     * @param complete_callback Callback function to be called upon completion.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     *          Only the first 65535 bytes will be received for oversized buffers.
     */
    template <
        IsWorkingMode RxWorkingModeT = WorkingModeT
    >
    bool ReceiveTo(
        IsUartMessage auto& rx_message,
        CallbackT&& complete_callback = [](){}
    ) noexcept
    requires (!std::same_as<RxWorkingModeT, WorkingMode::Blocking>)
    {
        m_receive_complete_callback.Set(
            std::move(complete_callback)
        );
        if constexpr (std::same_as<RxWorkingModeT, WorkingMode::Interrupt>){
            return (HAL_OK == HAL_UART_Receive_IT(
                &m_handle,
                reinterpret_cast<std::uint8_t*>(std::ranges::data(rx_message)),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message))
            ));
        } else if constexpr (std::same_as<RxWorkingModeT, WorkingMode::DMA>){
            return (HAL_OK == HAL_UART_Receive_DMA(
                &m_handle,
                reinterpret_cast<std::uint8_t*>(std::ranges::data(rx_message)),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message))
            ));
        }
    }

    /**
     * @brief Transmit data from the provided message buffer in blocking mode.
     * 
     * @tparam TxWorkingModeT   Working mode for transmitting (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param tx_message        A contiguous range containing the message to transmit.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     *          Only the first 65535 bytes will be transmitted for oversized buffers.
     */
    template <
        IsWorkingMode TxWorkingModeT = WorkingModeT,
        IsUartTimeout TimeoutV = UartTimeout<100>
    >
    bool Transmit(
        const IsUartMessage auto& tx_message
    ) noexcept
    requires std::same_as<TxWorkingModeT, WorkingMode::Blocking>
    {
        return (HAL_OK == HAL_UART_Transmit(
            &m_handle,
            reinterpret_cast<std::uint8_t*>(
                const_cast<char *>(std::ranges::data(tx_message))
            ),
            __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message)),
            TimeoutV::value
        ));
    }

    /**
     * @brief Transmit data from the provided message buffer in non-blocking mode.
     * 
     * @tparam TxWorkingModeT    Working mode for transmitting (default is WorkingModeT).
     * 
     * @param tx_message         A contiguous range containing the message to transmit.
     * @param complete_callback  Callback function to be called upon completion.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     *          Only the first 65535 bytes will be transmitted for oversized buffers.
     */
    template <
        IsWorkingMode TxWorkingModeT = WorkingModeT
    >
    bool Transmit(
        const IsUartMessage auto& tx_message,
        CallbackT&& complete_callback = [](){}
    ) noexcept
    requires (!std::same_as<TxWorkingModeT, WorkingMode::Blocking>)
    {
        m_transmit_complete_callback.Set(
            std::move(complete_callback)
        );
        if constexpr (std::same_as<TxWorkingModeT, WorkingMode::Interrupt>){
            return (HAL_OK == HAL_UART_Transmit_IT(
                &m_handle,
                reinterpret_cast<std::uint8_t*>(
                    const_cast<char *>(std::ranges::data(tx_message))
                ),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message))
            ));
        } else if constexpr (std::same_as<TxWorkingModeT, WorkingMode::DMA>){
            return (HAL_OK == HAL_UART_Transmit_DMA(
                &m_handle,
                reinterpret_cast<std::uint8_t*>(
                    const_cast<char *>(std::ranges::data(tx_message))
                ),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message))
            ));
        }
    }

private:
    UART_HandleTypeDef& m_handle;
    TransmitCompleteCallbackT m_transmit_complete_callback;
    ReceiveCompleteCallbackT m_receive_complete_callback;
};

} /* namespace STM32 */

#endif /* STM32_UART_HPP */
