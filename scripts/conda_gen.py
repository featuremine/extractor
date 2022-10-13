import hashlib
import base64
import os
from sys import argv
import errno
from shutil import copyfile
import zipfile
import tarfile
import shutil
import json
import time
import numpy
import pandas
import os
from argparse import ArgumentParser


def hashes(file_names):
    return [
        hashlib.sha256(open(file_name, 'rb').read()).hexdigest()
        for file_name in file_names
    ]


def sizes(file_names):
    return [
        os.stat(file_name).st_size
        for file_name in file_names
    ]


def gen_libpy_path(module, file, python_ver):
    return "lib/python%s%s" % (python_ver, file[file.rfind("/" + module + "/"):])


def is_script(file):
    return file.rfind("/scripts/") != -1


def gen_bin_path(file):
    s = "/scripts/"
    return "bin/%s" % (file[file.rfind(s) + len(s):])


def paths_gen(info_dir, module, file_names, python_ver):
    with open("%spaths.json" % (info_dir), 'w') as f:
        paths = []
        for file, sha, sz in zip(file_names, hashes(file_names), sizes(file_names)):
            path = {
                "_path": gen_bin_path(file) if is_script(file) else gen_libpy_path(module, file, python_ver),
                "path_type": "hardlink",
                "sha256": sha,
                "size_in_bytes": sz
            }
            paths.append(path)
        json.dump({"paths": paths, "paths_version": 1}, f, indent=2, separators=(',', ': '))


def index_gen(info_dir, module, version, build, python_ver):
    index = {
        "arch": "x86_64",
        "build": build,
        "build_number": 0,
        "depends": [
            "python >=%s" % (python_ver),
            "numpy >=%s" % (numpy.__version__),
            "pandas >=%s" % (pandas.__version__)
        ],
        "license": "MIT",
        "license_family": "MIT",
        "name": module,
        "platform": "linux",
        "subdir": "linux-64",
        "timestamp": round(time.time()),
        "version": version
    }
    with open("%sindex.json" % (info_dir), 'w') as f:
        json.dump(index, f, indent=2, separators=(',', ': '))


def files_gen(info_dir, module, file_names, python_ver, mode):
    with open("%sfiles" % (info_dir), mode) as f:
        for file in file_names:
            f.write("%s%s" % (gen_bin_path(file) if is_script(file) else gen_libpy_path(module, file, python_ver), "\n"))


def copy_files(dest, module, file_names):
    for file in file_names:
        os.makedirs(dest + file[file.rfind("/" + module + "/"):-len(os.path.basename(file))], exist_ok=True)
        copyfile(file, dest + file[file.rfind("/" + module + "/") + 1:])


def copy_bin(dest, file_names):
    s = "/scripts/"
    for file in file_names:
        i = file.rfind(s) + len(s)
        os.makedirs(dest + file[i:-len(os.path.basename(file))], exist_ok=True)
        copyfile(file, dest + file[i:])


def preproc_paths(files):
    ret = []
    scripts = []
    for file in files:
        if os.path.isdir(file):
            for sub_file in os.listdir(file):
                full_sub_file_name = file + '/' + sub_file
                if os.path.isdir(full_sub_file_name):
                    new_ret, new_scripts = preproc_paths([full_sub_file_name])
                    ret += new_ret
                    scripts += new_scripts
                elif is_script(full_sub_file_name):
                    scripts += [full_sub_file_name]
                else:
                    ret += [full_sub_file_name]
        elif is_script(file):
            scripts += [file]
        else:
            ret += [file]
    return ret, scripts


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument('--distro_name')
    parser.add_argument('--distro_ver')
    parser.add_argument('--module')
    parser.add_argument('--version')
    parser.add_argument('--base_path')
    parser.add_argument('--major_ver')
    parser.add_argument('--minor_ver')
    parser.add_argument('--files', nargs='*', default=[])
    parser.add_argument('--root_files', nargs='*', default=[])
    parser.add_argument('--bin_files', nargs='*', default=[])
    args = parser.parse_args()
    distro_name = args.distro_name
    distro_ver = args.distro_ver
    module = args.module
    version = args.version
    base_path = args.base_path
    major_ver = args.major_ver
    minor_ver = args.minor_ver
    files = args.files
    root_files = args.root_files
    bin_files = args.bin_files

    python_ver = "%s.%s" % (major_ver, minor_ver)
    info_dir = "%s/%s/info/" % (base_path, module)
    lib_dir = "%s/%s/lib/python%s/" % (base_path, module, python_ver)
    bin_dir = "%s/%s/bin/" % (base_path, module)

    try:
        os.makedirs(os.path.dirname(info_dir))
        os.makedirs(os.path.dirname(lib_dir))
        os.makedirs(os.path.dirname(bin_dir))
    except OSError as exc:
        if exc.errno != errno.EEXIST:
            raise

    os.chdir("%s/%s" % (base_path, module))

    build = "%s_%s_py%s%s" % (distro_name, distro_ver, major_ver, minor_ver)

    files, scripts = preproc_paths(files)

    index_gen(info_dir, module, version, build, python_ver)
    paths_gen(info_dir, module, files + scripts, python_ver)
    files_gen(info_dir, module, files, python_ver, 'w')
    files_gen(info_dir, module, scripts, python_ver, 'a')
    copy_files(lib_dir, module, files)
    copy_bin(bin_dir, scripts)

    zipname = '%s-%s-%s.tar.bz2' % (module, version, build)

    with tarfile.open(zipname, 'w:bz2') as tar:
        for file in files + scripts:
            tar.add(gen_bin_path(file) if is_script(file) else gen_libpy_path(module, file, python_ver))
        tar.add("info/index.json")
        tar.add("info/paths.json")
        tar.add("info/files")

    os.chdir("../")

    shutil.move('%s/%s' % (module, zipname), zipname)

    shutil.rmtree(module)
