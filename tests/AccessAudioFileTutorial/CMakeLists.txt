project(tst_talcs_AccessAudioFileTutorial)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qm_configure_target(${PROJECT_NAME}
    LINKS talcs::Core talcs::Format talcs::Device
    QT_LINKS Core Gui Widgets
)