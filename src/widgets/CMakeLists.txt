project(TalcsWidgets VERSION ${TALCS_VERSION} LANGUAGES CXX)

file(GLOB_RECURSE _src *.h *.cpp)

talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    QT_LINKS Core Gui Widgets
    LINKS talcs::Core talcs::Format talcs::Device
    QT_INCLUDE_PRIVATE Core Gui Widgets
)
