src_dir = meson.current_source_dir()

py_tests = ['book_ete', 'json2ore', 'ore_reader']

foreach test : py_tests
  test('python_' + test , python3,
    args : [src_dir + '/' + test + '.py'],
    env : ['PYTHONPATH='+ extractor_pymod_path + path_separator + ytp_pymod_path], timeout : 600, is_parallel : false)
endforeach

test('book_cross_validate_BTC_USD' , python3,
      args : [src_dir + '/book_cross_validate.py',
              '--ore', src_dir + '/../test/data/coinbase_BTC-USD.ore',
              '--imnts', 'BTC-USD'],
      env : ['PYTHONPATH='+ extractor_pymod_path + path_separator + ytp_pymod_path],
      timeout : 600, is_parallel : false)


test('book_cross_validate' , python3,
      args : [src_dir +'/book_cross_validate.py',
              '--imnts', '2_YEAR', '3_YEAR', '10_YEAR',
              '--ore', src_dir + '/../test/data/book.base.ore'],
      env : ['PYTHONPATH='+ extractor_pymod_path + path_separator + ytp_pymod_path],
      timeout : 600, is_parallel : false)


if install_samples
    install_data('book_ete.py', install_dir : 'tools/extractor')
    install_data('json2ore.py', install_dir : 'tools/extractor')
    install_data('ore_reader.py', install_dir : 'tools/extractor')
    install_data('ore.avsc', install_dir : 'tools/extractor')
    install_data('pg_play.py', install_dir : 'tools/extractor')
    install_data('pg_pub.py', install_dir : 'tools/extractor')
    install_data('kafka_play.py', install_dir : 'tools/extractor')
    install_data('kafka_poll.py', install_dir : 'tools/extractor')
    install_data('kafka_pub.py', install_dir : 'tools/extractor')
    install_data('bars.py', install_dir: 'tools/extractor')
    install_data('bpipe_poll.py', install_dir: 'tools/extractor')
    install_data('fmtron.py', install_dir: 'tools/extractor')
    install_data('book_build.py', install_dir: 'tools/extractor')
    install_data('book_cross_validate.py', install_dir: 'tools/extractor')
    install_data('yamal_ore_record.py', install_dir : 'tools/extractor')
    install_data('ore_yamal_play.py', install_dir : 'tools/extractor')
    install_data('live_book_build.py', install_dir : 'tools/extractor')
    install_data('ore_ytp_decode.py', install_dir : 'tools/extractor')
    install_data('ore_ytp_dump.py', install_dir : 'tools/extractor')
endif
