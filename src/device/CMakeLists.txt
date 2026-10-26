project(TalcsDevice VERSION ${TALCS_VERSION} LANGUAGES CXX)

set(_links_private)
set(_platform_src)

file(GLOB _src *.h *.cpp)

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
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(GLOB _platform_src built-in/NativeAudioDevice*.h built-in/NativeAudioDevice*.cpp)
    list(APPEND _links_private ${CMAKE_DL_LIBS})
endif()
list(APPEND _src ${_platform_src})

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Core
    LINKS_PRIVATE ${_links_private}
    QT_INCLUDE_PRIVATE Core
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN" INSTALL_RPATH_USE_LINK_PATH OFF)
endif()
