project(tst_talcs_AudioProcessorVisualization)

talcs_skip_without(FORMAT)

set(CMAKE_AUTOUIC on)
set(CMAKE_AUTOMOC on)
set(CMAKE_AUTORCC on)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qm_configure_target(${PROJECT_NAME}
    LINKS talcs::Core talcs::Format
    QT_LINKS Core Gui Widgets
)