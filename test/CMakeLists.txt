find_package(PkgConfig)
pkg_check_modules(PYTHON IMPORTED_TARGET python3-embed)
if (NOT PYTHON_FOUND)
    pkg_check_modules(PYTHON REQUIRED IMPORTED_TARGET python3)
endif (NOT PYTHON_FOUND)

function(download_archive URL MD5 FILENAME OUTPUT)
    get_filename_component(DIR "${OUTPUT}" DIRECTORY)
    file(MAKE_DIRECTORY "${DIR}")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/downloads")
    file(
        DOWNLOAD
        "${URL}"
        "${CMAKE_BINARY_DIR}/downloads/${FILENAME}"
        EXPECTED_MD5 ${MD5}
    )
    add_custom_command(
        OUTPUT "${OUTPUT}"
        DEPENDS "${CMAKE_BINARY_DIR}/downloads/${FILENAME}"
        COMMAND /bin/sh -c "gunzip -c '${CMAKE_BINARY_DIR}/downloads/${FILENAME}' >'${OUTPUT}'"
    )
endfunction()

download_archive(
    "https://fm-public-resources.s3.amazonaws.com/extractor-test-files/ytpseq_ore_bbo_trades.2.ytp.0001.gz"
    "82c03d684ca8bee7c63d8067d697bb2f" "ytpseq_ore_bbo_trades.2.ytp.0001.gz"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo_trades.ytp.0001"
)

download_archive(
    "https://fm-public-resources.s3.amazonaws.com/extractor-test-files/ytpseq_ore_bbo_trades.2.ytp.0002.gz"
    "9ffec2f7fadf4cea1e6f267bb22c8916" "ytpseq_ore_bbo_trades.2.ytp.0002.gz"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo_trades.ytp.0002"
)

download_archive(
    "https://fm-public-resources.s3.amazonaws.com/extractor-test-files/ytpseq_ore_bbo_trades.2.ytp.0002.gz"
    "9ffec2f7fadf4cea1e6f267bb22c8916" "ytpseq_ore_bbo_trades.2.ytp.0002.gz"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo_trades_partial.ytp.0002"
)

download_archive(
    "https://fm-public-resources.s3.amazonaws.com/extractor-test-files/coinbase_bulldozer.1.base.ytp.gz"
    "9163c76b619f58bdbf333289f7863228" "coinbase_bulldozer.1.base.ytp.gz"
    "${CMAKE_CURRENT_BINARY_DIR}/coinbase_bulldozer.base.ytp"
)

download_archive(
    "https://fm-public-resources.s3.amazonaws.com/extractor-test-files/ytpseq_ore_bbo.1.ytp.gz"
    "ad3e5f5be3d60a707d18bc51d4d5cbb9" "ytpseq_ore_bbo.1.ytp.gz"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo.ytp"
)

download_archive(
    "https://fm-public-resources.s3.amazonaws.com/extractor-test-files/ytpseq_ore_trades.1.ytp.gz"
    "c0beeafa482e0f4df2cf990bdda96a56" "ytpseq_ore_trades.1.ytp.gz"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_trades.ytp"
)

download_archive(
    "https://fm-public-resources.s3.amazonaws.com/extractor-test-files/consolidated.ytp.0001.gz"
    "f715bfbce52f70dbcd3476392028d6ae" "consolidated.ytp.0001.gz"
    "${CMAKE_CURRENT_BINARY_DIR}/consolidated.ytp.0001"
)

download_archive(
    "https://fm-public-resources.s3.amazonaws.com/extractor-test-files/trade_view.base.csv.gz"
    "5ef6ea21f7d2fa100ef977505ec52a21" "trade_view.base.csv.gz"
    "${CMAKE_CURRENT_BINARY_DIR}/trade_view.base.csv"
)

