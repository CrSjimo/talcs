project(TalcsDevice VERSION ${TALCS_VERSION} LANGUAGES CXX)

set(_defines)
set(_include_private)

file(GLOB_RECURSE _src *.h *.cpp)

if(WIN32 AND TALCS_ENABLE_ASIO)
    find_package(asiosdk REQUIRED)
    list(APPEND _defines TALCS_USE_FEATURE_ASIO)
    list(APPEND _include_private ${ASIOSDK_INCLUDE_DIR})
else()
    list(FILTER _src EXCLUDE REGEX ASIO)
endif()

find_package(SDL2 REQUIRED)

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Core
    LINKS_PRIVATE SDL2::SDL2
    DEFINES ${_defines}
    INCLUDE_PRIVATE ${_include_private}
    QT_INCLUDE_PRIVATE Core
)