cmake_minimum_required(VERSION 3.15)

file (STRINGS "VERSION" EXTRACTOR_VERSION)

project(
    extractor
    VERSION "${EXTRACTOR_VERSION}"
    DESCRIPTION "Featuremine extractor component"
    HOMEPAGE_URL "https://www.featuremine.com"
)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option (BUILD_TESTING "Enable build of the unit tests and their execution." ON)

set(Subproject_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(BUILD_TESTING)
    find_package(Subproject REQUIRED)
    add_subproject(
        NAME libgtest
        GIT_REVISION "v1.14.0"
        GIT_URL "https://github.com/google/googletest.git"
        TARGETS gtest
    )
endif()

find_package(Subproject REQUIRED)
add_subproject(
    NAME yamal
    VERSION_MIN "8.0.6"
    VERSION_MAX "8.1.0"
    GIT_REVISION "v8.0.7"
    GIT_URL "https://github.com/featuremine/yamal.git"
    TARGETS fmc++ ytp py_yamal_dep yamal-py
)

option (BUILD_TOOLS "Enable build of command line tools." ON)
option (BUILD_PACKAGE "Enable build of the self-extracting package." ON)
option (BUILD_WHEEL "Enable build of the python package." ON)
option (TEST_EXTENSIONS "Enable testing the extensions." ON)
option (BUILD_DOCUMENTATION "Build documentation." ON)

set(EXTRACTOR_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(EXTRACTOR_PYTHON_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python/include")
set(EXTRACTOR_PYTHON_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python/src")
set(EXTRACTOR_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(EXTRACTOR_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/bin")
set(EXTRACTOR_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/${CMAKE_INSTALL_LIBDIR}")
set(WHEEL_extractor_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/python" CACHE INTERNAL "Wheel path for extractor" FORCE)
set(EXTRACTOR_API_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/api/include" CACHE INTERNAL "API include directory path for extractor" FORCE)
set(EXTRACTOR_PYTHON_API_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/api/python/include" CACHE INTERNAL "Python API include directory path for extractor" FORCE)
set(package_FILES "")
set(package_DEP "")

if (BUILD_DOCUMENTATION)
    set(FmDocumentation_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    find_package(FmDocumentation REQUIRED)
    add_documentation(
        NAME extractor-docs
        BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/docs"
        SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/docs"
        TITLE "Extractor Documentation"
        VERSION "${PROJECT_VERSION}"
    )
endif()

set(FmConfig_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(FmConfig REQUIRED)
fm_config()
include(GNUInstallDirs)

set(ReleaseTools_DIR "${PROJECT_SOURCE_DIR}/cmake")
find_package(ReleaseTools REQUIRED)
get_python_platform(PYTHON_PLATFORM)

add_subdirectory(src)

function(extractor_install_directory)
    cmake_parse_arguments(
        ARG
        ""
        "DIRECTORY;DESTINATION;PATTERN"
        ""
        ${ARGN}
    )
    install(${ARGN})
    file(
        GLOB_RECURSE FILES
        LIST_DIRECTORIES OFF
        RELATIVE "${ARG_DIRECTORY}/.."
        "${ARG_DIRECTORY}/${ARG_PATTERN}"
    )
    foreach (FILE ${FILES})
        set(FILE_DEST "${PROJECT_BINARY_DIR}/package/${ARG_DESTINATION}/${FILE}")
        cmake_path(NORMAL_PATH FILE_DEST)
        cmake_path(GET FILE_DEST PARENT_PATH FILE_DIR)
        add_custom_command(
            OUTPUT
            "${FILE_DEST}"

            COMMAND
            ${CMAKE_COMMAND} -E make_directory
            "${FILE_DIR}"

            COMMAND
            ${CMAKE_COMMAND} -E copy
            "${ARG_DIRECTORY}/../${FILE}"
            "${FILE_DEST}"

            DEPENDS
            "${ARG_DIRECTORY}/../${FILE}"
        )
        list(APPEND package_FILES "${FILE_DEST}")
        list(APPEND package_DEP "${FILE_DEST}")
        set(package_FILES ${package_FILES} PARENT_SCOPE)
        set(package_DEP ${package_DEP} PARENT_SCOPE)
    endforeach ()
endfunction()

extractor_install_directory(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/extractor"
    DESTINATION "./include"
    FILES_MATCHING
    PATTERN "*.h"
)
extractor_install_directory(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/extractor"
    DESTINATION "./include"
    FILES_MATCHING
    PATTERN "*.hpp"
)

configure_file(
    "${PROJECT_SOURCE_DIR}/lib/pkgconfig/extractor.pc.in"
    "${EXTRACTOR_LIB_DIR}/pkgconfig/extractor.pc"
    @ONLY
)
install(
    FILES "${EXTRACTOR_LIB_DIR}/pkgconfig/extractor.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
list(APPEND package_FILES "${EXTRACTOR_LIB_DIR}/pkgconfig/extractor.pc")
list(APPEND package_DEP "${EXTRACTOR_LIB_DIR}/pkgconfig/extractor.pc")

configure_file(
    "${PROJECT_SOURCE_DIR}/lib/pkgconfig/py_extractor.pc.in"
    "${EXTRACTOR_LIB_DIR}/pkgconfig/py_extractor.pc"
    @ONLY
)
install(
    FILES "${EXTRACTOR_LIB_DIR}/pkgconfig/py_extractor.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
list(APPEND package_FILES "${EXTRACTOR_LIB_DIR}/pkgconfig/py_extractor.pc")
list(APPEND package_DEP "${EXTRACTOR_LIB_DIR}/pkgconfig/py_extractor.pc")

add_subdirectory(python)

macro(add_lib_to_package TARGET)
    add_custom_command(
        OUTPUT
        "${EXTRACTOR_LIB_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}${TARGET}${CMAKE_STATIC_LIBRARY_SUFFIX}"

        COMMAND
        ${CMAKE_COMMAND} -E copy
        "$<TARGET_FILE:${TARGET}>"
        "${EXTRACTOR_LIB_DIR}/$<TARGET_FILE_NAME:${TARGET}>"

        DEPENDS
        ${TARGET}
    )
    list(APPEND package_DEP "${EXTRACTOR_LIB_DIR}/$<TARGET_FILE_NAME:${TARGET}>")
    list(APPEND package_FILES "${EXTRACTOR_LIB_DIR}/$<TARGET_FILE_NAME:${TARGET}>")
endmacro()

add_lib_to_package("extractor")
add_lib_to_package("py_extractor")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/output")
add_custom_command(
    OUTPUT
    "${CMAKE_BINARY_DIR}/output/extractor-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}.tar.gz"

    COMMAND
    ${CMAKE_COMMAND} -E chdir "${CMAKE_CURRENT_BINARY_DIR}/package"
    ${CMAKE_COMMAND} -E tar "czvf" "${CMAKE_BINARY_DIR}/output/extractor-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}.tar.gz"
    ${package_FILES}

    DEPENDS
    ${package_DEP}
)

add_custom_target(
    extractor-package ALL
    DEPENDS
    "${CMAKE_BINARY_DIR}/output/extractor-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}.tar.gz"
)

if(BUILD_TESTING)
    enable_testing()
    find_program( MEMORYCHECK_COMMAND valgrind )
    set(MEMORYCHECK_COMMAND_OPTIONS "--log-fd=2 --verbose --trace-children=yes --leak-check=full --show-leak-kinds=all --track-origins=yes")
    set(VALGRIND_COMMAND_OPTIONS "--log-fd=2 --verbose --trace-children=yes --leak-check=full --show-leak-kinds=all --track-origins=yes")
    include(CTest)
    add_subdirectory(test)
endif()

set(CPACK_PACKAGE_VENDOR "Featuremine")
set(CPACK_PACKAGE_CONTACT "support@featuremine.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)