add_custom_target(
    extractor-testfiles ALL
    DEPENDS
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo_trades.ytp.0001"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo_trades.ytp.0002"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo_trades_partial.ytp.0002"
    "${CMAKE_CURRENT_BINARY_DIR}/coinbase_bulldozer.base.ytp"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo.ytp"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_trades.ytp"
    "${CMAKE_CURRENT_BINARY_DIR}/consolidated.ytp.0001"
    "${CMAKE_CURRENT_BINARY_DIR}/trade_view.base.csv"
)

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
        "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_NAME}.cpp"
        ${ARG_FILES}
    )
    target_include_directories(
        ${ARG_NAME}
        PUBLIC
        "${EXTRACTOR_INC_DIR}"
        "${EXTRACTOR_API_INC_DIR}"
        "${EXTRACTOR_PYTHON_INC_DIR}"
        "${EXTRACTOR_PYTHON_API_INC_DIR}"
        "${EXTRACTOR_SRC_DIR}"
    )
    target_link_libraries(
        ${ARG_NAME}
        PUBLIC
        fmc++
        ytp
        gtest
        PkgConfig::PYTHON
    )
    add_test(NAME extractor_${ARG_NAME} COMMAND ${ARG_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
endmacro()

extr_unit_test(
    NAME
    time64_serial
)
extr_unit_test(
    NAME
    arg_stack
    FILES
    "${EXTRACTOR_SRC_DIR}/arg_stack.cpp"
)
extr_unit_test(
    NAME
    arg_serial
    FILES
    "${EXTRACTOR_SRC_DIR}/arg_stack.cpp"
    "${EXTRACTOR_SRC_DIR}/arg_serial.cpp"
    "${EXTRACTOR_SRC_DIR}/type_sys.cpp"
    "${EXTRACTOR_SRC_DIR}/type_space.cpp"
    "${EXTRACTOR_SRC_DIR}/type_error.cpp"
    "${EXTRACTOR_SRC_DIR}/type_decl.cpp"
)
extr_unit_test(
    NAME
    call_stack
    FILES
    "${EXTRACTOR_SRC_DIR}/call_stack.cpp"
)
extr_unit_test(
    NAME
    comp_graph
    FILES
    "${EXTRACTOR_SRC_DIR}/comp_graph.cpp"
)
extr_unit_test(
    NAME
    exec_ctx
    FILES
    "${EXTRACTOR_SRC_DIR}/call_obj.cpp"
    "${EXTRACTOR_SRC_DIR}/comp_graph.cpp"
    "${EXTRACTOR_SRC_DIR}/call_stack.cpp"
    "${EXTRACTOR_SRC_DIR}/stream_ctx.cpp"
    "${EXTRACTOR_SRC_DIR}/exec_ctx.cpp"
)
extr_unit_test(
    NAME
    comp_sys
    FILES
    "${EXTRACTOR_SRC_DIR}/api.cpp"
    "${EXTRACTOR_SRC_DIR}/arg_stack.cpp"
    "${EXTRACTOR_SRC_DIR}/arg_serial.cpp"
    "${EXTRACTOR_SRC_DIR}/comp.cpp"
    "${EXTRACTOR_SRC_DIR}/call_obj.cpp"
    "${EXTRACTOR_SRC_DIR}/comp_graph.cpp"
    "${EXTRACTOR_SRC_DIR}/call_stack.cpp"
    "${EXTRACTOR_SRC_DIR}/stream_ctx.cpp"
    "${EXTRACTOR_SRC_DIR}/comp_sys.cpp"
    "${EXTRACTOR_SRC_DIR}/comp_def_simp.cpp"
    "${EXTRACTOR_SRC_DIR}/comp_def.cpp"
    "${EXTRACTOR_SRC_DIR}/type_sys.cpp"
    "${EXTRACTOR_SRC_DIR}/type_space.cpp"
    "${EXTRACTOR_SRC_DIR}/type_error.cpp"
    "${EXTRACTOR_SRC_DIR}/type_decl.cpp"
    "${EXTRACTOR_SRC_DIR}/exec_ctx.cpp"
)
extr_unit_test(
    NAME
    type_sys
    FILES
    "${EXTRACTOR_SRC_DIR}/type_sys.cpp"
    "${EXTRACTOR_SRC_DIR}/type_space.cpp"
    "${EXTRACTOR_SRC_DIR}/type_error.cpp"
    "${EXTRACTOR_SRC_DIR}/type_decl.cpp"
)
extr_unit_test(
    NAME
    tests
    FILES
    $<TARGET_OBJECTS:extractor_obj>
    $<TARGET_OBJECTS:extractor_book_lib_obj>
    $<TARGET_OBJECTS:py_extractor_obj>
)

find_program(PYTHON3_BIN "python3" REQUIRED)

macro(extr_py_unit_test)
    add_test(
        NAME extractor_py_${ARGV0} COMMAND
    
        "${PYTHON3_BIN}"
        "${CMAKE_CURRENT_SOURCE_DIR}/${ARGV0}.py"
    
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(
        extractor_py_${ARGV0}
        PROPERTIES
        ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_yamal_BUILD_DIR}/build/lib:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
        TIMEOUT 100
    )
endmacro()

extr_py_unit_test(
    perf_ident
)
extr_py_unit_test(
    bars
)
extr_py_unit_test(
    decbars
)
extr_py_unit_test(
    btec_bars
)
extr_py_unit_test(
    btec_bars128
)
extr_py_unit_test(
    csv_play
)
extr_py_unit_test(
    csv_play_wrong
)
extr_py_unit_test(
    csv_play_empty_strings
)
extr_py_unit_test(
    csv_play_empty_fail
)
extr_py_unit_test(
    csv_play_pipe
)
extr_py_unit_test(
    csv_play_pipe_fail
)
extr_py_unit_test(
    csv_play_pipe_empty_fail
)
extr_py_unit_test(
    csv_play_string_fail
)
extr_py_unit_test(
    csv_tail
)
extr_py_unit_test(
    csv_tail_pipe
)
extr_py_unit_test(
    vwap
)
extr_py_unit_test(
    vwap_reverse
)
extr_py_unit_test(
    proto_vwap
)
extr_py_unit_test(
    lag
)
extr_py_unit_test(
    csv_mkt_data
)
extr_py_unit_test(
    mp_mkt_data_play
)
extr_py_unit_test(
    mp_mkt_data_play_pipe
)
extr_py_unit_test(
    mp_mkt_data_play_pipe_fail
)
extr_py_unit_test(
    mp_mkt_data_perf
)
extr_py_unit_test(
    mp_mkt_data_record
)
extr_py_unit_test(
    csv_mkt_data_perf
)
extr_py_unit_test(
    convert
)
extr_py_unit_test(
    add
)
extr_py_unit_test(
    mult
)
extr_py_unit_test(
    ln
)
extr_py_unit_test(
    log
)
extr_py_unit_test(
    exp
)
extr_py_unit_test(
    pow
)
extr_py_unit_test(
    max_min
)
extr_py_unit_test(
    count
)
extr_py_unit_test(
    nano
)
extr_py_unit_test(
    result
)
extr_py_unit_test(
    fields
)
extr_py_unit_test(
    average_tw
)
extr_py_unit_test(
    nan
)
extr_py_unit_test(
    zero
)
extr_py_unit_test(
    is_zero
)
extr_py_unit_test(
    is_nan
)
extr_py_unit_test(
    is_inf
)
extr_py_unit_test(
    mp_skip_data
)
extr_py_unit_test(
    op_tests
)
extr_py_unit_test(
    sum
)
# extr_py_unit_test(
#     book_build
# )
extr_py_unit_test(
    book_msg
)
extr_py_unit_test(
    book_update
)
extr_py_unit_test(
    rational64
)
extr_py_unit_test(
    perf_timer
)
extr_py_unit_test(
    divide
)
extr_py_unit_test(
    op_overload
)
extr_py_unit_test(
    module
)
extr_py_unit_test(
    split_by
)
extr_py_unit_test(
    split_by_bars
)
extr_py_unit_test(
    pandas_play
)
extr_py_unit_test(
    sim_poll
)
extr_py_unit_test(
    tuple_split
)
extr_py_unit_test(
    callbacks
)
extr_py_unit_test(
    round
)
extr_py_unit_test(
    custom
)
extr_py_unit_test(
    substr
)
extr_py_unit_test(
    unicode
)
extr_py_unit_test(
    frame
)
extr_py_unit_test(
    join
)
extr_py_unit_test(
    py_book
)
extr_py_unit_test(
    book_montage
)
extr_py_unit_test(
    book_montage128
)
extr_py_unit_test(
    top_book_update
)
extr_py_unit_test(
    api_test
)
extr_py_unit_test(
    accumulate
)
extr_py_unit_test(
    live_batch
)
extr_py_unit_test(
    datetime64
)
extr_py_unit_test(
    join_growing_frames
)
extr_py_unit_test(
    frame_iters
)
extr_py_unit_test(
    find
)
extr_py_unit_test(
    time_lag
)
extr_py_unit_test(
    book_play_split
)
extr_py_unit_test(
    ytp_sequence
)
extr_py_unit_test(
    timeout
)
extr_py_unit_test(
    heartbeat
)
extr_py_unit_test(
    delayed
)
extr_py_unit_test(
    book_stats
)
extr_py_unit_test(
    ore_ytp_decode
)
extr_py_unit_test(
    coinbase
)
extr_py_unit_test(
    numerical
)
extr_py_unit_test(
    error_reporting
)
extr_py_unit_test(
    ore_split
)

add_library(
    ext_lib
    SHARED
    "ext_lib.cpp"
)
target_include_directories(
    ext_lib
    PUBLIC
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_API_INC_DIR}"
)
target_link_libraries(
    ext_lib
    PUBLIC
    extractor
    ytp
)
set_target_properties(
    ext_lib
    PROPERTIES
    OUTPUT_NAME "ext_lib"
    PREFIX ""
    VERSION "1.0.0"
    SOVERSION "0"
)

add_executable(
    ext_exe
    "ext_exe.cpp"
)
target_include_directories(
    ext_exe
    PUBLIC
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_API_INC_DIR}"
)
target_link_libraries(
    ext_exe
    PUBLIC
    extractor
    ytp
    gtest
)
add_test(NAME extractor_ext_lib COMMAND ext_exe)
set_tests_properties(
    extractor_ext_lib
    PROPERTIES
    ENVIRONMENT "EXTRACTORPATH=${CMAKE_CURRENT_BINARY_DIR}"
    TIMEOUT 100
)

