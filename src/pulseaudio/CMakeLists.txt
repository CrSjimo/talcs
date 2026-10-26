project(TalcsPulseAudio VERSION ${TALCS_VERSION} LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSEAUDIO REQUIRED IMPORTED_TARGET libpulse)

file(GLOB _src *.h *.cpp)
talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Device
    LINKS_PRIVATE PkgConfig::PULSEAUDIO
    QT_INCLUDE_PRIVATE Core
)
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN" INSTALL_RPATH_USE_LINK_PATH OFF)
