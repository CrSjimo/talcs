project(TalcsDevice VERSION ${TALCS_VERSION} LANGUAGES CXX)

set(_links_private)

file(GLOB _src *.h *.cpp)
list(FILTER _src EXCLUDE REGEX SDL)

if(WIN32)
    file(GLOB _platform_src built-in/WASAPI*.h built-in/WASAPI*.cpp)
    list(APPEND _links_private ole32 uuid avrt)
elseif(APPLE)
    file(GLOB _platform_src built-in/CoreAudio*.h built-in/CoreAudio*.cpp)
    find_library(TALCS_CORE_AUDIO_FRAMEWORK CoreAudio REQUIRED)
    find_library(TALCS_AUDIO_UNIT_FRAMEWORK AudioUnit REQUIRED)
    find_library(TALCS_CORE_FOUNDATION_FRAMEWORK CoreFoundation REQUIRED)
    list(APPEND _links_private
        ${TALCS_CORE_AUDIO_FRAMEWORK}
        ${TALCS_AUDIO_UNIT_FRAMEWORK}
        ${TALCS_CORE_FOUNDATION_FRAMEWORK}
    )
else()
    file(GLOB _platform_src SDL*.h SDL*.cpp built-in/SDL*.h built-in/SDL*.cpp)
    find_package(SDL2 REQUIRED)
    list(APPEND _links_private SDL2::SDL2)
endif()
list(APPEND _src ${_platform_src})

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Core
    LINKS_PRIVATE ${_links_private}
    QT_INCLUDE_PRIVATE Core
)
