/* SPDX-FileCopyrightText: Copyright (c) 2022-2026 Oğuz Toraman <oguz.toraman@tutanota.com> */
/* SPDX-License-Identifier: LGPL-3.0-only */

#ifndef STM32_SPI_HPP
#define STM32_SPI_HPP

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ranges>

#include "Config.hpp"
#include "__Internal/__Utility.hpp"

#include "main.h"

#if !defined(HAL_SPI_MODULE_ENABLED) /* module check */
#error "HAL SPI module is not enabled!"
#endif /* module check */

#if !defined(HAL_DMA_MODULE_ENABLED) /* module check */
#error "HAL DMA module is not enabled!"
#endif /* module check */

#if (USE_HAL_SPI_REGISTER_CALLBACKS != 1) /* module check */
#error "HAL SPI callbacks are not enabled!"
#endif /* module check */

namespace STM32 {

/**
 * @struct SpiTimeout, A utility struct to hold the SPI operation timeout value.
 * 
 * @tparam TimeoutV   SPI operation timeout value in milliseconds.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Spi.hpp>
 *
 * using MySpiTimeout = STM32::SpiTimeout<200>;
 * auto timeout = MySpiTimeout::value; // timeout is 200ms.
 * @endcode
 */
template <std::uint32_t TimeoutV>
struct SpiTimeout : __Internal::__Constant<std::uint32_t, TimeoutV> {
    static_assert(
        TimeoutV > 0,
        "Timeout must be greater than zero"
    );
};

/**
 * @brief IsSpiTimeout, A concept to check if a type is a SpiTimeout.
 * 
 * @tparam T        Type to be checked.
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Spi.hpp>
 * 
 * static_assert(STM32::IsSpiTimeout<STM32::SpiTimeout<100>>);
 * static_assert(!STM32::IsSpiTimeout<int>);
 * @endcode
 */
template <typename T>
concept IsSpiTimeout =
    __Internal::__IsConstant<T> &&
    std::same_as<typename T::ValueTypeT, std::uint32_t> &&
    T::value > 0;

/**
 * @brief IsSpiMessage, A concept to check if a type is a valid SPI message buffer.
 * 
 * Valid types must be:
 * - Contiguous range (data stored in continuous memory)
 * - Sized range (size() is available)
 * - std::uint8_t value type (SPI operates on bytes)
 * 
 * @tparam T        Type to be checked.
 * 
 * @note Accepts both fixed-size containers (std::array) and dynamic containers
 *       (std::vector). Buffer sizes exceeding 65535 bytes will be silently
 *       clamped to 65535 (std::uint16_t max) during transmission/reception.
 * 
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Spi.hpp>
 * static_assert(STM32::IsSpiMessage<std::array<std::uint8_t, 100>>);  // OK
 * static_assert(STM32::IsSpiMessage<std::vector<std::uint8_t>>);      // OK
 * static_assert(!STM32::IsSpiMessage<std::array<int, 100>>);          // Fails: not uint8_t
 * static_assert(!STM32::IsSpiMessage<int>);                           // Fails: not a range
 * @endcode
 */
template <typename T>
concept IsSpiMessage = __Internal::__IsMessage<T, std::uint8_t>;

/**
 * @class Spi, A class to manage SPI functionality on STM32 microcontrollers.
 * 
 * SPI (Serial Peripheral Interface) is a synchronous, full-duplex serial protocol.
 * This class supports:
 * - Transmit only (Transmit)
 * - Receive only (ReceiveTo)
 * - Simultaneous transmit and receive (TransmitReceive)
 * 
 * @tparam WorkingModeT         Working mode of the SPI
 *                              (WorkingMode::Blocking, WorkingMode::Interrupt, WorkingMode::DMA).
 * @tparam UniqueTagT           Unique tag type to differentiate multiple Spi instances.
 *                              UniqueTagT must be STM32_UNIQUE_TAG.
 *
 * @note Spi class is non-copyable and non-movable.
 * @note CS/NSS pin management is the user's responsibility (manual GPIO or hardware NSS).
 *
 * @example Usage:
 * @code {.cpp}
 * #include <STM32LibraryCollection/Spi.hpp>
 *
 * SPI_HandleTypeDef hspi1; // Assume properly initialized by CubeMX
 *
 * // Create SPI with DMA as default mode
 * STM32::Spi<STM32::WorkingMode::DMA, STM32_UNIQUE_TAG> spi{hspi1};
 *
 * std::array<std::uint8_t, 10> tx_data{0x01, 0x02, 0x03};
 * std::array<std::uint8_t, 10> rx_data{};
 *
 * // 1. Transmit only (e.g., sending command to display)
 * spi.Transmit(tx_data, [](){
 *     // TX complete - can deassert CS now
 * });
 *
 * // 2. Receive only (e.g., reading sensor data)
 * spi.ReceiveTo(rx_data, [](){
 *     // RX complete - process rx_data
 * });
 *
 * // 3. Full-duplex: transmit command while receiving response
 * spi.TransmitReceive(tx_data, rx_data, [](){
 *     // TX/RX complete - tx_data sent, rx_data filled
 * });
 *
 * // 4. Override mode per-operation
 * spi.Transmit<STM32::WorkingMode::Blocking>(tx_data);
 *
 * // 5. Blocking mode with custom timeout
 * spi.TransmitReceive<STM32::WorkingMode::Blocking, STM32::SpiTimeout<500>>(tx_data, rx_data);
 * @endcode
 */
template <IsWorkingMode WorkingModeT, __Internal::__IsUniqueTag UniqueTagT>
class Spi {
    using TransmitCompleteCallbackT = __Internal::__CallbackManager<
        SPI_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
        HAL_SPI_RegisterCallback, HAL_SPI_UnRegisterCallback, HAL_SPI_TX_COMPLETE_CB_ID
    >;
    using ReceiveCompleteCallbackT = __Internal::__CallbackManager<
        SPI_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
        HAL_SPI_RegisterCallback, HAL_SPI_UnRegisterCallback, HAL_SPI_RX_COMPLETE_CB_ID
    >;
    using TransmitReceiveCompleteCallbackT = __Internal::__CallbackManager<
        SPI_HandleTypeDef, UniqueTagT, STM32_UNIQUE_TAG,
        HAL_SPI_RegisterCallback, HAL_SPI_UnRegisterCallback, HAL_SPI_TX_RX_COMPLETE_CB_ID
    >;
public:

    /**
     * @brief Construct Spi class.
     * 
     * @param handle        Reference to the SPI handle.
     * 
     * @note HAL callbacks are automatically registered via RAII.
     */
    explicit Spi(SPI_HandleTypeDef& handle) noexcept
      : m_handle{handle},
        m_transmit_complete_callback{handle},
        m_receive_complete_callback{handle},
        m_transmit_receive_complete_callback{handle}
    { }

    /**
     * @defgroup Deleted copy and move members.
     * @{
     */
    Spi(const Spi&) = delete;
    Spi& operator=(const Spi&) = delete;
    Spi(Spi&&) = delete;
    Spi& operator=(Spi&&) = delete;
    /** @} */

    /**
     * @brief Destroy Spi class.
     * 
     * @note Callbacks are automatically unregistered via RAII.
     */
    ~Spi() = default;

    /**
     * @returns SPI handle reference.
     */
    [[nodiscard]]
    auto&& GetHandle(this auto&& self) noexcept
    {
        return std::forward<decltype(self)>(self).m_handle;
    }

    /* ======================== Receive Operations ======================== */

    /**
     * @brief Receive data into the provided buffer in blocking mode.
     * 
     * @tparam RxWorkingModeT   Working mode for receiving (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param rx_message        A contiguous range to store the received data.
     * 
     * @returns True on success, false otherwise.
     *
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsWorkingMode RxWorkingModeT = WorkingModeT,
        IsSpiTimeout TimeoutV = SpiTimeout<100>
    >
    bool ReceiveTo(
        IsSpiMessage auto& rx_message
    ) noexcept
    requires std::same_as<RxWorkingModeT, WorkingMode::Blocking>
    {
        return (HAL_OK == HAL_SPI_Receive(
            &m_handle,
            std::ranges::data(rx_message),
            __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message)),
            TimeoutV::value
        ));
    }

    /**
     * @brief Receive data into the provided buffer in non-blocking mode.
     * 
     * @tparam RxWorkingModeT   Working mode for receiving (default is WorkingModeT).
     * 
     * @param rx_message        A contiguous range to store the received data.
     * @param complete_callback Callback function to be called upon completion.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsWorkingMode RxWorkingModeT = WorkingModeT
    >
    bool ReceiveTo(
        IsSpiMessage auto& rx_message,
        CallbackT&& complete_callback = [](){}
    ) noexcept
    requires (!std::same_as<RxWorkingModeT, WorkingMode::Blocking>)
    {
        m_receive_complete_callback.Set(
            std::move(complete_callback)
        );
        if constexpr (std::same_as<RxWorkingModeT, WorkingMode::Interrupt>) {
            return (HAL_OK == HAL_SPI_Receive_IT(
                &m_handle,
                std::ranges::data(rx_message),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message))
            ));
        } else if constexpr (std::same_as<RxWorkingModeT, WorkingMode::DMA>) {
            return (HAL_OK == HAL_SPI_Receive_DMA(
                &m_handle,
                std::ranges::data(rx_message),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(rx_message))
            ));
        }
    }

    /* ======================== Transmit Operations ======================== */

