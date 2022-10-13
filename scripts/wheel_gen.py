import hashlib
import base64
import os
from sys import argv
import errno
from shutil import copyfile
from zipfile import ZipFile, ZipInfo
import shutil
from argparse import ArgumentParser


def hashes(file_names):
    return [
        base64.urlsafe_b64encode(hashlib.sha256(open(file_name, 'rb').read()).digest()).decode('latin1').rstrip('=')
        for file_name in file_names
    ]


def sizes(file_names):
    return [
        os.stat(file_name).st_size
        for file_name in file_names
    ]


def record_gen(dist_info, module, file_names):
    with open("%sRECORD" % (dist_info), 'w') as f:
        for file, sha, sz in zip(file_names, hashes(file_names), sizes(file_names)):
            f.write("%s,sha256=%s,%d\n" % (os.path.basename(file), sha, sz))
        f.write("%s.dist-info/RECORD,,\n" % (module))


def metadata_gen(dist_info, module, version):
    with open("%sMETADATA" % (dist_info), 'w') as f:
        f.write("Metadata-Version: 2.1\n")
        f.write("Name: %s\n" % (module))
        f.write("Version: %s\n" % (version))
        f.write("Summary: %s extension\n" % (module))
        f.write("Home-page: http://www.featuremine.com\n")
        f.write("Maintainer: Featuremine Corporation\n")
        f.write("Maintainer-email: support@featuremine.com\n")
        f.write("License: UNKNOWN\n")
        f.write("Platform: UNKNOWN\n\n")
        f.write("%s extension.\n" % (module))


def wheel_gen(dist_info, major_ver, minor_ver):
    with open("%sWHEEL" % (dist_info), 'w') as f:
        v = "%s%s" % (major_ver, minor_ver)
        f.write("Wheel-Version: 1.0\n")
        f.write("Generator: bdist_wheel (0.33.1)\n")
        f.write("Root-Is-Purelib: false\n")
        if int(major_ver) >= 3 and int(minor_ver) >= 8:
            f.write("Tag: cp%s-cp%s-linux_x86_64\n" % (v, v))
        else:
            f.write("Tag: cp%s-cp%sm-linux_x86_64\n" % (v, v))


def top_level_gen(dist_info, module):
    with open("%stop_level.txt" % (dist_info), 'w') as f:
        f.write("\n%s\n\n" % (module))


def resolve_target_path(file, base_path, module, bin_files=False, root_files=False):
    if bin_files or root_files:
        return "%s" % (file[file.rfind("/"):].lstrip("/"))
    else:
        pos = file.rfind(base_path)
        if pos < 0:
            return "%s/%s" % (module, file[file.rfind("/"):].lstrip("/"))
        else:
            return "%s" % (file[pos + len(base_path):].lstrip("/"))


def files_setup(base_path, module, file_names, bin_files=False, root_files=False):
    for file in file_names:
        t_path = resolve_target_path(file, base_path, module, bin_files, root_files)
        pos = t_path.rfind("/")
        if pos >= 0:
            os.makedirs(t_path[:pos], exist_ok=True)
        copyfile(file, t_path)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument('--module')
    parser.add_argument('--version')
    parser.add_argument('--base_path')
    parser.add_argument('--major_ver')
    parser.add_argument('--minor_ver')
    parser.add_argument('--files', nargs='*', default=[])
    parser.add_argument('--root_files', nargs='*', default=[])
    parser.add_argument('--bin_files', nargs='*', default=[])
    args = parser.parse_args()
    module = args.module
    version = args.version
    base_path = args.base_path
    major_ver = args.major_ver
    minor_ver = args.minor_ver
    files = args.files
    root_files = args.root_files
    bin_files = args.bin_files

    dist_info = "%s/%s/%s-%s.dist-info/" % (base_path, module, module, version)

    try:
        os.makedirs(os.path.dirname(dist_info))
    except OSError as exc:
        if exc.errno != errno.EEXIST:
            raise

    os.chdir("%s/%s" % (base_path, module))

    wheel_gen(dist_info, major_ver, minor_ver)
    top_level_gen(dist_info, module)
    metadata_gen(dist_info, module, version)
    record_gen(dist_info, module, files + root_files + bin_files)
    files_setup(base_path, module, files)
    files_setup(base_path, module, root_files, root_files=True)
    files_setup(base_path, module, bin_files, bin_files=True)

    if int(major_ver) >= 3 and int(minor_ver) >= 8:
        zipname = '%s-%s-cp%s%s-cp%s%s-linux_x86_64.whl' % (args.module,
                                                            args.version,
                                                            major_ver,
                                                            minor_ver,
                                                            major_ver,
                                                            minor_ver)
    else:
        zipname = '%s-%s-cp%s%s-cp%s%sm-linux_x86_64.whl' % (args.module,
                                                             args.version,
                                                             major_ver,
                                                             minor_ver,
                                                             major_ver,
                                                             minor_ver)

    with ZipFile(zipname, 'w') as zip:
        for file in files:
            zip.write(resolve_target_path(file, base_path, module))
        for file in root_files:
            zip.write(resolve_target_path(file, base_path, module, root_files=True))
        for file in bin_files:
            zip.write(resolve_target_path(file, base_path, module, bin_files=True))
        zip.write("%s-%s.dist-info/WHEEL" % (module, version))
        zip.write("%s-%s.dist-info/top_level.txt" % (module, version))
        zip.write("%s-%s.dist-info/METADATA" % (module, version))
        zip.write("%s-%s.dist-info/RECORD" % (module, version))

    os.chdir("../")

    shutil.move('%s/%s' % (module, zipname), zipname)
