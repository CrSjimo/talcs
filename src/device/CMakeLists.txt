project(TalcsDevice VERSION ${TALCS_VERSION} LANGUAGES CXX)

set(_links_private)

file(GLOB_RECURSE _src *.h *.cpp)

if(WIN32)
    list(APPEND _links_private ole32 uuid avrt)
    list(FILTER _src EXCLUDE REGEX SDL)
else()
    find_package(SDL2 REQUIRED)
    list(APPEND _links_private SDL2::SDL2)
    list(FILTER _src EXCLUDE REGEX WASAPI)
endif()

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Core
    LINKS_PRIVATE ${_links_private}
    QT_INCLUDE_PRIVATE Core
)
