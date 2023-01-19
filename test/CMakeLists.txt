find_program(PYTHON3_BIN "python3" REQUIRED)
find_package(PkgConfig)
pkg_check_modules(GTEST REQUIRED IMPORTED_TARGET gtest)
pkg_check_modules(PYTHON IMPORTED_TARGET python3-embed)
if (NOT PYTHON_FOUND)
    pkg_check_modules(PYTHON REQUIRED IMPORTED_TARGET python3)
endif (NOT PYTHON_FOUND)

macro(extr_unit_test)
    set(oneValueArgs NAME)
    set(multiValueArgs "FILES;ENVIRONMENT")
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
        "${EXTRACTOR_PYTHON_INC_DIR}"
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

add_custom_command(
    OUTPUT
    "${CMAKE_CURRENT_BINARY_DIR}/coinbase_feed.ytp"

    DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/data/coinbase_feed.txt.xz"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/coinbase_feed_comp.txt.xz"

    COMMAND
    ${CMAKE_COMMAND} -E remove -f "${CMAKE_CURRENT_BINARY_DIR}/coinbase_feed.ytp"

    COMMAND
    "${PYTHON3_BIN}" "${PROJECT_SOURCE_DIR}/tools/ore_ytp_write.py"
    "--ytp" "${CMAKE_CURRENT_BINARY_DIR}/coinbase_feed.ytp"
    "--channel" "ore/imnts/coinbase/BTC-USD"
    "--peer" "feed_handler"
    "--ore" "${CMAKE_CURRENT_SOURCE_DIR}/data/coinbase_feed.txt.xz"
    "--decompress"

    COMMAND
    "${PYTHON3_BIN}" "${PROJECT_SOURCE_DIR}/tools/ore_ytp_write.py"
    "--ytp" "${CMAKE_CURRENT_BINARY_DIR}/coinbase_feed.ytp"
    "--channel" "ore_comp/imnts/coinbase/BTC-USD"
    "--peer" "feed_handler"
    "--ore" "${CMAKE_CURRENT_SOURCE_DIR}/data/coinbase_feed_comp.txt.xz"
    "--decompress"
)
add_custom_target(
    extractor_coinbasefeed ALL
    DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/coinbase_feed.ytp"
)

