src_dir = meson.current_source_dir()

test_license_path = join_paths(src_dir, 'test.lic')

py_tests = ['perf_ident',
            'bars',
            'btec_bars',
            'csv_play',
            'csv_play_wrong',
            'csv_play_empty_strings',
            'csv_play_empty_fail',
            'csv_play_pipe',
            'csv_play_pipe_fail',
            'csv_play_pipe_empty_fail',
            'csv_tail',
            'csv_tail_pipe',
            'vwap',
            'vwap_reverse',
            'proto_vwap',
            'lag',
            'csv_mkt_data',
            'mp_mkt_data_play',
            'mp_mkt_data_play_pipe',
            'mp_mkt_data_play_pipe_fail',
            'mp_mkt_data_perf',
            'mp_mkt_data_record',
            'csv_mkt_data_perf',
            'convert',
            'add',
            'mult',
            'ln',
            'log',
            'exp',
            'pow',
            'max_min',
            'count',
            'nano',
            'result',
            'fields',
            'average_tw',
            'nan',
            'zero',
            'is_zero',
            'is_nan',
            'is_inf',
            'mp_skip_data',
            'op_tests',
            'sum',
            'book_build',
            'book_msg',
            'book_update',
            'rational64',
            'perf_timer',
            'divide',
            'op_overload',
            'module',
            'split_by',
            'split_by_bars',
            'pandas_play',
            'sim_poll',
            'tuple_split',
            'callbacks',
            'round',
            'custom',
            'substr',
            'unicode',
            'frame',
            'join',
            'py_book',
            'book_montage',
            'top_book_update',
            'api_test',
            'accumulate',
            'live_batch',
            'datetime64',
            'join_growing_frames',
            'frame_iters',
            'find',
            'time_lag',
            'ore_sim_split',
            'book_play_split',
            'announcements_book_build',
            'ytp_sequence',
            'timeout',
            'heartbeat',
            'delayed',
            'book_stats',
            'ore_ytp_decode',
            'coinbase']

if is_linux
    extr_tests_dep = [py3_dep, shared_ytp_dep, pyshared_ytp_dep, gtest, dl] + rlm_dep
elif is_macos
    extr_tests_dep = [py3_dep, shared_ytp_dep, pyshared_ytp_dep, gtest]
elif is_windows
    extr_tests_dep = [py3_dep, shared_ytp_dep, pyshared_ytp_dep, gtest]
endif

if fmtron
    extr_tests_dep += fmtron_dep
endif

ema_ext = shared_library('ema', 'ema_ext.c',
                        dependencies : [shared_dep],
                        version : '1.0.0',
                        soversion : '0',
                        install : false,
                        link_language : 'c')

book_play_ext = shared_library('book_play', ['book_play_ext.c', '../src/book_play_split.cpp'],
                        include_directories : extractor_srcdir,
                        dependencies : [static_libstdcxx_dep, shared_dep],
                        version : '1.0.0',
                        soversion : '0',
                        install : false,
                        link_language : 'c')

ext_lib = shared_library('ext_lib', 'ext_lib.cpp',
                          dependencies : [static_libstdcxx_dep, shared_dep],
                          version : '1.0.0',
                          soversion : '0',
                          install : false)

ext_exe = executable(
    'ext_exe', 'ext_exe.cpp',
    dependencies : [gtest, shared_dep])
test('ext_lib', ext_exe, args: [src_dir, ext_lib.full_path()], is_parallel : false)

split_sample_trade = executable(
    'split_sample_trade', 'split_sample_trade.cpp',
    dependencies : [gtest, shared_dep])

if fmtron
    fmemaconfig_version = configuration_data()
    fmemaconfig_version.set_quoted('RDMFieldDictionary', src_dir + '/RDMFieldDictionary')
    fmemaconfig_version.set_quoted('enumtype', src_dir + '/enumtype.def')

    fmtron_ema_cfg = configure_file(
      input : 'FmEmaConfig.xml.in',
      output : 'FmEmaConfig.xml',
      configuration : fmemaconfig_version,
    )
    fmtron_tests = ['fmtron_sys',
                    'fmtron_op',
                    'fmtron_several_clients']
    foreach test : fmtron_tests
        test('python_' + test, python3,
            args : [src_dir + '/' + test + '.py', ext_lib.full_path(), fmtron_ema_cfg ],
            env : [
                'PYTHONPATH='+ extractor_pymod_path + path_separator + ytp_pymod_path,
                'LD_LIBRARY_PATH='+meson.build_root() + '/src/' + ':' + fmtron_path + 'test'],
            is_parallel : false,
            timeout : 180)
    endforeach
