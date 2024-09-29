project(tst_talcs_WavpackAudioFormatIO)

talcs_skip_without(FORMAT)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qm_configure_target(${PROJECT_NAME}
    LINKS talcs::Core talcs::Format
    QT_LINKS Core Gui Widgets
)