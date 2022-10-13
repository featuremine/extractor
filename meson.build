project('extractor', 'cpp', 'c', version:'6.6.2')

is_windows = build_machine.system() == 'windows'
is_arm = build_machine.cpu_family() == 'aarch64'
is_macos = build_machine.system() == 'darwin'
is_linux = build_machine.system() == 'linux'
is_unix = is_linux or is_macos

if not is_windows
  add_project_arguments('-Wno-unused-variable', language : 'cpp')
  add_project_arguments('-Wno-deprecated-declarations', language : 'cpp')
  # Commented out since this option is not available in gcc 7.2.1
  # add_project_arguments('-Wclass-memaccess', language : 'cpp')
endif

if is_windows
  add_project_arguments('/std:c++17', language : 'cpp')
  add_project_arguments('-D_WIN32_WINNT=0x0A00', language : 'cpp')
  add_project_arguments('-D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING', language : 'cpp')
elif is_linux
  add_project_arguments('-std=c++17', language : 'cpp')
  add_project_arguments('-fconcepts', language : 'cpp')
  add_project_link_arguments('-static-libstdc++', language : 'cpp')
  add_project_link_arguments('-lstdc++fs', language : ['cpp', 'c'])
  rpathargs = ['-Wl,-rpath=$ORIGIN']
elif is_macos
  add_project_arguments('-std=c++17', language : 'cpp')
  rpathargs = ['-Wl,-rpath,@loader_path', '-Wl,-rpath,@loader_path/featuremine']
endif

is_debug = false

if get_option('buildtype').startswith('debug')
  add_project_arguments('-DNBUILD', language : 'cpp')
  is_debug = true
endif

extr_incdir = include_directories('include')
is_subproject = meson.is_subproject()

cpp_compiler = meson.get_compiler('cpp')
c_compiler = meson.get_compiler('c')

if is_macos
  static_libstdcxx = cpp_compiler.find_library('c++')
  static_link_args = ['-fvisibility-inlines-hidden',
                      '-fvisibility=hidden']
elif is_linux
  add_project_link_arguments('-lstdc++fs', language : ['cpp', 'c'])
  static_libstdcxx = [cpp_compiler.find_library('stdc++', static: true), cpp_compiler.find_library('m', required : true)]
  static_link_args = ['-fvisibility-inlines-hidden',
                      '-fvisibility=hidden',
                      '-Wl,--exclude-libs=ALL']
elif is_windows
  static_link_args = []
  static_libstdcxx = []
  static_libstdcxx_dep = declare_dependency(dependencies : static_libstdcxx)
endif

if is_unix
  static_libstdcxx_dep = declare_dependency(dependencies : static_libstdcxx)
  static_cpp_args = ['-fvisibility=hidden',
                    '-fvisibility-inlines-hidden']
elif is_windows
  static_cpp_args = []
endif


pymod = import('python')
if is_subproject
  py_version = get_option('py_version')
  python3 = pymod.find_installation(py_version)
else
  python3 = pymod.find_installation()
endif

if (python3.language_version().split('.')[1].to_int() < 6)
  error('support python version 3.6 and higher')
endif

pp = run_command(python3, '-c', 'import sys;print(sys.executable)')
if pp.returncode() != 0
  error('failed command')
endif
python_path = pp.stdout().strip()

py3_dep = python3.dependency(embed:true)

#numpy
pp = run_command(python3, '-c', 'import numpy;print(numpy.get_include())')
if pp.returncode() != 0
  error('failed command to obtain numpy include directory')
endif
numpy_incdir = pp.stdout().strip()

clients = get_option('clients')
qsemble = clients.contains('qsemble')
exos = clients.contains('exos')
ep = clients.contains('exoduspoint')
schonfeld = clients.contains('schonfeld')
trial = clients.contains('trial')
install_samples = get_option('install_samples')

