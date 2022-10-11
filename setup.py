"""
        COPYRIGHT (c) 2022 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.
"""

import sys
import subprocess
from os import path

extractor_version = list(open(path.join(path.dirname(__file__), 'VERSION')))[0].strip()

if sys.version_info < (3, 6, 0):
    print('Tried to install with an unsupported version of Python. '
          'extractor requires Python 3.6.0 or greater')
    sys.exit(1)

subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'pkgconfig'])
    
import os
import sysconfig
import struct
import setuptools
import pkgconfig
import pathlib
from setuptools.command.build_ext import build_ext
from distutils.command.build_scripts import build_scripts
from distutils.file_util import copy_file
from setuptools.command.test import test
from buildutils.build import build_library
from buildutils.fetch import fetch_library


_this_dir = os.path.dirname(__file__)
debug_mode = "--debug" in sys.argv
is_darwin = sys.platform == "darwin"

include_dirs = [os.path.join(_this_dir, 'include'), os.path.join(_this_dir, 'src'), os.path.join(_this_dir, 'python')]
extra_compile_args = [
    '-std=c++17',
    '-O3',
    '-fvisibility-inlines-hidden',
    '-fvisibility=hidden',
    '-DPY_EXTR_VER="'+extractor_version+'"',
    '-DNO_DLL_DECORATOR'
]

extra_link_args = [
    '-Wl,--exclude-libs,ALL',
    '-static-libgcc',
    '-static-libstdc++',
]

def add_library(dest, package, version=None):
    if version is not None:
        pkgver = pkgconfig.modversion(package)
        assert pkgver == version, '{}: {}, version expected: {}'.format(package, pkgver, version)
    for token in pkgconfig.cflags(package).split() + pkgconfig.libs(package, static=True).split():
        k = token[:2]
        if k == '-I':
            dest.include_dirs += [token[2:]]
        elif k == '-L':
            dest.library_dirs += [token[2:]]
        elif k == '-l':
            dest.libraries += [token[2:]]

def add_numpy(dest):
    import numpy
    dest.include_dirs += [numpy.get_include()]

def add_extractor(dest, build_temp):
    rootdir = os.path.split(os.path.abspath(os.path.split(os.path.abspath(os.path.split(os.path.abspath(build_temp))[0]))[0]))[0]
    dest.library_dirs += [os.path.join(rootdir, 'package', 'lib')]
    dest.libraries += ["extractor"]

def install_library(build_temp, install_dir, libname, reponame, version=None, force_repo=False, force_system=False):
    if force_repo:
        need_build = True
    elif force_system:
        need_build = False
    else:
        # Look for library using pkg-config (and minimum version)
        try:
            if version is None and pkgconfig.exists(libname) or pkgconfig.installed(libname, version):
                need_build = False
            else:
                need_build = True
        except EnvironmentError:
            # pkg-config not available in path
            need_build = True

    if need_build:
        print(
            '*WARNING* no {} detected or rebuild forced. '
            'Attempting to build it from source now. '
            'If you have {} installed, it may be out of date or is not being detected.'.format(libname, libname)
        )
        src_dir = os.path.join(os.path.abspath(build_temp), 'src', reponame)
        tmp_dir = os.path.join(os.path.abspath(build_temp), 'depbuild', reponame)
        if not os.path.exists(src_dir):
            pathlib.Path(src_dir).mkdir(parents=True, exist_ok=True)
        if not os.path.exists(install_dir):
            pathlib.Path(install_dir).mkdir(parents=True, exist_ok=True)
        if not os.path.exists(tmp_dir):
            pathlib.Path(tmp_dir).mkdir(parents=True, exist_ok=True)

        # Check if we've already built the library        
        try:
            pkgconfig.modversion(libname)
            print('{} already built at {}'.format(reponame, install_dir))
        except pkgconfig.PackageNotFoundError as e:
            src_dir = fetch_library(reponame, src_dir, version)
            build_library(reponame, src_dir, tmp_dir, install_dir, debug_mode)

def extra_link_args_get(compiler, extra_link_args):
    is_clang = compiler.compiler_type == 'unix' and ('clang' in compiler.compiler_cxx[0] or is_darwin)
    ret = extra_link_args
    if is_clang:
        ret = [arg for arg in ret if arg not in [
            '-static-libgcc',
            '-static-libstdc++',
        ]]

    if is_darwin:
        ret = [arg for arg in ret if arg not in [
            '-Wl,--exclude-libs,ALL',
        ]]
    return ret

class build_extractor_ext(build_ext):
    def run(self):
        self.install_dir = os.path.join(
            os.path.abspath(self.build_temp), 'build{}'.format(8 * struct.calcsize('P'))
        )

        os.environ["PKG_CONFIG_PATH"] = os.path.join(self.install_dir, 'lib64', 'pkgconfig') + ':' \
                                        + os.path.join(self.install_dir, 'lib', 'pkgconfig') + ':' \
                                        + os.getenv("PKG_CONFIG_PATH", '')
        install_library(self.build_temp, self.install_dir, 'ytp', 'yamal', 'api_additions')
        add_library(self, 'ytp')
        install_library(self.build_temp, self.install_dir, 'pyytp', 'yamal-python', 'api_additions')
        add_library(self, 'pyytp')
        add_numpy(self)
        add_extractor(self, self.build_temp)
        return super().run()

    def build_extension(self, ext):
        ext.extra_link_args = extra_link_args_get(self.compiler, ext.extra_link_args)
        # thanks to @jaimergp (https://github.com/conda-forge/staged-recipes/pull/17766)
        # issue: target has a mix of c and c++ source files
        #        gcc warns about passing -std=c++11 for c files, but clang errors out
        compile_original = self.compiler._compile

        def compile_patched(obj, src, ext, cc_args, extra_postargs, pp_opts):
            if src.lower().endswith('.c'):
                extra_postargs = [arg for arg in extra_postargs if not arg.lower().startswith('-std=')]
            return compile_original(obj, src, ext, cc_args, extra_postargs, pp_opts)
        self.compiler._compile = compile_patched


        return super().build_extension(ext)

    def copy_extensions_to_source(self):
        build_scripts = self.get_finalized_command('build_scripts')
        for module in build_scripts.extractor_modules:
            dest_filename = os.path.join('extractor', os.path.basename(module))
            copy_file(
                module, dest_filename, verbose=self.verbose,
                dry_run=self.dry_run
            )
        return super().copy_extensions_to_source()

setuptools.setup (
    name = 'extractor',
    version = extractor_version,
    author='Featuremine Corporation',
    author_email='support@featuremine.com',
    url='https://www.featuremine.com',
    description='Featuremine YTP packages',
    long_description='Featuremine YTP packages',
    classifiers=[
        'Programming Language :: Python :: 3 :: Only',
    ],
    license='COPYRIGHT (c) 2022 by Featuremine Corporation',
    ext_modules = [
        setuptools.Extension(
            'extractor.extractor',
            sources=[
                'python/py_extractor.cpp',
                'python/book/py_book.cpp',
                'python/py_extractor_main.cpp'
            ],
            include_dirs=include_dirs,
            extra_compile_args=extra_compile_args,
            extra_link_args=extra_link_args,
            language='c++',
        ),
    ],
    packages=['extractor', 'extractor.tests'],
    scripts=['scripts/test-extractor-python'],
    cmdclass = {
        'build_ext': build_extractor_ext
    },
)