macro(extr_py_unit_test)
    set(oneValueArgs "NAME;SCRIPT")
    set(multiValueArgs "ENVIRONMENT")
    cmake_parse_arguments(ARG
                          ""
                          "${oneValueArgs}"
                          "${multiValueArgs}"
                          ${ARGN}
    )
    if (NOT ARG_SCRIPT)
        set(ARG_SCRIPT "${ARG_NAME}.py")
    endif()
    add_test(
        NAME extractor_py_${ARG_NAME} COMMAND
    
        "${PYTHON3_BIN}"
        "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SCRIPT}"
    
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
    string(
        JOIN
        ";"
        ENV
        "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH}"
        "PYTHONUNBUFFERED=1"
        "PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
        "ENABLE_COUNTERS=1"
        ${ARG_ENVIRONMENT}
    )
    set_tests_properties(
        extractor_py_${ARG_NAME}
        PROPERTIES
        ENVIRONMENT "${ENV}"
        TIMEOUT 100
    )
endmacro()

extr_py_unit_test(
    NAME perf_ident
)
extr_py_unit_test(
    NAME bars
)
extr_py_unit_test(
    NAME decbars
)
extr_py_unit_test(
    NAME btec_bars
)
extr_py_unit_test(
    NAME btec_bars128
)
extr_py_unit_test(
    NAME csv_play
)
extr_py_unit_test(
    NAME csv_play_wrong
)
extr_py_unit_test(
    NAME csv_play_empty_strings
)
extr_py_unit_test(
    NAME csv_play_empty_fail
)
extr_py_unit_test(
    NAME csv_play_pipe
)
extr_py_unit_test(
    NAME csv_play_pipe_fail
)
extr_py_unit_test(
    NAME csv_play_pipe_empty_fail
)
extr_py_unit_test(
    NAME csv_tail
)
extr_py_unit_test(
    NAME csv_tail_pipe
)
extr_py_unit_test(
    NAME vwap
)
extr_py_unit_test(
    NAME vwap_reverse
)
extr_py_unit_test(
    NAME proto_vwap
)
extr_py_unit_test(
    NAME lag
)
extr_py_unit_test(
    NAME csv_mkt_data
)
extr_py_unit_test(
    NAME mp_mkt_data_play
)
extr_py_unit_test(
    NAME mp_mkt_data_play_pipe
)
extr_py_unit_test(
    NAME mp_mkt_data_play_pipe_fail
)
extr_py_unit_test(
    NAME mp_mkt_data_perf
)
extr_py_unit_test(
    NAME mp_mkt_data_record
)
extr_py_unit_test(
    NAME csv_mkt_data_perf
)
extr_py_unit_test(
    NAME convert
)
extr_py_unit_test(
    NAME add
)
extr_py_unit_test(
    NAME mult
)
extr_py_unit_test(
    NAME ln
)
extr_py_unit_test(
    NAME log
)
extr_py_unit_test(
    NAME exp
)
extr_py_unit_test(
    NAME pow
)
extr_py_unit_test(
    NAME max_min
)
extr_py_unit_test(
    NAME count
)
extr_py_unit_test(
    NAME nano
)
extr_py_unit_test(
    NAME result
)
extr_py_unit_test(
    NAME fields
)
extr_py_unit_test(
    NAME average_tw
)
extr_py_unit_test(
    NAME nan
)
extr_py_unit_test(
    NAME zero
)
extr_py_unit_test(
    NAME is_zero
)
extr_py_unit_test(
    NAME is_nan
)
extr_py_unit_test(
    NAME is_inf
)
extr_py_unit_test(
    NAME mp_skip_data
)
extr_py_unit_test(
    NAME op_tests
)
extr_py_unit_test(
    NAME sum
)
# extr_py_unit_test(
#     book_build
# )
extr_py_unit_test(
    NAME book_msg
)
extr_py_unit_test(
    NAME book_update
)
extr_py_unit_test(
    NAME rational64
)
extr_py_unit_test(
    NAME perf_timer
)
extr_py_unit_test(
    NAME divide
)
extr_py_unit_test(
    NAME op_overload
)
extr_py_unit_test(
    NAME module
)
extr_py_unit_test(
    NAME split_by
)
extr_py_unit_test(
    NAME split_by_bars
)
extr_py_unit_test(
    NAME pandas_play
)
extr_py_unit_test(
    NAME sim_poll
)
extr_py_unit_test(
    NAME tuple_split
)
extr_py_unit_test(
    NAME callbacks
)
extr_py_unit_test(
    NAME round
)
extr_py_unit_test(
    NAME custom
)
extr_py_unit_test(
    NAME substr
)
extr_py_unit_test(
    NAME unicode
)
extr_py_unit_test(
    NAME frame
)
extr_py_unit_test(
    NAME join
)
extr_py_unit_test(
    NAME py_book
)
extr_py_unit_test(
    NAME book_montage
)
extr_py_unit_test(
    NAME book_montage128
)
extr_py_unit_test(
    NAME top_book_update
)
extr_py_unit_test(
    NAME api_test
)
extr_py_unit_test(
    NAME accumulate
)
extr_py_unit_test(
    NAME live_batch
)
extr_py_unit_test(
    NAME datetime64
)
extr_py_unit_test(
    NAME join_growing_frames
)
extr_py_unit_test(
    NAME frame_iters
)
extr_py_unit_test(
    NAME find
)
extr_py_unit_test(
    NAME time_lag
)
# extr_py_unit_test(
#     ore_sim_split
# )
extr_py_unit_test(
    NAME book_play_split
)
extr_py_unit_test(
    NAME announcements_book_build
)
extr_py_unit_test(
    NAME ytp_sequence
)
extr_py_unit_test(
    NAME timeout
)
extr_py_unit_test(
    NAME heartbeat
)
extr_py_unit_test(
    NAME delayed
)
extr_py_unit_test(
    NAME book_stats
)
extr_py_unit_test(
    NAME ore_ytp_decode
)
extr_py_unit_test(
    NAME coinbase_decimal
    SCRIPT coinbase.py
    ENVIRONMENT
    "COINBASE_FEED_YTP=${CMAKE_CURRENT_BINARY_DIR}/coinbase_feed.ytp"
    "COINBASE_FEED_CH=ore/imnts/coinbase/BTC-USD"
    "BOOK_BASE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/coinbase_book.base.csv"
    "BOOK_TEST_FILE=${CMAKE_CURRENT_BINARY_DIR}/coinbase_book.test.csv"
)
extr_py_unit_test(
    NAME coinbase_compatibility
    SCRIPT coinbase.py
    ENVIRONMENT
    "COINBASE_FEED_YTP=${CMAKE_CURRENT_BINARY_DIR}/coinbase_feed.ytp"
    "COINBASE_FEED_CH=ore_comp/imnts/coinbase/BTC-USD"
    "BOOK_BASE_FILE=${CMAKE_CURRENT_SOURCE_DIR}/data/coinbase_book_comp.base.csv"
    "BOOK_TEST_FILE=${CMAKE_CURRENT_BINARY_DIR}/coinbase_book_comp.test.csv"
)
extr_py_unit_test(
    NAME numerical
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
add_test(NAME extractor_ext_lib COMMAND ext_exe ${CMAKE_CURRENT_SOURCE_DIR} $<TARGET_FILE:ext_lib>)

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
    NAME extractor_python_ext_test COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/ext_test.py"
    $<TARGET_FILE:ext_lib>

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ext_test
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)


add_test(
    NAME extractor_python_ema_test COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/ema_test.py"
    $<TARGET_FILE:ema_ext>

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ema_test
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)

add_test(
    NAME extractor_python_ytp_book_bbo COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/book_bbo.py"
    "--ytp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ytpseq_ore_bbo.ytp"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ytp_book_bbo
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)


add_test(
    NAME extractor_python_ytp_book_trade COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/book_bbo.py"
    "--ytp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ytpseq_ore_trades.ytp"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ytp_book_trade
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)

add_test(
    NAME extractor_python_ytp_book_bbo_trade_multi_file COMMAND

    "${PYTHON3_BIN}"
    "${CMAKE_CURRENT_SOURCE_DIR}/book_bbo_multi_file.py"
    "--ytp"
    "${CMAKE_CURRENT_SOURCE_DIR}/data/ytpseq_ore_bbo_trades.ytp"
    "--markets"
    "NYSE,NASDAQOMX,BATS,DirectEdgeX,DirectEdgeA,NYSEArca,NASDAQOMXBX,BATSY"
    "--imnts"
    "APLE,APAM,BMY,BAC"
    "--test"
    "true"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
set_tests_properties(
    extractor_python_ytp_book_bbo_trade_multi_file
    PROPERTIES
    ENVIRONMENT "PYTHONPATH=${PROJECT_SOURCE_DIR}/tools:${WHEEL_extractor_BUILD_DIR}/build/lib:$ENV{PYTHONPATH};PYTHONUNBUFFERED=1;PATH=${WHEEL_extractor_BUILD_DIR}/build/scripts:$ENV{PATH}"
    TIMEOUT 100
)
