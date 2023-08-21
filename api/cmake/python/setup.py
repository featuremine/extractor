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
import os
import setuptools


if sys.version_info < (3, 6, 0):
    print('Tried to install with an unsupported version of Python. ' +
          os.getenv('PACKAGE_NAME') +
          ' requires Python 3.6.0 or greater')
    sys.exit(1)


scripts = []
scripts_joined = os.getenv('SCRIPTS')
if len(scripts_joined) > 0:
    for p in scripts_joined.split(':'):
        scripts.append(p)
for root, dirs, files in os.walk('scripts'):
    for name in files:
        scripts.append(os.path.join('scripts', name))


packages = []
packages_joined = os.getenv('PACKAGES')
if len(packages_joined) > 0:
    for p in packages_joined.split(':'):
        packages.append(p)
        packages.append(p + '.*')


install_requires = []
install_requires_joined = os.getenv('INSTALL_REQUIRES')
if len(install_requires_joined) > 0:
    for p in install_requires_joined.split(':'):
        install_requires.append(p)


setuptools.setup(
    name = os.getenv('PACKAGE_NAME'),
    version = os.getenv('PACKAGE_VERSION'),
    author='Featuremine Corporation',
    author_email='support@featuremine.com',
    url='https://www.featuremine.com',
    description=os.getenv('PACKAGE_DESCRIPTION'),
    long_description=os.getenv('PACKAGE_LONG_DESCRIPTION'),
    classifiers=[
        'Programming Language :: Python :: 3 :: Only',
    ],
    license='COPYRIGHT (c) 2022 by Featuremine Corporation',
    scripts=scripts,
    packages=setuptools.find_packages(include=tuple(packages)),
    install_requires=install_requires,
)
