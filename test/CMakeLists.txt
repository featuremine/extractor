find_package(PkgConfig)
pkg_check_modules(GTEST REQUIRED IMPORTED_TARGET gtest)
pkg_check_modules(PYTHON IMPORTED_TARGET python3-embed)
if (NOT PYTHON_FOUND)
    pkg_check_modules(PYTHON REQUIRED IMPORTED_TARGET python3)
endif (NOT PYTHON_FOUND)

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
        "${EXTRACTOR_SRC_DIR}"
    )
    target_link_libraries(
        ${ARG_NAME}
        PUBLIC
        fmc++
        ytp
        PkgConfig::GTEST
        PkgConfig::PYTHON
    )
    add_test(NAME ${ARG_NAME} COMMAND ${ARG_NAME} ${CMAKE_CURRENT_SOURCE_DIR})
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
    $<TARGET_OBJECTS:py_extractor_obj>
)

find_program(PYTHON3_BIN "python3" REQUIRED)

macro(extr_py_unit_test)
    add_test(
        NAME py_${ARGV0} COMMAND
    
        "${PYTHON3_BIN}"
        "${CMAKE_CURRENT_SOURCE_DIR}/${ARGV0}.py"
    
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    set_tests_properties(
        py_${ARGV0}
        PROPERTIES
        ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}/tools:/${CMAKE_BINARY_DIR}/python_dist/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${CMAKE_BINARY_DIR}/python_dist/build/${PYTHON3_SCRIPTS_DIR}:$ENV{PATH}"
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
extr_py_unit_test(
    book_build
)
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
    ore_sim_split
)
extr_py_unit_test(
    book_play_split
)
extr_py_unit_test(
    announcements_book_build
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

add_library(
    ema_ext
    SHARED
    "ema_ext.c"
)
target_include_directories(
    ema_ext
    PUBLIC
    "${EXTRACTOR_INC_DIR}"
)
target_link_libraries(
    ema_ext
    PUBLIC
    extractor
    ytp
)
set_target_properties(
    ema_ext
    PROPERTIES
    OUTPUT_NAME "ema"
    VERSION "1.0.0"
    SOVERSION "0"
)


add_library(
    book_play_ext
    SHARED
    "book_play_ext.c"
    "../src/book_play_split.cpp"
)
target_include_directories(
    book_play_ext
    PUBLIC
    "${EXTRACTOR_INC_DIR}"
    "${EXTRACTOR_SRC_DIR}"
)
target_link_libraries(
    book_play_ext
    PUBLIC
    extractor
    ytp
)
set_target_properties(
    book_play_ext
    PROPERTIES
    OUTPUT_NAME "book_play"
    VERSION "1.0.0"
    SOVERSION "0"
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
)
target_link_libraries(
    ext_exe
    PUBLIC
    extractor
    ytp
    PkgConfig::GTEST
)
add_test(NAME ext_lib COMMAND ext_exe ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:ext_lib>)

add_executable(
    split_sample_trade
    "split_sample_trade.cpp"
)
target_include_directories(
    split_sample_trade
    PUBLIC
    "${EXTRACTOR_INC_DIR}"
)
target_link_libraries(
    split_sample_trade
    PUBLIC
    extractor
    ytp
    PkgConfig::GTEST
)

add_test(
    NAME python_ext_test COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext_test.py"
    $<TARGET_FILE:ext_lib>

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    python_ext_test
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}/tools:/${CMAKE_BINARY_DIR}/python_dist/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${CMAKE_BINARY_DIR}/python_dist/build/${PYTHON3_SCRIPTS_DIR}:$ENV{PATH}"
    TIMEOUT 100
)


add_test(
    NAME python_ema_test COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/ema_test.py"
    $<TARGET_FILE:ema_ext>

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    python_ema_test
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}/tools:/${CMAKE_BINARY_DIR}/python_dist/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${CMAKE_BINARY_DIR}/python_dist/build/${PYTHON3_SCRIPTS_DIR}:$ENV{PATH}"
    TIMEOUT 100
)

add_test(
    NAME python_ytp_book_bbo COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/book_bbo.py"
    "--ytp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ytpseq_ore_bbo.ytp"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    python_ytp_book_bbo
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}/tools:/${CMAKE_BINARY_DIR}/python_dist/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${CMAKE_BINARY_DIR}/python_dist/build/${PYTHON3_SCRIPTS_DIR}:$ENV{PATH}"
    TIMEOUT 100
)


add_test(
    NAME python_ytp_book_trade COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/book_bbo.py"
    "--ytp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ytpseq_ore_trades.ytp"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    python_ytp_book_trade
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${CMAKE_SOURCE_DIR}/tools:/${CMAKE_BINARY_DIR}/python_dist/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${CMAKE_BINARY_DIR}/python_dist/build/${PYTHON3_SCRIPTS_DIR}:$ENV{PATH}"
    TIMEOUT 100
)