endif

test('python_ext_test' , python3,
    args : [src_dir + '/ext_test.py', ext_lib.full_path()],
    env : ['PYTHONPATH='+ extractor_pymod_path + path_separator + ytp_pymod_path], timeout : 600)

test('python_ema_test' , python3,
    args : [src_dir + '/ema_test.py', ema_ext.full_path()],
    env : ['PYTHONPATH='+ extractor_pymod_path + path_separator + ytp_pymod_path], timeout : 600)
    
test('python_ytp_book_bbo' , python3,
    args : [src_dir + '/book_bbo.py', 
            '--license', test_license_path,
            '--ytp', join_paths(src_dir, 'data', 'ytpseq_ore_bbo.ytp')],
    env : ['PYTHONPATH='+ extractor_pymod_path + path_separator + ytp_pymod_path], timeout : 100,
    is_parallel : false)
    
test('python_ytp_book_trade' , python3,
    args : [src_dir + '/book_bbo.py', 
            '--license', test_license_path,
            '--ytp', join_paths(src_dir, 'data', 'ytpseq_ore_trades.ytp')],
    env : ['PYTHONPATH='+ extractor_pymod_path + path_separator + ytp_pymod_path], timeout : 100,
    is_parallel : false)

foreach test : extractor_tests
    name = test[0]
    files = test[1]
    main = name + '.cpp'
    exe = executable(
        name, files, main,
        include_directories : extr_incdirs,
        dependencies : extr_tests_dep,
        cpp_args : ['-DPY_EXTR_VER="'+meson.project_version()+'"', '-DNO_DLL_DECORATOR'])
    test(name, exe, args: [src_dir], timeout : 180)
endforeach

tool_pymod_path = meson.current_source_dir() + '/../tools'
warning(tool_pymod_path)

foreach test : py_tests
    test('python_' + test , python3,
        args : [src_dir + '/' + test + '.py'],
        env : ['PYTHONPATH='+ extractor_pymod_path + path_separator + ytp_pymod_path + path_separator + tool_pymod_path],
        timeout : 600)
endforeach

if install_samples
    foreach test : py_tests
        install_data(test + '.py', install_dir : 'test/extractor')
    endforeach

    install_data('book_stats.py', install_dir : 'samples')
    install_data('book_montage.py', install_dir : 'samples')
    install_data('top_book_update.py', install_dir : 'samples')

    install_data('sim_poll.py', install_dir : 'samples')

    install_data('data/feature_currency.csv', install_dir : 'test/extractor/data')
    install_data('data/itch.20150901.partial.json.gz', install_dir : 'test/extractor/data')
    install_data('data/book.base.ore', install_dir : 'test/extractor/data')
    install_data('data/book_levels.python.base.csv', install_dir : 'test/extractor/data')

    install_data('bars.py', install_dir : 'samples')
    install_data('data/sip_quotes_20171018.mp', install_dir : 'test/extractor/data')
    install_data('data/sip_trades_20171018.mp', install_dir : 'test/extractor/data')
    install_data('data/bar_20171018.base.csv', install_dir : 'test/extractor/data')

    install_data('op_tests.py', install_dir : 'samples')
    install_data('data/test_input.csv', install_dir : 'test/extractor/data')
    install_data('data/averages_input.csv', install_dir : 'test/extractor/data')

    install_data('tuple_split.py', install_dir : 'samples')

    install_data('ext_lib.cpp', install_dir : 'samples')
    install_data('ext_exe.cpp', install_dir : 'samples')
    install_data('ext_test.py', install_dir : 'samples')
    install_data('ema_ext.c', install_dir : 'samples')
    install_data('ema_test.py', install_dir : 'samples')
    install_data('data/coinbase_BTC-USD.ore', install_dir : 'test/extractor/data')
    install_data('book_play_ext.c', install_dir : 'samples')

endif
