project(TalcsGui VERSION ${TALCS_VERSION} LANGUAGES CXX)

file(GLOB_RECURSE _src *.h *.cpp)

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    QT_LINKS Core Gui
    LINKS talcs::Core
    QT_INCLUDE_PRIVATE Core Gui
)
