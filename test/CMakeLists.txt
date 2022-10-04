find_package(PkgConfig)
pkg_check_modules(GTEST REQUIRED IMPORTED_TARGET gtest)

add_executable(
    tests_extractor_time64_serial
    "time64_serial.cpp"
)
target_include_directories(
    tests_extractor_time64_serial
    PUBLIC
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_SRC_DIR}"
)
target_link_libraries(
    tests_extractor_time64_serial
    PUBLIC
    fmc++
    ytp
    rlm
    PkgConfig::GTEST
)
add_test(NAME extractor_time64_serial COMMAND tests_extractor_time64_serial)
