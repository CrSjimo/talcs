project(TalcsFormat VERSION ${TALCS_VERSION} LANGUAGES CXX)

file(GLOB_RECURSE _src *.h *.cpp)

find_package(SndFile REQUIRED)
find_package(wavpack CONFIG REQUIRED)
find_package(unofficial-r8brain-free-src REQUIRED)

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Core
    LINKS_PRIVATE SndFile::sndfile unofficial::r8brain-free-src::r8bsrc WavPack::WavPack
    QT_INCLUDE_PRIVATE Core
)