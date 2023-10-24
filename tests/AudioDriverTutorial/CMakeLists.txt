project(tst_talcs_AudioDriverTutorial)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qtmediate_configure_target(${PROJECT_NAME}
    LINKS talcs
    QT_LINKS Core
)