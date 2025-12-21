/* SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_UART_HPP
#define STM32_UART_HPP

#include <array>
#include <concepts>
#include <cstdint>
#include <limits>

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
 * @struct UartTimeout, A utility struct to hold UART transmit timeout value.
 * 
 * @tparam TimeoutV   Transmit timeout value in milliseconds.
 * @example Usage;
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
 * @example Usage;
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
 * @class Uart, A class to manage UART functionality on STM32 microcontrollers.
 * 
 * @tparam WorkingModeT         Working mode of the UART
 *                              (WorkingMode::Blocking, WorkingMode::Interrupt, WorkingMode::DMA).
 * @tparam UniqueTagT           Unique tag type to differentiate multiple Uart instances.
 *                              UniqueTagT must be STM32_UNIQUE_TAG.
 *
 * @note Uart class is non-copyable and non-movable.
 *
 * @example Usage;
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
 * uart1.Transmit<100, STM32::WorkingMode::Blocking>(tx_message);
 *
 * // 3. Blocking mode with custom timeout (500ms)
 * uart1.Transmit<100, STM32::WorkingMode::Blocking, STM32::UartTimeout<500>>(tx_message);
 * uart1.ReceiveTo<100, STM32::WorkingMode::Blocking, STM32::UartTimeout<500>>(rx_message);
 *
 * // 4. Partial transmit/receive (only 50 bytes)
 * uart1.Transmit(tx_message, [](){}, 50);
 * uart1.ReceiveTo(rx_message, [](){}, 50);
 *
 * // 5. Access underlying HAL handle if needed
 * auto& handle = uart1.GetHandle();
 * @endcode
 */
template <IsWorkingMode WorkingModeT, __Internal::__IsUniqueTag UniqueTagT>
class Uart {
    using UartCallbackManagerT = __Internal::__CallbackManager<
        UART_HandleTypeDef, UniqueTagT
    >;
public:

    /**
     * @typedef CallbackT, Type alias for callback functions.
     */
    using CallbackT = UartCallbackManagerT::CallbackT;

