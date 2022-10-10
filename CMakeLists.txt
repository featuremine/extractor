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
    GIT_REVISION "api_additions"
    GIT_URL "https://github.com/featuremine/yamal.git"
    TARGETS fmc++ ytp
)
add_subproject(
    NAME yamal-python
    GIT_REVISION "api_additions"
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
execute_process(COMMAND ${PYTHON3_BIN} -c "import sys;import platform;sys.stdout.write('lib.%s-%s-%d.%d' % ((sys.platform,platform.machine()) + sys.version_info[:2]))" OUTPUT_VARIABLE PYTHON3_LIB_DIR)
include(GNUInstallDirs)

add_subdirectory(src)
add_subdirectory(python)

add_custom_target(
    extractor-whl ALL

    COMMAND
    "${PYTHON3_BIN}" "setup.py"

    "build"
    "--build-base=${CMAKE_CURRENT_BINARY_DIR}/python/build"
    ${DEBUG_FLAG}

    "egg_info"
    "--egg-base" "${CMAKE_CURRENT_BINARY_DIR}/python"

    "bdist_wheel"
    "--bdist-dir=${CMAKE_CURRENT_BINARY_DIR}/python/bdist"
    "--dist-dir=${CMAKE_BINARY_DIR}/output"

    COMMAND
    "${PYTHON3_BIN}" "setup.py"

    "build"
    "--build-base=${CMAKE_CURRENT_BINARY_DIR}/python/build"
    ${DEBUG_FLAG}

    "build_scripts"
    "--executable=${PYTHON3_BIN}"
    "-f"

    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
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
        ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/python/build/${PYTHON3_LIB_DIR}:$ENV{PYTHONPATH}:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${CMAKE_CURRENT_BINARY_DIR}/python/build/${PYTHON3_SCRIPTS_DIR}:$ENV{PATH}"
        TIMEOUT 100
    )
endif()