add_executable(
    split_sample_trade
    "split_sample_trade.cpp"
)
target_include_directories(
    split_sample_trade
    PUBLIC
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_API_INC_DIR}"
)
target_link_libraries(
    split_sample_trade
    PUBLIC
    extractor
    ytp
    gtest
)

add_test(
    NAME extractor_python_ext_test COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext_test.py"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ext_test
    PROPERTIES
    ENVIRONMENT "EXTRACTORPATH=$<TARGET_FILE_DIR:ext_lib>;PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_yamal_BUILD_DIR}/build/lib:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)


add_test(
    NAME extractor_python_ema_test COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/ema_test.py"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ema_test
    PROPERTIES
    ENVIRONMENT "EXTRACTORPATH=$<TARGET_FILE_DIR:ema_ext>;PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_yamal_BUILD_DIR}/build/lib:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)

add_test(
    NAME extractor_python_ytp_book_bbo COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/book_bbo.py"
    "--ytp"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo.ytp"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ytp_book_bbo
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_yamal_BUILD_DIR}/build/lib:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)


add_test(
    NAME extractor_python_ytp_book_trade COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/book_bbo.py"
    "--ytp"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_trades.ytp"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ytp_book_trade
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_yamal_BUILD_DIR}/build/lib:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)

