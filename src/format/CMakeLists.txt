project(TalcsFormat VERSION ${TALCS_VERSION} LANGUAGES CXX)

file(GLOB_RECURSE _src *.h *.cpp *.mm)

set(_links_private
    SndFile::sndfile
    unofficial::r8brain-free-src::r8bsrc
    WavPack::WavPack
)

if(WIN32)
    list(APPEND _links_private mfplat mfreadwrite mfuuid ole32)
else()
    list(FILTER _src EXCLUDE REGEX "MediaFoundationAACAudioFormatIO")
endif()

if(APPLE)
    enable_language(OBJCXX)
    set(CMAKE_OBJCXX_STANDARD 17)
    set(CMAKE_OBJCXX_STANDARD_REQUIRED ON)
    find_library(TALCS_AUDIO_TOOLBOX_FRAMEWORK AudioToolbox)
    if(NOT TALCS_AUDIO_TOOLBOX_FRAMEWORK)
        message(FATAL_ERROR "AudioToolbox framework not found")
    endif()
    list(APPEND _links_private ${TALCS_AUDIO_TOOLBOX_FRAMEWORK})
else()
    list(FILTER _src EXCLUDE REGEX "CoreAudioAACAudioFormatIO")
endif()

find_package(SndFile REQUIRED)
find_package(wavpack CONFIG REQUIRED)
find_package(unofficial-r8brain-free-src REQUIRED)

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Core
    LINKS_PRIVATE ${_links_private}
    QT_INCLUDE_PRIVATE Core
)
