# SPDX-FileCopyrightText: Copyright (c) 2022-2025 Oğuz Toraman <oguz.toraman@tutanota.com>
# SPDX-License-Identifier: LGPL-3.0-only

cmake_minimum_required(VERSION 3.20)

project(STM32LibraryCollection
    VERSION 1.0.0
    DESCRIPTION "Useful, header-only C++ library collection for STM32 microcontrollers."
    LANGUAGES CXX
)

message(STATUS "STM32LibraryCollection Build type: " ${CMAKE_BUILD_TYPE})

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

set(STM32LibraryCollection_INCLUDE_DIR
    ${STM32LibraryCollection_SOURCE_DIR}/Include/STM32LibraryCollection
)

set(STM32LibraryCollection_HEADER_FILES
    ${STM32LibraryCollection_INCLUDE_DIR}/Adc.hpp
    ${STM32LibraryCollection_INCLUDE_DIR}/Config.hpp
    ${STM32LibraryCollection_INCLUDE_DIR}/Dac.hpp
    ${STM32LibraryCollection_INCLUDE_DIR}/Gpio.hpp
    ${STM32LibraryCollection_INCLUDE_DIR}/Hcsr04.hpp
    ${STM32LibraryCollection_INCLUDE_DIR}/L298n.hpp
    ${STM32LibraryCollection_INCLUDE_DIR}/Pwm.hpp
    ${STM32LibraryCollection_INCLUDE_DIR}/Servo.hpp
    ${STM32LibraryCollection_INCLUDE_DIR}/Timer.hpp
    ${STM32LibraryCollection_INCLUDE_DIR}/Uart.hpp
    ${STM32LibraryCollection_INCLUDE_DIR}/Utility.hpp
)

add_library(STM32LibraryCollection INTERFACE)

add_library(STM32::LibraryCollection ALIAS STM32LibraryCollection)

include(GNUInstallDirs)

target_include_directories(STM32LibraryCollection
    INTERFACE
        $<BUILD_INTERFACE:${STM32LibraryCollection_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set_target_properties(STM32LibraryCollection PROPERTIES
    CXX_STANDARD 23
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
    VERSION ${STM32LibraryCollection_VERSION}
    SOVERSION ${STM32LibraryCollection_VERSION_MAJOR}
)

target_sources(STM32LibraryCollection
    INTERFACE
        FILE_SET headers
        TYPE HEADERS
        BASE_DIRS ./Include
        FILES ${STM32LibraryCollection_HEADER_FILES}
)

install(
    TARGETS
        STM32LibraryCollection
    EXPORT
        STM32LibraryCollectionTargets
    FILE_SET
        headers
    LIBRARY DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/STM32LibraryCollection
    COMPONENT
        Library
)

install(
    EXPORT
        STM32LibraryCollectionTargets
    NAMESPACE
        STM32::
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/STM32LibraryCollection
)
