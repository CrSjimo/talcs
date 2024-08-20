project(tst_talcs_RemoteAudioDevice)

talcs_skip_without(DEVICE FORMAT REMOTE)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qm_configure_target(${PROJECT_NAME}
    LINKS talcs::Core talcs::Device talcs::Format talcs::Remote
    QT_LINKS Core Gui Widgets
)