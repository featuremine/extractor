find_package(PkgConfig)
pkg_check_modules(PYTHON IMPORTED_TARGET python3-embed)
if (NOT PYTHON_FOUND)
    pkg_check_modules(PYTHON REQUIRED IMPORTED_TARGET python3)
endif (NOT PYTHON_FOUND)

find_program(PYTHON3_BIN "python3" REQUIRED)
execute_process(COMMAND ${PYTHON3_BIN} -c
    "import numpy; print(numpy.get_include())"
    OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE)

add_library(
    py_extractor_obj
    STATIC
    "src/book/book.cpp"
    "src/extractor.cpp"
    "src/decimal.c"
)
target_link_libraries(
    py_extractor_obj
    PUBLIC
    fmc++
    ytp
    extractor
    py_yamal_dep
)
target_include_directories(
    py_extractor_obj
    PUBLIC
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_API_INC_DIR}"
    "${EXTRACTOR_PYTHON_INC_DIR}"
    "${EXTRACTOR_PYTHON_API_INC_DIR}"
    "${EXTRACTOR_PYTHON_SRC_DIR}"
    "${EXTRACTOR_SRC_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${NUMPY_INCLUDE_DIR}"
    "${PYTHON_INCLUDE_DIRS}"
)
target_compile_definitions(
    py_extractor_obj
    PRIVATE
    PY_EXTR_VER="${PROJECT_VERSION}"
    NO_DLL_DECORATOR
    UNIVERSAL
)

add_library(
    py_extractor_dep
    INTERFACE
)
target_link_libraries(
    py_extractor_dep
    INTERFACE
    fmc++
    ytp
    extractor
)
target_include_directories(
    py_extractor_dep
    INTERFACE
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_API_INC_DIR}"
    "${EXTRACTOR_PYTHON_INC_DIR}"
    "${EXTRACTOR_PYTHON_API_INC_DIR}"
    "${EXTRACTOR_PYTHON_SRC_DIR}"
    "${PYTHON_INCLUDE_DIRS}"
)

add_library(
    py_extractor
    STATIC
    $<TARGET_OBJECTS:py_extractor_obj>
)
target_link_libraries(
    py_extractor
    PRIVATE
    py_extractor_dep
)

set_target_properties(
    py_extractor
    PROPERTIES
    OUTPUT_NAME "py_extractor"
    LIBRARY_OUTPUT_DIRECTORY "${EXTRACTOR_LIB_DIR}"
)
install(
    TARGETS
    py_extractor
)

add_library(
    extractor_module
    SHARED
    "src/extractor_main.cpp"
)
target_link_libraries(
    extractor_module
    PRIVATE
    fmc++
    ytp
    extractor
    py_extractor
)
set_target_properties(
    extractor_module
    PROPERTIES
    OUTPUT_NAME "extractor"
    PREFIX ""
    SUFFIX ".so"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/dist/extractor"
)
target_include_directories(
    extractor_module
    PRIVATE
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_API_INC_DIR}"
    "${EXTRACTOR_PYTHON_INC_DIR}"
    "${EXTRACTOR_PYTHON_API_INC_DIR}"
    "${EXTRACTOR_PYTHON_SRC_DIR}"
    "${EXTRACTOR_SRC_DIR}"
    "${PYTHON_INCLUDE_DIRS}"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(extractor_module PRIVATE -Wl,-rpath,@loader_path)
    target_link_options(extractor_module PRIVATE -undefined dynamic_lookup)
else()
    set_property(TARGET extractor_module PROPERTY BUILD_RPATH_USE_ORIGIN TRUE)
endif()

file(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/include
    DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/dist/extractor"
    FILES_MATCHING
    PATTERN "*.h"
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

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
    set(DEBUG_FLAG "--debug")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(PYTHON_PLATFORM "manylinux_2_17_${CMAKE_SYSTEM_PROCESSOR}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    execute_process(COMMAND sw_vers -productVersion
                    OUTPUT_VARIABLE MACOS_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE)
    STRING(REGEX REPLACE "^([0-9]+)\\.?([0-9]+)\\.?([0-9]+)?$" "\\1" MACOS_VERSION_MAJOR "${MACOS_VERSION}")
    set(PYTHON_PLATFORM "macosx_${MACOS_VERSION_MAJOR}_0_${CMAKE_SYSTEM_PROCESSOR}")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PYTHON_PLATFORM "win_${CMAKE_SYSTEM_PROCESSOR}")
else()
    message(FATAL_ERROR "Platform not supported")
endif()

if(BUILD_WHEEL)
    add_custom_command(
        OUTPUT "${CMAKE_BINARY_DIR}/output/extractor-${PROJECT_VERSION}-py3-none-manylinux_2_17_x86_64.whl"

        COMMAND
        "${PYTHON3_BIN}" "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"

        "build"
        "--build-base=${WHEEL_extractor_BUILD_DIR}/build"
        "--build-lib=${WHEEL_extractor_BUILD_DIR}/build/lib"
        ${DEBUG_FLAG}

        "egg_info"
        "--egg-base" "${WHEEL_extractor_BUILD_DIR}"

        "bdist_wheel"
        "--bdist-dir=${WHEEL_extractor_BUILD_DIR}/bdist"
        "--dist-dir=${CMAKE_BINARY_DIR}/output"

        "--plat-name=${PYTHON_PLATFORM}"

        COMMAND
        "${PYTHON3_BIN}" "${CMAKE_CURRENT_SOURCE_DIR}/setup.py"

        "build_scripts"
        "--executable=${PYTHON3_BIN}"

        "build"
        "--build-base=${WHEEL_extractor_BUILD_DIR}/build"
        "--build-lib=${WHEEL_extractor_BUILD_DIR}/build/lib"
        "--build-scripts=${WHEEL_extractor_BUILD_DIR}/build/scripts"
        ${DEBUG_FLAG}

        WORKING_DIRECTORY ${WHEEL_extractor_BUILD_DIR}/dist

        DEPENDS extractor extractor_module
    )
    add_custom_target(
        extractor-whl ALL
        DEPENDS "${CMAKE_BINARY_DIR}/output/extractor-${PROJECT_VERSION}-py3-none-manylinux_2_17_x86_64.whl"
    )
endif()

if(BUILD_TESTING)
    enable_testing()
    if(TEST_EXTENSIONS)
        if(BUILD_WHEEL)
            set(WheelPackage_DIR "${PROJECT_SOURCE_DIR}/cmake")
            find_package(WheelPackage REQUIRED)
            test_python_package(
                NAME extractor_py
                PACKAGE extractor
                TIMEOUT 100
                TEST "test-extractor-python"
                PYTHONPATH "${WHEEL_yamal_BUILD_DIR}/build/lib"
            )

            add_custom_target(
                extractor_py ALL
                DEPENDS yamal-py
            )
        
        endif()
    endif()
endif()
