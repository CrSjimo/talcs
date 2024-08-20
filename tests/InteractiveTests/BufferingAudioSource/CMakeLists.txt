project(tst_talcs_BufferingAudioSource)

talcs_skip_without(DEVICE FORMAT)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qm_configure_target(${PROJECT_NAME}
    LINKS talcs::Core talcs::Device talcs::Format
    QT_LINKS Core Gui Widgets
)