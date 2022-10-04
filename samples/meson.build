src_dir = meson.current_source_dir()

if install_samples
    install_data('accumulator.cpp', install_dir : 'samples')
    install_data('accumulate.base.csv', install_dir : 'samples')
    install_data('pandas_play_file.csv', install_dir : 'samples')
endif


exe = executable(
    'accumulator_sample', 'accumulator.cpp',
    dependencies : [shared_dep, gtest])
test(
    'accumulator_sample', exe,
    workdir: join_paths(meson.source_root(), src_dir + '/../test/data'),
    args : [src_dir + '/../test/test.lic'],
    is_parallel: false)
