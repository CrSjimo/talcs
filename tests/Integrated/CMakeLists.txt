project(tst_talcs_Integrated)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qtmediate_configure_target(${PROJECT_NAME}
    LINKS talcs::Core talcs::Device talcs::Format
    QT_LINKS Core Gui Widgets
)