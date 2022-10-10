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

add_subdirectory(src)
add_subdirectory(python)

if(BUILD_WHEEL)
    set(WheelPackage_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
    find_package(WheelPackage REQUIRED)

    python_package(
        NAME extractor
        SHORT_DESCRIPTION "Featuremine extractor tests"
        LONG_DESCRIPTION "Featuremine extractor tests"
        PACKAGES extractor
        TESTS_PACKAGE "extractor.tests"
        SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python"
        BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/python"
        INSTALL_REQUIRES "yamal"
    )
endif()

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(test)
    if(TEST_EXTENSIONS)
        if(BUILD_WHEEL)
            test_python_package(
                NAME extractor_py
                PACKAGE extractor
                TIMEOUT 45
                TEST test-extractor-python

                ENVIRONMENT
                "YAMALCOMPPATH=${MODULES_DIR}"
            )
        endif()
    endif()
endif()