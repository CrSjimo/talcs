cmake_minimum_required(VERSION 3.17)

project(talcs VERSION 0.0.1.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)

# ----------------------------------
# Build Options
# ----------------------------------
option(TALCS_BUILD_STATIC "Build static libraries" OFF)
option(TALCS_BUILD_TRANSLATIONS "Build translations" ON)
option(TALCS_BUILD_TESTS "Build test cases" OFF)
option(TALCS_BUILD_DOCUMENTATIONS "Build documentations" OFF)
option(TALCS_INSTALL "Install library" ON)

option(TALCS_DEVICE "Build talcs::Device library" ON)
option(TALCS_DEVICE_ENABLE_SDL "Enable SDL in talcs::Device library" ON)
option(TALCS_DEVICE_ENABLE_ASIO "Enable ASIO in talcs::Device library" OFF)
option(TALCS_FORMAT "Build talcs::Format library" ON)
option(TALCS_MIDI "Build talcs::Midi library" ON)
option(TALCS_REMOTE "Build talcs::Remote library" ON)
option(TALCS_JUCE_ADAPTER "Build talcs::JuceAdapter library" OFF)
option(TALCS_DSPX "Build talcs::Dspx library" OFF)


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

add_subdirectory(lib)

if(TALCS_BUILD_TESTS)
    add_subdirectory(tests)
endif()

# ----------------------------------
# Documentation
# ----------------------------------
if(TALCS_BUILD_DOCUMENTATIONS)
    if(NOT DOXYGEN_EXECUTABLE)
        find_package(Doxygen REQUIRED)
    endif()

    set(_install_options)

    if(TALCS_INSTALL)
        set(_install_options INSTALL_DIR share/doc/${TALCS_INSTALL_NAME})
    endif()

    set(TALCS_DOXYGEN_TAGFILE ${CMAKE_BUILD_SHARE_DIR}/doc/${TALCS_INSTALL_NAME}/talcs_tagfile.xml
        CACHE FILEPATH "talcs doxygen tag file path" FORCE
    )

    set(_doc_targets TalcsCore TalcsDevice TalcsFormat TalcsMidi TalcsRemote)

    qm_import(Doxygen)
    qm_setup_doxygen(talcs_RunDoxygen
        NAME "TALCS"
        DESCRIPTION "TALCS: Audio Library of CrSjimo"
        MDFILE README.md
        OUTPUT_DIR ${CMAKE_BUILD_SHARE_DIR}/doc/${TALCS_INSTALL_NAME}
        INPUT src doc
        TARGETS ${_doc_targets}
        DEPENDS ${_doc_targets}
        NO_EXPAND_MACROS Q_OBJECT Q_GADGET Q_DECLARE_TR_FUNCTIONS
        COMPILE_DEFINITIONS Q_SIGNALS=Q_SIGNALS Q_SLOTS=Q_SLOTS
        GENERATE_TAGFILE "${TALCS_DOXYGEN_TAGFILE}"
        ${_install_options}
    )
endif()