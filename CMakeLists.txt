cmake_minimum_required(VERSION 3.15)

file (STRINGS "VERSION" EXTRACTOR_VERSION)

project(
    extractor
    VERSION "${EXTRACTOR_VERSION}"
    DESCRIPTION "Featuremine extractor component"
    HOMEPAGE_URL "https://www.featuremine.com"
)

set(Subproject_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Subproject REQUIRED)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_subproject(
    NAME yamal
    VERSION "7.2.23"
    GIT_REVISION "v7.2.23"
    GIT_URL "https://github.com/featuremine/yamal.git"
    TARGETS fmc++ ytp
)
add_subproject(
    NAME yamal-python
    VERSION "7.2.20"
    GIT_REVISION "v7.2.20"
    GIT_URL "https://github.com/featuremine/yamal-python.git"
    VARIABLES yamal-python-incdir yamal-python-source
)

option (BUILD_TESTING "Enable build of the unit tests and their execution." ON)
option (BUILD_TOOLS "Enable build of command line tools." ON)
option (BUILD_PACKAGE "Enable build of the self-extracting package." ON)
option (BUILD_WHEEL "Enable build of the python package." ON)
option (TEST_EXTENSIONS "Enable testing the extensions." ON)

set(EXTRACTOR_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(EXTRACTOR_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(EXTRACTOR_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/bin")
set(EXTRACTOR_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/lib")

set(FmConfig_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(FmConfig REQUIRED)
fm_config()
execute_process(COMMAND ${PYTHON3_BIN} -c "import sys;sys.stdout.write('scripts-%d.%d' % sys.version_info[:2])" OUTPUT_VARIABLE PYTHON3_SCRIPTS_DIR)
include(GNUInstallDirs)

add_subdirectory(src)
add_subdirectory(python)

install(
    DIRECTORY "${PROJECT_SOURCE_DIR}/include/extractor"
    DESTINATION "./include"
    FILES_MATCHING
    PATTERN "*.h"
    PATTERN "*.hpp"
)
configure_file(
    ${PROJECT_SOURCE_DIR}/lib/pkgconfig/extractor.pc.in
    ${PROJECT_BINARY_DIR}/extractor.pc
    @ONLY
)
install(
    FILES "${PROJECT_BINARY_DIR}/extractor.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)
configure_file(
    ${PROJECT_SOURCE_DIR}/lib/pkgconfig/py_extractor.pc.in
    ${PROJECT_BINARY_DIR}/py_extractor.pc
    @ONLY
)
install(
    FILES "${PROJECT_BINARY_DIR}/py_extractor.pc"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig"
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/extractor/__init__.py"
    "${CMAKE_BINARY_DIR}/python_dist/extractor/__init__.py"
    COPYONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/extractor/tests/__init__.py"
    "${CMAKE_BINARY_DIR}/python_dist/extractor/tests/__init__.py"
    COPYONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/extractor/tests/component.py"
    "${CMAKE_BINARY_DIR}/python_dist/extractor/tests/component.py"
    COPYONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/test-extractor-python"
    "${CMAKE_BINARY_DIR}/python_dist/scripts/test-extractor-python"
    COPYONLY
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PYTHON_PLATFORM "manylinux_2_17_${CMAKE_SYSTEM_PROCESSOR}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    execute_process(COMMAND sw_vers -productVersion
                    OUTPUT_VARIABLE MACOS_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\1"
           MACOS_VERSION_MAJOR "${MACOS_VERSION}")
    string(REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+).*$" "\\2"
           MACOS_VERSION_MINOR "${MACOS_VERSION}")
    set(PYTHON_PLATFORM "macosx_${MACOS_VERSION_MAJOR}_${MACOS_VERSION_MINOR}_${CMAKE_SYSTEM_PROCESSOR}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PYTHON_PLATFORM "win_${CMAKE_SYSTEM_PROCESSOR}")
else()
    message(FATAL_ERROR "Platform not supported")
endif()

add_custom_target(
    extractor-whl ALL

    COMMAND
    "${PYTHON3_BIN}" "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"

    "build"
    "--build-base=${CMAKE_CURRENT_BINARY_DIR}/python_dist/build"
    ${DEBUG_FLAG}

    "egg_info"
    "--egg-base" "${CMAKE_CURRENT_BINARY_DIR}/python_dist"

    "bdist_wheel"
    "--bdist-dir=${CMAKE_CURRENT_BINARY_DIR}/python_dist/bdist"
    "--dist-dir=${CMAKE_BINARY_DIR}/output"

    "--plat-name=${PYTHON_PLATFORM}"

    COMMAND
    "${PYTHON3_BIN}" "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"

    "build"
    "--build-base=${CMAKE_CURRENT_BINARY_DIR}/python_dist/build"
    ${DEBUG_FLAG}

    "build_scripts"
    "--executable=${PYTHON3_BIN}"
    "-f"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python_dist

    DEPENDS extractor extractor_module
)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
    find_program(PYTHON3_BIN "python3" REQUIRED)
    add_test(
        NAME extractor_py COMMAND

        "/bin/sh"
        "-c"
        "test-extractor-python"

        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python
    )
    set_tests_properties(
        extractor_py
        PROPERTIES
        ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python_dist/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${CMAKE_CURRENT_BINARY_DIR}/python_dist/build/${PYTHON3_SCRIPTS_DIR}:$ENV{PATH}"
        TIMEOUT 100
    )
endif()

set(CPACK_PACKAGE_VENDOR "Featuremine")
set(CPACK_PACKAGE_CONTACT "support@featuremine.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)