    /**
     * @brief Construct Uart class.
     * 
     * @param handle        Reference to the UART handle.
     */
    explicit Uart(UART_HandleTypeDef& handle) noexcept
      : m_handle{handle}
    {
        HAL_UART_RegisterCallback(
            &m_handle,
            HAL_UART_TX_COMPLETE_CB_ID,
            UartCallbackManagerT::TransmitCompleteCallback
        );
        HAL_UART_RegisterCallback(
            &m_handle,
            HAL_UART_RX_COMPLETE_CB_ID,
            UartCallbackManagerT::ReceiveCompleteCallback
        );
    }

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
     */
    ~Uart()
    {
        HAL_UART_UnRegisterCallback(
            &m_handle,
            HAL_UART_TX_COMPLETE_CB_ID
        );
        HAL_UART_UnRegisterCallback(
            &m_handle,
            HAL_UART_RX_COMPLETE_CB_ID
        );
        UartCallbackManagerT::UnRegisterCallbacks();
    }

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
     * @tparam MessageLengthV   Length of the message buffer.
     * @tparam RxWorkingModeT   Working mode for receiving (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param rx_message        Array to store the received message.
     * @param size              Number of bytes to receive (default is MessageLengthV).
     * 
     * @returns True on success, false otherwise.
     */
    template <
        std::size_t MessageLengthV,
        IsWorkingMode RxWorkingModeT = WorkingModeT,
        IsUartTimeout TimeoutV = UartTimeout<100>
    >
    bool ReceiveTo(
        std::array<char, MessageLengthV>& rx_message,
        std::uint16_t size = MessageLengthV
    ) noexcept
    requires std::same_as<RxWorkingModeT, WorkingMode::Blocking>
    {
        static_assert(
            MessageLengthV <= std::numeric_limits<std::uint16_t>::max(),
            "MessageLengthV must be less than or equal to std::uint16_t max value"
        );
        return (HAL_OK == HAL_UART_Receive(
            &m_handle,
            reinterpret_cast<std::uint8_t*>(rx_message.data()),
            size,
            TimeoutV::value
        ));
    }

    /**
     * @brief Receive data into the provided message buffer in non-blocking mode.
     * 
     * @tparam MessageLengthV   Length of the message buffer.
     * @tparam RxWorkingModeT   Working mode for receiving (default is WorkingModeT).
     * 
     * @param rx_message        Array to store the received message.
     * @param complete_callback Callback function to be called upon completion.
     * @param size              Number of bytes to receive (default is MessageLengthV).
     * 
     * @returns True on success, false otherwise.
     */
    template <
        std::size_t MessageLengthV,
        IsWorkingMode RxWorkingModeT = WorkingModeT
    >
    bool ReceiveTo(
        std::array<char, MessageLengthV>& rx_message,
        CallbackT&& complete_callback = [](){},
        std::uint16_t size = MessageLengthV
    ) noexcept
    requires (!std::same_as<RxWorkingModeT, WorkingMode::Blocking>)
    {
        static_assert(
            MessageLengthV <= std::numeric_limits<std::uint16_t>::max(),
            "MessageLengthV must be less than or equal to std::uint16_t max value"
        );
        UartCallbackManagerT::RegisterReceiveCompleteCallback(
            std::move(complete_callback)
        );
        if constexpr (std::same_as<RxWorkingModeT, WorkingMode::Interrupt>){
            return (HAL_OK == HAL_UART_Receive_IT(
                &m_handle,
                reinterpret_cast<std::uint8_t*>(rx_message.data()),
                size
            ));
        } else if constexpr (std::same_as<RxWorkingModeT, WorkingMode::DMA>){
            return (HAL_OK == HAL_UART_Receive_DMA(
                &m_handle,
                reinterpret_cast<std::uint8_t*>(rx_message.data()),
                size
            ));
        }
    }

    /**
     * @brief Transmit data from the provided message buffer in blocking mode.
     * 
     * @tparam MessageLengthV   Length of the message buffer.
     * @tparam TxWorkingModeT   Working mode for transmitting (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param tx_message        Array containing the message to transmit.
     * @param size              Number of bytes to transmit (default is MessageLengthV).
     * 
     * @returns True on success, false otherwise.
     */
    template <
        std::size_t MessageLengthV,
        IsWorkingMode TxWorkingModeT = WorkingModeT,
        IsUartTimeout TimeoutV = UartTimeout<100>
    >
    bool Transmit(
        const std::array<char, MessageLengthV>& tx_message,
        std::uint16_t size = MessageLengthV
    ) noexcept
    requires std::same_as<TxWorkingModeT, WorkingMode::Blocking>
    {
        static_assert(
            MessageLengthV <= std::numeric_limits<std::uint16_t>::max(),
            "MessageLengthV must be less than or equal to std::uint16_t max value"
        );
        return (HAL_OK == HAL_UART_Transmit(
            &m_handle,
            reinterpret_cast<std::uint8_t*>(
                const_cast<char *>(
                    tx_message.data()
                )
            ),
            size,
            TimeoutV::value
        ));
    }

    /**
     * @brief Transmit data from the provided message buffer in non-blocking mode.
     * 
     * @tparam MessageLengthV    Length of the message buffer.
     * @tparam TxWorkingModeT   Working mode for transmitting (default is WorkingModeT).
     * 
     * @param tx_message         Array containing the message to transmit.
     * @param complete_callback  Callback function to be called upon completion.
     * @param size               Number of bytes to transmit (default is MessageLengthV).
     * 
     * @returns True on success, false otherwise.
     */
    template <
        std::size_t MessageLengthV,
        IsWorkingMode TxWorkingModeT = WorkingModeT
    >
    bool Transmit(
        const std::array<char, MessageLengthV>& tx_message,
        [[maybe_unused]] CallbackT&& complete_callback = [](){},
        std::uint16_t size = MessageLengthV
    ) noexcept
    requires (!std::same_as<TxWorkingModeT, WorkingMode::Blocking>)
    {
        static_assert(
            MessageLengthV <= std::numeric_limits<std::uint16_t>::max(),
            "MessageLengthV must be less than or equal to std::uint16_t max value"
        );
        UartCallbackManagerT::RegisterTransmitCompleteCallback(
            std::move(complete_callback)
        );
        if constexpr (std::same_as<TxWorkingModeT, WorkingMode::Interrupt>){
            return (HAL_OK == HAL_UART_Transmit_IT(
                &m_handle,
                reinterpret_cast<std::uint8_t*>(
                    const_cast<char *>(
                        tx_message.data()
                    )
                ),
                size
            ));
        } else if constexpr (std::same_as<TxWorkingModeT, WorkingMode::DMA>){
            return (HAL_OK == HAL_UART_Transmit_DMA(
                &m_handle,
                reinterpret_cast<std::uint8_t*>(
                    const_cast<char *>(
                        tx_message.data()
                    )
                ),
                size
            ));
        }
    }

private:
    UART_HandleTypeDef& m_handle;
};

} /* namespace STM32 */

#endif /* STM32_UART_HPP */
