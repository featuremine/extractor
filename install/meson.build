cc = meson.get_compiler('cpp')

extr_incdir = include_directories('include/extractor')

extr_lib_path = meson.current_source_dir() + '/lib'
is_arm = build_machine.cpu_family() == 'aarch64'

common_incdir = include_directories('include')
common_dep = declare_dependency(include_directories : [common_incdir],
                                dependencies : [cc.find_library('fmc', dirs : extr_lib_path)],
                                version : meson.project_version())

ytp_incdir = include_directories('include')
ytp_dep = declare_dependency(include_directories : [ytp_incdir],
                             dependencies : [cc.find_library('ytp', dirs : extr_lib_path)],
                             version : meson.project_version())

shared_extr_dep = declare_dependency(include_directories : [extr_incdir],
                                      dependencies: [common_dep, ytp_dep, cc.find_library('extractor', dirs : extr_lib_path)],
                                      version : meson.project_version())

if not meson.is_subproject()
    add_project_arguments('-std=c++17', language : 'cpp')
    add_project_link_arguments('-lstdc++fs', language : ['cpp', 'c'])
    accumulator_sample = executable('accumulator_sample', 'samples/accumulator.cpp',
                                    dependencies : [shared_extr_dep])
    configure_file(input : 'samples/accumulate.base.csv', output : 'accumulate.base.csv', copy: true)
    configure_file(input : 'samples/pandas_play_file.csv', output : 'pandas_play_file.csv', copy: true)
endif
