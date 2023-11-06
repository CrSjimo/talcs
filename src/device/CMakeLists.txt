project(TalcsDevice VERSION ${TALCS_VERSION} LANGUAGES CXX)

set(CMAKE_AUTOUIC on)
set(CMAKE_AUTOMOC on)
set(CMAKE_AUTORCC on)

set(_defines)
set(_include_private)

file(GLOB_RECURSE _src *.h *.cpp)

if(WIN32 AND TALCS_ENABLE_ASIO)
    if(NOT TALCS_ASIOSDK_DIR)
        message(FATAL_ERROR "TALCS: TALCS_ASIOSDK_DIR not specified.")
    endif()

    list(APPEND _defines TALCS_USE_FEATURE_ASIO)
    list(APPEND _include_private ${TALCS_ASIOSDK_DIR}/common)
else()
    list(FILTER _src EXCLUDE REGEX ASIO)
endif()

find_package(SDL2 REQUIRED)

talcs_add_library(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS talcs::Core
    LINKS_PRIVATE SDL2::SDL2
    DEFINES ${_defines}
    INCLUDE_PRIVATE ${_include_private}
    QT_INCLUDE_PRIVATE Core
)