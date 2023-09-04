# ----------------------------------
# CMake API
# ----------------------------------
function(talcs_add_library _target _type)
    add_library(${_target} ${_type} ${ARGN})

    if(WIN32)
        set(RC_VERSION_STRING ${TALCS_INSTALL_NAME})
        include(${QTMEDIATE_MODULES_DIR}/winrc.cmake)
    endif()

    if(TALCS_INSTALL)
        install(TARGETS ${_target}
            EXPORT ${TALCS_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        )

        target_include_directories(${PROJECT_NAME} PUBLIC
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TALCS_INSTALL_NAME}>"
        )
    endif()

    set_property(GLOBAL APPEND PROPERTY TALCS_TRANSLATE_TARGETS ${_target})
endfunction()

function(talcs_add_plugin _target _category)
    add_library(${_target} SHARED ${ARGN})
    add_library(talcs::${_target} ALIAS ${_target})

    if(WIN32)
        set(RC_VERSION_STRING ${TALCS_VERSION})
        include(${TALCS_MODULES_DIR}/winrc.cmake)
    endif()

    set(_output_dir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${TALCS_INSTALL_NAME}/plugins/${_category})
    set_target_properties(${_target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${_output_dir}
        ARCHIVE_OUTPUT_DIRECTORY ${_output_dir}
        LIBRARY_OUTPUT_DIRECTORY ${_output_dir}
    )

    if(TALCS_INSTALL)
        install(TARGETS ${_target}
            RUNTIME DESTINATION "${CMAKE_INSTALL_LIBDIR}/${TALCS_INSTALL_NAME}/plugins/${_category}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/${TALCS_INSTALL_NAME}/plugins/${_category}" OPTIONAL
        )
    endif()

    set_property(GLOBAL APPEND PROPERTY TRANSLATE_TARGETS ${_target})
endfunction()

function(talcs_include_directories _target)
    foreach(_dir ${ARGN})
        get_filename_component(_abs_dir ${_dir} ABSOLUTE)
        file(RELATIVE_PATH _rel_path ${TALCS_SOURCE_DIR} ${_abs_dir})
        target_include_directories(${_target} PUBLIC
            "$<BUILD_INTERFACE:${_abs_dir}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TALCS_INSTALL_NAME}/${_rel_path}>"
        )
    endforeach()
endfunction()

macro(talcs_install_headers _dir)
    if(TALCS_INSTALL)
        get_filename_component(_abs_dir ${_dir} ABSOLUTE)
        file(RELATIVE_PATH _rel_path ${TALCS_SOURCE_DIR} ${_abs_dir})
        install(DIRECTORY ${_abs_dir}/
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${TALCS_INSTALL_NAME}/${_rel_path}
            FILES_MATCHING PATTERN "*.h"
        )
    endif()
endmacro()

# ----------------------------------
# Main Project
# ----------------------------------
set(CMAKE_AUTOUIC on)
set(CMAKE_AUTOMOC on)
set(CMAKE_AUTORCC on)

find_package(SDL2 REQUIRED)
find_package(SndFile REQUIRED)
find_package(SampleRate REQUIRED)

if(NOT TARGET qtmediate::Core)
    find_package(qtmediate REQUIRED)
endif()

include(${QTMEDIATE_MODULES_DIR}/qtmediateMacros.cmake)

set(RC_DESCRIPTION "TALCS: Audio Library of CrSjimo")
set(RC_COPYRIGHT "Copyright 2022-2023 CrSjimo")

# Platform libs
set(_platform_libs)

if(APPLE)
    list(APPEND _platform_libs "-framework CoreFoundation")
endif()

if(NOT WIN32)
    list(APPEND _platform_libs dl)
endif()

# Configure target
file(GLOB_RECURSE _src *.h *.hpp *.cpp *.cc)

if(WIN32 AND TALCS_ENABLE_ASIO)
    if(NOT TALCS_ASIOSDK_DIR)
        message(FATAL_ERROR "TALCS: TALCS_ASIOSDK_DIR not specified.")
    endif()
    target_compile_definitions(${PROJECT_NAME} PUBLIC TALCS_USE_FEATURE_ASIO)
    target_include_directories(${PROJECT_NAME} PRIVATE ${TALCS_ASIOSDK_DIR}/common)
else()
    list(FILTER _src EXCLUDE REGEX "ASIO*")
endif()

talcs_add_library(${PROJECT_NAME} SHARED ${_src})

set_target_properties(${PROJECT_NAME} PROPERTIES EXPORT_NAME Talcs)

target_compile_definitions(${PROJECT_NAME} PRIVATE TALCS_LIBRARY)

# Link
target_link_libraries(${PROJECT_NAME} PUBLIC qtmediate::Core)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${_platform_libs}
    SDL2::SDL2 SndFile::sndfile SampleRate::samplerate
)

qtmediate_link_qt_libraries(${PROJECT_NAME} PUBLIC
    Core
)

# Include
talcs_include_directories(${PROJECT_NAME}
    .
)

qtmediate_include_qt_private(${PROJECT_NAME} PRIVATE
    Core
)

# Extra
talcs_install_headers(.)

if(TALCS_BUILD_DOCUMENTATIONS)
    if(NOT DOXYGEN_EXECUTABLE)
        find_package(Doxygen REQUIRED)
    endif()

    include(${QTMEDIATE_MODULES_DIR}/doxygen/doxygen.cmake)

    set(_install_options)

    if(TALCS_INSTALL)
        set(_install_options INSTALL_DIR share/doc/${TALCS_INSTALL_NAME})
    endif()

    set(TALCS_DOXYGEN_TAGFILE ${CMAKE_BUILD_SHARE_DIR}/doc/${TALCS_INSTALL_NAME}/talcs_tagfile.xml
        CACHE FILEPATH "talcs doxygen tag file path" FORCE
    )

    setup_doxygen_command(talcs_RunDoxygen
        NAME "TALCS"
        DESCRIPTION "TALCS: Audio Library of CrSjimo"
        MDFILE ../README.md
        OUTPUT_DIR ${CMAKE_BUILD_SHARE_DIR}/doc/${TALCS_INSTALL_NAME}
        INPUT .
        TARGETS talcs
        DEPENDS talcs
        NO_EXPAND_MACROS Q_OBJECT Q_GADGET Q_DECLARE_TR_FUNCTIONS
        COMPILE_DEFINITIONS Q_SIGNALS=Q_SIGNALS Q_SLOTS=Q_SLOTS
        GENERATE_TAGFILE "${TALCS_DOXYGEN_TAGFILE}"
        ${_install_options}
    )
endif()

if(TALCS_INSTALL)
    # Add install target
    set(_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${TALCS_INSTALL_NAME})

    # Add version file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${TALCS_INSTALL_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )

    # Add configuration file
    configure_package_config_file(
        ${CMAKE_CURRENT_LIST_DIR}/${TALCS_INSTALL_NAME}Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/${TALCS_INSTALL_NAME}Config.cmake"
        INSTALL_DESTINATION ${_install_dir}
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
    )

    # Install cmake files
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/${TALCS_INSTALL_NAME}Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/${TALCS_INSTALL_NAME}ConfigVersion.cmake"
        DESTINATION ${_install_dir}
    )

    # Install cmake targets files
    install(EXPORT ${TALCS_INSTALL_NAME}Targets
        FILE "${TALCS_INSTALL_NAME}Targets.cmake"
        NAMESPACE talcs::
        DESTINATION ${_install_dir}
    )
endif()