project(TalcsCore VERSION ${TALCS_VERSION} LANGUAGES CXX)

file(GLOB_RECURSE _src *.h *.cpp)

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    QT_LINKS Core
    QT_INCLUDE_PRIVATE Core
)

find_path(INTERVAL_TREE_INCLUDE_DIRS "interval-tree/interval_tree.hpp" REQUIRED)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${INTERVAL_TREE_INCLUDE_DIRS}>"
)

if(TALCS_INSTALL)
    file(RELATIVE_PATH _rel_path ${CMAKE_INSTALL_PREFIX} ${INTERVAL_TREE_INCLUDE_DIRS})
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$<INSTALL_INTERFACE:${_rel_path}>"
    )
endif()