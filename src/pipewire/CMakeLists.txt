project(TalcsPipeWire VERSION ${TALCS_VERSION} LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PIPEWIRE REQUIRED IMPORTED_TARGET libpipewire-0.3>=0.3.65)

file(GLOB _src *.h *.cpp)
talcs_add_library(${PROJECT_NAME} AUTOGEN
    SOURCES ${_src}
    LINKS talcs::Device
    LINKS_PRIVATE PkgConfig::PIPEWIRE
    QT_INCLUDE_PRIVATE Core
)
set_target_properties(${PROJECT_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN" INSTALL_RPATH_USE_LINK_PATH OFF)