extensions_env = get_option('extensions')
fmtron = extensions_env.contains('fmtron')
if fmtron
  fmtron_sp = subproject('fmtron-lib')
  #fmtron_ema_cfg = fmtron_sp.get_variable('fmtron_ema_cfg')
  fmtron_dep = declare_dependency(dependencies: fmtron_sp.get_variable('fmtron_dep'))
  add_project_arguments('-DFMTRON', language : 'cpp')
  fmtron_path = fmtron_sp.get_variable('lib_fmtron_abs')
endif

common_sp = subproject('common')
fmc_dep = common_sp.get_variable('fmc_dep')
shared_fmc_dep = common_sp.get_variable('shared_fmc_dep')
shared_fmc = common_sp.get_variable('shared_fmc')
static_fmcpp_dep = common_sp.get_variable('shared_fmc_dep')
static_fmcpp = common_sp.get_variable('static_fmcpp')

rlm_sp = subproject('rlm')
rlm_dep = rlm_sp.get_variable('rlm_dep')
rlm_path = rlm_sp.get_variable('rlm_path')

if is_unix
  thread_dep = dependency('threads')
elif is_windows
  thread = []
  thread_dep = declare_dependency(dependencies : thread)
endif

yamal_sp = subproject('yamal') # Subproject import syntax
shared_ytp_dep = yamal_sp.get_variable('shared_dep') # Dependency to use with your targets
ytp_pymod_path = yamal_sp.get_variable('ytp_pymod_path')
shared_ytp = yamal_sp.get_variable('shared') # Dependency to use with your targets
pyshared_ytp = yamal_sp.get_variable('pyshared')
pyshared_ytp_dep = yamal_sp.get_variable('pyshared_dep')

asio_sp = subproject('asio') # subproject import syntax
asio_dep = declare_dependency(compile_args:['-DASIO_STANDALONE',
                                            '-Wno-unused-local-typedefs'],
                              dependencies: asio_sp.get_variable('asio_dep')) # dependency declaration, we should move this to the asio repo at  some point

if is_linux
  dw = cpp_compiler.find_library('dw', required : true)
  unwind = cpp_compiler.find_library('unwind', required : true)
  if is_arm
    unwind_lib = cpp_compiler.find_library('unwind-aarch64', required : true)
  else
    unwind_lib = cpp_compiler.find_library('unwind-x86_64', required : true)
  endif
  dl = cpp_compiler.find_library('dl', required : true)
  m_dep = c_compiler.find_library('m', required : false)

  deps = [py3_dep, shared_ytp_dep, thread_dep, dw, unwind, unwind_lib, m_dep]
elif is_macos
  deps = [py3_dep, shared_ytp_dep, thread_dep]
elif is_windows
  deps = [py3_dep, shared_ytp_dep, thread_dep]
endif

if is_windows
  base = 'C:/vcpkg/installed/x64-windows'
  if is_debug
    vcpkg_dir = base + '/debug'
    gtest_lib = 'gtestd'
  else
    vcpkg_dir = base
    gtest_lib = 'gtest'
  endif

  gtest_lib = meson.get_compiler('cpp').find_library(gtest_lib, dirs: vcpkg_dir+'/lib', required: true)
  gtest_inc = include_directories( base+'/include')
  gtest = declare_dependency(dependencies : [gtest_lib],
    include_directories : gtest_inc)
else
  gtest = dependency('gtest')
endif

extr_incdir = include_directories('include')
extractor_srcdir = include_directories('.')

if is_windows
  path_separator = ';'
else
  path_separator = ':'
endif


subdir('scripts')
subdir('include')
subdir('python')
if not is_windows
  subdir('src')
  subdir('lib')
  subdir('tools')
  subdir('test')
endif
subdir('samples')

if not is_subproject and not is_windows
  run_target('doc', command : [doc, meson.project_version(), meson.source_root()])
  run_target('style', command : style)
endif

if not is_subproject and not is_windows
  install_subdir('doc', install_dir : '.')
  install_data('README', install_dir : '.')
  install_data('LICENSE', install_dir : '.')
  if is_unix
    meson.add_install_script('scripts/setup.sh', meson.project_name(), meson.project_version())
  endif
  run_target('package',command:pkg_cmd)
endif
