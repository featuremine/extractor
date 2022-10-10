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
import sys
import os
import shutil


def fetch_library(reponame, savedir, version='main'):
    dep_dir = os.getenv('DEPENDENCIES_DIR', None)
    if dep_dir is None:
        shutil.rmtree(savedir, ignore_errors=True)
        args = [
            'git',
            'clone',
            '--depth=1',
            '--branch=' + version,
            os.getenv('GIT_URL', default='https://github.com/featuremine/{}.git').format(reponame),
            savedir
        ]
        fetch = subprocess.Popen(args, stdout=sys.stdout)
        returncode = fetch.wait()
        if returncode != 0:
            raise RuntimeError("{} git clone failed {}".format(reponame, returncode))
        return savedir
    else:
        return os.path.join(dep_dir, reponame)
