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

import subprocess
import os
import shutil
import struct
import sys


def build_library(libname, src_dir, tmp_base_dir, build_dir, debug=False):
    tmp_dir = os.path.join(tmp_base_dir, "build{}".format(8 * struct.calcsize("P")))

    # Clean the tmp build directory every time
    shutil.rmtree(tmp_dir, ignore_errors=True)
    os.mkdir(tmp_dir)

    cxxflags = os.environ.get("CXXFLAGS", None)
    ldflags = os.environ.get("LDFLAGS", None)
    os.environ["CXXFLAGS"] = cxxflags or ""
    os.environ["LDFLAGS"] = ldflags or ""

    # Enable ninja for compilation if available
    build_type = []
    if shutil.which("ninja"):
        build_type = ["-G", "Ninja"]

    # Determine python shell architecture for Windows
    python_arch = 8 * struct.calcsize("P")
    build_arch = []
    build_flags = []
    if os.name == "nt":
        if python_arch == 64:
            build_arch_flag = "x64"
        elif python_arch == 32:
            build_arch_flag = "Win32"
        else:
            raise RuntimeError("Unknown windows build arch")
        build_arch = ["-A", build_arch_flag]
        build_flags = ["--config", "Release"]
        print("Building {} for {}".format(libname, python_arch))

    if not shutil.which("cmake"):
        raise RuntimeError("Could not find cmake in your path!")

    args = [
        "cmake",
        "-DCMAKE_POSITION_INDEPENDENT_CODE=1",
        "-DBUILD_TESTING=OFF",
        "-DBUILD_TOOLS=OFF",
        "-DBUILD_API_DOCS=OFF",
        "-DCMAKE_BUILD_TYPE=Debug" if debug else "-DCMAKE_BUILD_TYPE=Release",
        "-DBUILD_SHARED_LIBS=OFF",
        "-DCMAKE_INSTALL_PREFIX:PATH={}".format(build_dir),
        src_dir,
    ]
    args.extend(build_type)
    args.extend(build_arch)
    conf = subprocess.Popen(args, cwd=tmp_dir, stdout=sys.stdout)
    returncode = conf.wait()
    if returncode != 0:
        raise RuntimeError("{} CMake configure failed {}".format(libname, returncode))

    # Run build through cmake
    args = [
        "cmake",
        "--build",
        ".",
        "--target",
        "install",
    ]
    args.extend(build_flags)
    build = subprocess.Popen(args, cwd=tmp_dir, stdout=sys.stdout)
    returncode = build.wait()
    if cxxflags is None:
        del os.environ["CXXFLAGS"]
    else:
        os.environ["CXXFLAGS"] = cxxflags
    if ldflags is None:
        del os.environ["LDFLAGS"]
    else:
        os.environ["LDFLAGS"] = ldflags
    if returncode != 0:
        raise RuntimeError("{} compilation failed: {}".format(libname, returncode))
