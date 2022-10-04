find_package(PkgConfig)
pkg_check_modules(GTEST REQUIRED IMPORTED_TARGET gtest)

macro(extr_unit_test)
    set(oneValueArgs NAME)
    set(multiValueArgs FILES)
    cmake_parse_arguments(ARG
                          ""
                          "${oneValueArgs}"
                          "${multiValueArgs}"
                          ${ARGN}
    )
    add_executable(
        ${ARG_NAME}
        "${ARG_NAME}.cpp"
        ${ARG_FILES}
    )
    target_include_directories(
        ${ARG_NAME}
        PUBLIC
        "${EXTRACTOR_INC_DIR}"
        "${EXTRACTOR_SRC_DIR}"
    )
    target_link_libraries(
        ${ARG_NAME}
        PUBLIC
        fmc++
        ytp
        rlm
        PkgConfig::GTEST
    )
    add_test(NAME ${ARG_NAME} COMMAND ${ARG_NAME})
endmacro()

extr_unit_test(NAME time64_serial)
extr_unit_test(NAME arg_stack FILES "${EXTRACTOR_SRC_DIR}/arg_stack.cpp")
