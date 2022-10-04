extr_incdirs = [numpy_incdir, extractor_srcdir, extractor_incdir]

if is_linux
    shared_deps_st = [static_libstdcxx_dep] + rlm_dep
    shared_deps_sh = [shared_ytp_dep, fmc_dep, dl, m_dep]
elif is_macos
    shared_deps_st = [static_libstdcxx_dep]
    shared_deps_sh = [shared_ytp_dep, fmc_dep]
elif is_windows
    shared_deps_st = []
    shared_deps_sh = [shared_ytp_dep, fmc_dep]
endif

if fmtron
    shared_deps_sh += fmtron_dep
endif

shared = shared_library('extractor', [extractor_src],
                      include_directories : [extr_incdirs],
                      dependencies : shared_deps_sh + shared_deps_st,
                      cpp_args : static_cpp_args + ['-DBUILDING_DLL'],
                      link_args: static_link_args + rpathargs,
                      link_language: 'c',
                      install: true,
                      build_rpath: '$ORIGIN',
                      install_rpath: '$ORIGIN',
                      install_dir: 'lib')

shared_dep = declare_dependency(link_with: shared,
                                include_directories : [extractor_incdir],
                                dependencies: [shared_deps_sh],
                                version : meson.project_version())

py_shared_extr_dependencies = [py3_dep, shared_dep] + shared_deps_sh

if fmtron
    py_shared_extr_dependencies += fmtron_dep
endif

pyshared = shared_library('pyextractor', [py_extractor_src],
                      include_directories : [extr_incdirs],
                      dependencies : [py3_dep, shared_dep, pyshared_ytp_dep] + shared_deps_st,
                      cpp_args : static_cpp_args + ['-DPY_EXTR_VER="'+meson.project_version()+'"', '-DBUILDING_DLL'],
                      link_args: static_link_args + rpathargs,
                      link_language: 'c',
                      install: true,
                      build_rpath: '$ORIGIN',
                      install_rpath: '$ORIGIN',
                      install_dir: 'lib')

pyshared_dep = declare_dependency(link_with: pyshared,
                                  include_directories : [extractor_incdir],
                                  dependencies: [py3_dep, shared_dep],
                                  version : meson.project_version())


extractor = python3.extension_module('extractor', [py_extractor_main_src],
                                     dependencies : [pyshared_dep],
                                     cpp_args : static_cpp_args + ['-DBUILDING_DLL'],
                                     link_args: static_link_args + rpathargs,
                                     link_language: 'c')

extractor_pymod_path = meson.current_build_dir()

py_srcs = files([
   '__init__.py'
])

tools_bins = files(
  '../tools/book_ete.py',
  '../tools/bpipe_poll.py'
)

hadron = import('hadron')
extractor_pkg = hadron.package(
    'extractor',
    version: meson.project_version(),
    suffix: '',
    py_sources: py_srcs,
    extensions: [extractor, pyshared, shared, shared_fmc, shared_ytp, pyshared_ytp],
    #bin_files: tools_bins,
    install: qsemble or exos or ep or trial,
    verify: false,
    documentation: false,
    style: false,
    python: python3)
