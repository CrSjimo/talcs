project(tst_talcs_midi)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qtmediate_configure_target(${PROJECT_NAME}
    LINKS talcs::Midi talcs::Core talcs::Device
    QT_LINKS Core Gui Widgets
)