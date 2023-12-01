cmake_minimum_required(VERSION 3.17)

project(talcs VERSION 0.0.1.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# ----------------------------------
# Build Options
# ----------------------------------
option(TALCS_BUILD_STATIC "Build static libraries" OFF)
option(TALCS_BUILD_TRANSLATIONS "Build translations" ON)
option(TALCS_BUILD_TESTS "Build test cases" ON)
option(TALCS_BUILD_DOCUMENTATIONS "Build documentations" OFF)
option(TALCS_INSTALL "Install library" ON)
option(TALCS_ENABLE_ASIO "Enable ASIO feature" OFF)

# ----------------------------------
# CMake Settings
# ----------------------------------
if(MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /manifest:no")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /manifest:no")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")

    if(NOT DEFINED CMAKE_DEBUG_POSTFIX)
        set(CMAKE_DEBUG_POSTFIX "d")
    endif()
endif()

if(TALCS_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

# ----------------------------------
# Project Variables
# ----------------------------------
set(TALCS_VERSION ${PROJECT_VERSION})
set(TALCS_INSTALL_NAME ${PROJECT_NAME})
set(TALCS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# ----------------------------------
# Find basic dependencies
# ----------------------------------
find_package(qmsetup REQUIRED)

qm_import(Filesystem)
qm_init_directories()

# ----------------------------------
# Main Project
# ----------------------------------
add_subdirectory(src)

if(TALCS_BUILD_TESTS)
    add_subdirectory(tests)
endif()