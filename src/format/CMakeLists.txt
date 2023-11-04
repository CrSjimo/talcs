project(TalcsFormat VERSION ${TALCS_VERSION} LANGUAGES CXX)

set(CMAKE_AUTOUIC on)
set(CMAKE_AUTOMOC on)
set(CMAKE_AUTORCC on)

file(GLOB_RECURSE _src *.h *.cpp)

find_package(SndFile REQUIRED)
find_package(SampleRate REQUIRED)

talcs_add_library(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS TalcsCore
    LINKS_PRIVATE SndFile::sndfile SampleRate::samplerate r8brain
    QT_INCLUDE_PRIVATE Core
)