project(tst_talcs_WaveformPainter)

talcs_skip_without(FORMAT GUI)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qm_configure_target(${PROJECT_NAME}
    LINKS talcs::Core talcs::Gui talcs::Format
    QT_LINKS Core Gui Widgets
)