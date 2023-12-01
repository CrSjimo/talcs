project(TalcsFormat VERSION ${TALCS_VERSION} LANGUAGES CXX)

file(GLOB_RECURSE _src *.h *.cpp)

find_package(SndFile REQUIRED)

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Core
    LINKS_PRIVATE SndFile::sndfile r8brain
    QT_INCLUDE_PRIVATE Core
)