add_test(
    NAME extractor_python_trade_view_sim COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/trade_view.py"
    "--ytp-file"
    "${CMAKE_CURRENT_BINARY_DIR}/consolidated.ytp"
    "--base"
    "${CMAKE_CURRENT_BINARY_DIR}/trade_view.{}.csv"
    "--mode"
    "sim"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_trade_view_sim
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_yamal_BUILD_DIR}/build/lib:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)

add_test(
    NAME extractor_python_trade_view_live COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/trade_view.py"
    "--ytp-file"
    "${CMAKE_CURRENT_BINARY_DIR}/consolidated.ytp"
    "--base"
    "${CMAKE_CURRENT_BINARY_DIR}/trade_view.{}.csv"
    "--mode"
    "live"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_trade_view_live
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_yamal_BUILD_DIR}/build/lib:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)

add_test(
    NAME extractor_python_ytp_book_bbo_trade_multi_file COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/book_bbo_multi_file.py"
    "--ytp"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo_trades.ytp"
    "--markets"
    "NYSE,NASDAQOMX,BATS,DirectEdgeX,DirectEdgeA,NYSEArca,NASDAQOMXBX,BATSY"
    "--imnts"
    "APLE,APAM,BMY,BAC"
    "--testcount"
    "291"
    "--testname"
    "book_bbo_trades_multi_file"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ytp_book_bbo_trade_multi_file
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_yamal_BUILD_DIR}/build/lib:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)

add_test(
    NAME extractor_python_ytp_book_bbo_trade_multi_file_partial COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/book_bbo_multi_file.py"
    "--ytp"
    "${CMAKE_CURRENT_BINARY_DIR}/ytpseq_ore_bbo_trades_partial.ytp"
    "--markets"
    "NYSE,NASDAQOMX,BATS,DirectEdgeX,DirectEdgeA,NYSEArca,NASDAQOMXBX,BATSY"
    "--imnts"
    "APLE,APAM,BMY,BAC"
    "--testcount"
    "215"
    "--testname"
    "book_bbo_trades_multi_file_partial"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ytp_book_bbo_trade_multi_file_partial
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_yamal_BUILD_DIR}/build/lib:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)
