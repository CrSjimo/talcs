project(TalcsCore VERSION ${TALCS_VERSION} LANGUAGES CXX)

set(CMAKE_AUTOUIC on)
set(CMAKE_AUTOMOC on)
set(CMAKE_AUTORCC on)

file(GLOB_RECURSE _src *.h *.hpp *.cpp *.cc)

if(NOT WIN32 OR NOT TALCS_ENABLE_ASIO)
    list(FILTER _src EXCLUDE REGEX "ASIO*")
endif()

find_path(INTERVAL_TREE_INCLUDE_DIRS "interval-tree/interval_tree.hpp" REQUIRED)

talcs_add_library(${PROJECT_NAME}
    SOURCES ${_src}
    QT_LINKS Core
    QT_INCLUDE_PRIVATE Core
    INCLUDE_PRIVATE ${INTERVAL_TREE_INCLUDE_DIRS}
)