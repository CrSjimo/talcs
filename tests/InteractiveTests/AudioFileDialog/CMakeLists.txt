project(tst_talcs_AudioFileDialog)

talcs_skip_without(WIDGETS)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qm_configure_target(${PROJECT_NAME}
    LINKS talcs::Widgets
    QT_LINKS Core Gui Widgets
)