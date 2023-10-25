project(TalcsRemote VERSION ${TALCS_VERSION} LANGUAGES CXX)

set(CMAKE_AUTOUIC on)
set(CMAKE_AUTOMOC on)
set(CMAKE_AUTORCC on)

file(GLOB_RECURSE _src *.h *.cpp)

find_package(rpclib REQUIRED)

talcs_add_library(${PROJECT_NAME}
    SOURCES ${_src}
    LINKS TalcsDevice rpclib::rpc
    QT_INCLUDE_PRIVATE Core
)
