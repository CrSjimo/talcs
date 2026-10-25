# SPDX-FileCopyrightText: 2023 CrSjimo
# SPDX-License-Identifier: GPL-3.0-only

project(TalcsAsio VERSION ${TALCS_VERSION} LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "talcs::Asio requires Windows.")
endif()

if(NOT TALCS_DEVICE)
    message(FATAL_ERROR "talcs::Asio requires talcs::Device.")
endif()

find_path(TALCS_ASIOSDK_INCLUDE_DIR
    NAMES iasiodrv.h
    PATH_SUFFIXES asiosdk/common
    REQUIRED
)

file(GLOB_RECURSE _src *.h *.cpp)

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Device
    LINKS_PRIVATE ole32 uuid advapi32 user32
    INCLUDE_PRIVATE ${TALCS_ASIOSDK_INCLUDE_DIR}
    QT_INCLUDE_PRIVATE Core
)
