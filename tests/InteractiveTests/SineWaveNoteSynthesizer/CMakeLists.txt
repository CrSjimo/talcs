project(tst_talcs_SineWaveNoteSynthesizer)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qm_configure_target(${PROJECT_NAME}
    LINKS talcs::Core talcs::Device
    QT_LINKS Core
)