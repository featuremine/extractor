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
    GIT_REVISION "v7.2.25"
    VERSION "7.2.25"
    GIT_URL "https://github.com/featuremine/yamal.git"
    TARGETS fmc++ ytp py_yamal_dep
)

option (BUILD_TESTING "Enable build of the unit tests and their execution." ON)
option (BUILD_TOOLS "Enable build of command line tools." ON)
option (BUILD_PACKAGE "Enable build of the self-extracting package." ON)
option (BUILD_WHEEL "Enable build of the python package." ON)
option (TEST_EXTENSIONS "Enable testing the extensions." ON)

set(EXTRACTOR_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(EXTRACTOR_PYTHON_INC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python/include")
set(EXTRACTOR_PYTHON_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/python/src")
set(EXTRACTOR_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(EXTRACTOR_BIN_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/bin")
set(EXTRACTOR_LIB_DIR "${CMAKE_CURRENT_BINARY_DIR}/package/lib")
set(WHEEL_extractor_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/python")

set(FmConfig_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(FmConfig REQUIRED)
fm_config()
include(GNUInstallDirs)

add_subdirectory(src)

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
    "${WHEEL_extractor_BUILD_DIR}/dist/extractor/__init__.py"
    COPYONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/extractor/tests/__init__.py"
    "${WHEEL_extractor_BUILD_DIR}/dist/extractor/tests/__init__.py"
    COPYONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/extractor/tests/arithmetic.py"
    "${WHEEL_extractor_BUILD_DIR}/dist/extractor/tests/arithmetic.py"
    COPYONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/extractor/tests/component.py"
    "${WHEEL_extractor_BUILD_DIR}/dist/extractor/tests/component.py"
    COPYONLY
)

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/scripts/test-extractor-python"
    "${WHEEL_extractor_BUILD_DIR}/dist/scripts/test-extractor-python"
    COPYONLY
)

file(
    COPY "${PROJECT_SOURCE_DIR}/LICENSE"
    DESTINATION "${WHEEL_extractor_BUILD_DIR}/dist"
)

add_subdirectory(python)

if(BUILD_TESTING)
    enable_testing()
    find_program( MEMORYCHECK_COMMAND valgrind )
    set(MEMORYCHECK_COMMAND_OPTIONS "--log-fd=2 --verbose --trace-children=yes --leak-check=full --show-leak-kinds=all --track-origins=yes")
    set(VALGRIND_COMMAND_OPTIONS "--log-fd=2 --verbose --trace-children=yes --leak-check=full --show-leak-kinds=all --track-origins=yes")
    include(CTest)
    add_subdirectory(test)

    if(TEST_EXTENSIONS)
        if(BUILD_WHEEL)
            set(WheelPackage_DIR "${PROJECT_SOURCE_DIR}/cmake")
            find_package(WheelPackage REQUIRED)
            test_python_package(
                NAME extractor_py
                PACKAGE extractor
                TIMEOUT 100
                TEST "test-extractor-python"
            )
        endif()
    endif()
endif()

set(CPACK_PACKAGE_VENDOR "Featuremine")
set(CPACK_PACKAGE_CONTACT "support@featuremine.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}-${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)
