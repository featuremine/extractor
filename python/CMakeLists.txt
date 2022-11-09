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
)
target_link_libraries(
    py_extractor_obj
    PUBLIC
    fmc++
    ytp
    extractor
)
target_include_directories(
    py_extractor_obj
    PUBLIC
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_PYTHON_INC_DIR}"
    "${EXTRACTOR_PYTHON_SRC_DIR}"
    "${EXTRACTOR_SRC_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${NUMPY_INCLUDE_DIR}"
    "${yamal-python-incdir}"
    "${PYTHON_INCLUDE_DIRS}"
)
target_compile_definitions(
    py_extractor_obj
    PRIVATE
    PY_EXTR_VER="${PROJECT_VERSION}"
    NO_DLL_DECORATOR
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
    "${EXTRACTOR_PYTHON_INC_DIR}"
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
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/python_dist/extractor"
)
target_include_directories(
    extractor_module
    PRIVATE
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_PYTHON_INC_DIR}"
    "${EXTRACTOR_PYTHON_SRC_DIR}"
    "${EXTRACTOR_SRC_DIR}"
    "${PYTHON_INCLUDE_DIRS}"
)
file(
    COPY ${PROJECT_SOURCE_DIR}/python/include
    DESTINATION "${CMAKE_BINARY_DIR}/python_dist/extractor"
    FILES_MATCHING
        PATTERN "*.h"
)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_options(extractor_module PRIVATE -Wl,-rpath,@loader_path)
    target_link_options(extractor_module PRIVATE -undefined dynamic_lookup)
else()
    set_property(TARGET extractor_module PROPERTY BUILD_RPATH_USE_ORIGIN TRUE)
endif()