    /**
     * @brief Transmit data from the provided buffer in blocking mode.
     * 
     * @tparam TxWorkingModeT   Working mode for transmitting (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param tx_message        A contiguous range containing the data to transmit.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsWorkingMode TxWorkingModeT = WorkingModeT,
        IsSpiTimeout TimeoutV = SpiTimeout<100>
    >
    bool Transmit(
        const IsSpiMessage auto& tx_message
    ) noexcept
    requires std::same_as<TxWorkingModeT, WorkingMode::Blocking>
    {
        return (HAL_OK == HAL_SPI_Transmit(
            &m_handle,
            const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
            __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message)),
            TimeoutV::value
        ));
    }

    /**
     * @brief Transmit data from the provided buffer in non-blocking mode.
     * 
     * @tparam TxWorkingModeT    Working mode for transmitting (default is WorkingModeT).
     * 
     * @param tx_message         A contiguous range containing the data to transmit.
     * @param complete_callback  Callback function to be called upon completion.
     * 
     * @returns True on success, false otherwise.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsWorkingMode TxWorkingModeT = WorkingModeT
    >
    bool Transmit(
        const IsSpiMessage auto& tx_message,
        CallbackT&& complete_callback = [](){}
    ) noexcept
    requires (!std::same_as<TxWorkingModeT, WorkingMode::Blocking>)
    {
        m_transmit_complete_callback.Set(
            std::move(complete_callback)
        );
        if constexpr (std::same_as<TxWorkingModeT, WorkingMode::Interrupt>) {
            return (HAL_OK == HAL_SPI_Transmit_IT(
                &m_handle,
                const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message))
            ));
        } else if constexpr (std::same_as<TxWorkingModeT, WorkingMode::DMA>) {
            return (HAL_OK == HAL_SPI_Transmit_DMA(
                &m_handle,
                const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
                __Internal::__ClampMessageLength<std::uint16_t>(std::ranges::size(tx_message))
            ));
        }
    }

    /* ==================== Transmit-Receive Operations ==================== */

    /**
     * @brief Simultaneously transmit and receive data in blocking mode.
     * 
     * This is full-duplex SPI operation: data is transmitted from tx_message
     * while simultaneously receiving into rx_message.
     * 
     * @tparam TxRxWorkingModeT Working mode for the operation (default is WorkingModeT).
     * @tparam TimeoutV         Timeout for blocking mode (default is 100ms).
     * 
     * @param tx_message        A contiguous range containing the data to transmit.
     * @param rx_message        A contiguous range to store the received data.
     * 
     * @returns True on success, false otherwise.
     * 
     * @note tx_message and rx_message must have the same size. The smaller size
     *       is used if they differ.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsWorkingMode TxRxWorkingModeT = WorkingModeT,
        IsSpiTimeout TimeoutV = SpiTimeout<100>
    >
    bool TransmitReceive(
        const IsSpiMessage auto& tx_message,
        IsSpiMessage auto& rx_message
    ) noexcept
    requires std::same_as<TxRxWorkingModeT, WorkingMode::Blocking>
    {
        const auto size = __Internal::__ClampMessageLength<std::uint16_t>(
            std::min(std::ranges::size(tx_message), std::ranges::size(rx_message))
        );
        return (HAL_OK == HAL_SPI_TransmitReceive(
            &m_handle,
            const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
            std::ranges::data(rx_message),
            size,
            TimeoutV::value
        ));
    }

    /**
     * @brief Simultaneously transmit and receive data in non-blocking mode.
     * 
     * This is full-duplex SPI operation: data is transmitted from tx_message
     * while simultaneously receiving into rx_message.
     * 
     * @tparam TxRxWorkingModeT Working mode for the operation (default is WorkingModeT).
     * 
     * @param tx_message        A contiguous range containing the data to transmit.
     * @param rx_message        A contiguous range to store the received data.
     * @param complete_callback Callback function to be called upon completion.
     * 
     * @returns True on success, false otherwise.
     * 
     * @note tx_message and rx_message must have the same size. The smaller size
     *       is used if they differ.
     * 
     * @warning Buffer sizes exceeding 65535 bytes are silently clamped to 65535.
     */
    template <
        IsWorkingMode TxRxWorkingModeT = WorkingModeT
    >
    bool TransmitReceive(
        const IsSpiMessage auto& tx_message,
        IsSpiMessage auto& rx_message,
        CallbackT&& complete_callback = [](){}
    ) noexcept
    requires (!std::same_as<TxRxWorkingModeT, WorkingMode::Blocking>)
    {
        m_transmit_receive_complete_callback.Set(
            std::move(complete_callback)
        );
        const auto size = __Internal::__ClampMessageLength<std::uint16_t>(
            std::min(std::ranges::size(tx_message), std::ranges::size(rx_message))
        );
        if constexpr (std::same_as<TxRxWorkingModeT, WorkingMode::Interrupt>) {
            return (HAL_OK == HAL_SPI_TransmitReceive_IT(
                &m_handle,
                const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
                std::ranges::data(rx_message),
                size
            ));
        } else if constexpr (std::same_as<TxRxWorkingModeT, WorkingMode::DMA>) {
            return (HAL_OK == HAL_SPI_TransmitReceive_DMA(
                &m_handle,
                const_cast<std::uint8_t*>(std::ranges::data(tx_message)),
                std::ranges::data(rx_message),
                size
            ));
        }
    }

private:
    SPI_HandleTypeDef& m_handle;
    TransmitCompleteCallbackT m_transmit_complete_callback;
    ReceiveCompleteCallbackT m_receive_complete_callback;
    TransmitReceiveCompleteCallbackT m_transmit_receive_complete_callback;
};

} /* namespace STM32 */

#endif /* STM32_SPI_HPP */
