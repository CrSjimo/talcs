project(TalcsCore VERSION ${TALCS_VERSION} LANGUAGES CXX)

set(CMAKE_AUTOUIC on)
set(CMAKE_AUTOMOC on)
set(CMAKE_AUTORCC on)

file(GLOB_RECURSE _src *.h *.cpp)

find_path(INTERVAL_TREE_INCLUDE_DIRS "interval-tree/interval_tree.hpp" REQUIRED)

talcs_add_library(${PROJECT_NAME}
    SOURCES ${_src}
    QT_LINKS Core
    QT_INCLUDE_PRIVATE Core
)

target_include_directories(${PROJECT_NAME} PUBLIC ${INTERVAL_TREE_INCLUDE_DIRS})