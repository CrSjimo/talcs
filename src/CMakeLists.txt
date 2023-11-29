set(TALCS_PROJECT_DESCRIPTION "TALCS: Audio Library of CrSjimo")
set(TALCS_PROJECT_COPYRIGHT "Copyright 2022-2023 CrSjimo")
set(TALCS_GENERATED_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../include)

# ----------------------------------
# CMake API
# ----------------------------------
function(talcs_add_library _target)
    set(options)
    set(oneValueArgs PREFIX)
    set(multiValueArgs)
    cmake_parse_arguments(FUNC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    set(CMAKE_AUTOUIC on)
    set(CMAKE_AUTOMOC on)
    set(CMAKE_AUTORCC on)

    if(TALCS_BUILD_STATIC)
        set(_type STATIC)
    else()
        set(_type SHARED)
    endif()

    add_library(${_target} ${_type})

    if(WIN32 AND NOT TALCS_BUILD_STATIC)
        qtmediate_add_win_rc(${_target}
            DESCRIPTION ${TALCS_PROJECT_DESCRIPTION}
            COPYRIGHT ${TALCS_PROJECT_COPYRIGHT}
        )
    endif()

    if(FUNC_PREFIX)
        set(_prefix_option PREFIX ${FUNC_PREFIX})
    else()
        set(_prefix_option)
    endif()

    # Set global definitions
    qtmediate_export_defines(${_target} ${_prefix_option})

    # Configure target
    qtmediate_configure_target(${_target} ${FUNC_UNPARSED_ARGUMENTS})

    # Library name
    string(LENGTH "${_target}" _stringLength)
    math(EXPR _startIndex "5")
    math(EXPR _length "${_stringLength} - ${_startIndex}")
    string(SUBSTRING "${_target}" ${_startIndex} ${_length} _name)
    add_library(talcs::${_name} ALIAS ${_target})
    set_target_properties(${_target} PROPERTIES EXPORT_NAME ${_name})

    if(TALCS_INSTALL)
        install(TARGETS ${_target}
            EXPORT ${TALCS_INSTALL_NAME}Targets
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" OPTIONAL
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}" OPTIONAL
        )

        target_include_directories(${PROJECT_NAME} PUBLIC
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${TALCS_INSTALL_NAME}>"
        )

        set(_gen_include_options INSTALL_DIR "${CMAKE_INSTALL_INCLUDEDIR}/${TALCS_INSTALL_NAME}/${_target}")
    else()
        set(_gen_include_options)
    endif()

    # Generate a standard include directory in build directory
    qtmediate_sync_include(. "${TALCS_GENERATED_INCLUDE_DIR}/${_target}" ${_gen_include_options})
    target_include_directories(${PROJECT_NAME} PUBLIC
        "$<BUILD_INTERFACE:${TALCS_GENERATED_INCLUDE_DIR}>"
    )

    target_include_directories(${PROJECT_NAME} PRIVATE .)

    # Add to translate targets
    set_property(GLOBAL APPEND PROPERTY TALCS_TRANSLATE_TARGETS ${_target})
endfunction()

# ----------------------------------
# Main Project
# ----------------------------------
add_subdirectory(core)

add_subdirectory(device)

add_subdirectory(format)

add_subdirectory(remote)

add_subdirectory(synthesis)

add_subdirectory(midi)

add_subdirectory(3rdparty)

# ----------------------------------
# Documentation
# ----------------------------------
if(TALCS_BUILD_DOCUMENTATIONS)
    if(NOT DOXYGEN_EXECUTABLE)
        find_package(Doxygen REQUIRED)
    endif()

    set(_install_options)

    if(TALCS_INSTALL)
        set(_install_options INSTALL_DIR share/doc/${TALCS_INSTALL_NAME})
    endif()

    set(TALCS_DOXYGEN_TAGFILE ${CMAKE_BUILD_SHARE_DIR}/doc/${TALCS_INSTALL_NAME}/talcs_tagfile.xml
        CACHE FILEPATH "talcs doxygen tag file path" FORCE
    )

    set(_doc_targets TalcsCore TalcsDevice TalcsFormat TalcsRemote TalcsSynthesis)

    qtmediate_setup_doxygen(talcs_RunDoxygen
        NAME "TALCS"
        DESCRIPTION "TALCS: Audio Library of CrSjimo"
        MDFILE ../README.md
        OUTPUT_DIR ${CMAKE_BUILD_SHARE_DIR}/doc/${TALCS_INSTALL_NAME}
        INPUT core device format remote synthesis ../doc
        TARGETS ${_doc_targets}
        DEPENDS ${_doc_targets}
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