project(TalcsDevice VERSION ${TALCS_VERSION} LANGUAGES CXX)

set(_defines)
set(_include_private)
set(_links_private)

file(GLOB_RECURSE _src *.h *.cpp)

if(WIN32 AND TALCS_DEVICE_ENABLE_ASIO)
    if(NOT TALCS_ASIOSDK_DIR)
        message(FATAL_ERROR "TALCS: TALCS_ASIOSDK_DIR not specified.")
    endif()
    list(APPEND _defines TALCS_USE_FEATURE_ASIO)
    file(TO_CMAKE_PATH ${TALCS_ASIOSDK_DIR} _asiosdk_dir)
    list(APPEND _include_private ${_asiosdk_dir}/common)
else()
    list(FILTER _src EXCLUDE REGEX ASIO)
endif()


if(TALCS_DEVICE_ENABLE_SDL)
    find_package(SDL2 REQUIRED)
    list(APPEND _links_private SDL2::SDL2)
    list(APPEND _defines TALCS_USE_FEATURE_SDL)
endif()

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Core
    LINKS_PRIVATE ${_links_private}
    DEFINES ${_defines}
    INCLUDE_PRIVATE ${_include_private}
    QT_INCLUDE_PRIVATE Core
)