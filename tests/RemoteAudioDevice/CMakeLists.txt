project(tst_talcs_RemoteAudioDevice)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qtmediate_configure_target(${PROJECT_NAME}
    LINKS talcs
    QT_LINKS Core Gui Widgets
)