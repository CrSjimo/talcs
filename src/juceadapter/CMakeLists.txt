project(TalcsJuceAdapter VERSION ${TALCS_VERSION} LANGUAGES CXX)

file(GLOB_RECURSE _src *.h *.cpp)

find_package(JUCE CONFIG REQUIRED)

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Core
    LINKS_PRIVATE juce::juce_audio_basics
    QT_INCLUDE_PRIVATE Core
)