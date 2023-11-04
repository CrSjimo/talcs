project(tst_talcs_r8brain)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qtmediate_configure_target(${PROJECT_NAME}
    LINKS talcs::Core talcs::Format
    QT_LINKS Core
)