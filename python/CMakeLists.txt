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
    "book/py_book.cpp"
    "py_extractor.cpp"
)
target_link_libraries(
    py_extractor_obj
    PUBLIC
    fmc++
    ytp
    PkgConfig::PYTHON
    extractor
)
target_include_directories(
    py_extractor_obj
    PUBLIC
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_SRC_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${NUMPY_INCLUDE_DIR}"
    "${yamal-python-incdir}"
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
    PkgConfig::PYTHON
    extractor
)
target_include_directories(
    py_extractor_dep
    INTERFACE
    "${EXTRACTOR_INC_DIR}"
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

add_library(
    extractor_module
    SHARED
    "py_extractor_main.cpp"
)
target_link_libraries(
    extractor_module
    PRIVATE
    fmc++
    ytp
    extractor
    py_extractor
    PkgConfig::PYTHON
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
    "${EXTRACTOR_SRC_DIR}"
)
