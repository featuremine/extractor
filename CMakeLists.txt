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
    GIT_REVISION "cmp_improvement_string_view"
    GIT_URL "https://github.com/featuremine/yamal.git"
    TARGETS fmc++ ytp
)
add_subproject(
    NAME yamal-python
    GIT_REVISION "expose_targets"
    GIT_URL "https://github.com/featuremine/yamal-python.git"
    VARIABLES yamal-python-incdir yamal-python-source
)
add_subproject(
    NAME rlm
    VERSION "1.0.0"
    GIT_REVISION "v1.0.0"
    GIT_URL "https://github.com/featuremine/rlm.git"
    TARGETS rlm
)
add_subproject(
    NAME license
    GIT_REVISION "main"
    GIT_URL "https://github.com/featuremine/license"
    VARIABLES LICENSE_PATH
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

add_subdirectory(src)
add_subdirectory(python)

enable_testing()
add_subdirectory(test)
