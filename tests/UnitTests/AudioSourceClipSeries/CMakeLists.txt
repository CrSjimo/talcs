project(talcs_UnitTest_AudioSourceClipSeries)

set(CMAKE_AUTOUIC on)
set(CMAKE_AUTOMOC on)
set(CMAKE_AUTORCC on)

file(GLOB _src *.h *.cpp)

add_executable(${PROJECT_NAME} ${_src})

qm_configure_target(${PROJECT_NAME}
    LINKS talcs::Core
    QT_LINKS Core Test
)