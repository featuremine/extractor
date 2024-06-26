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
from os import path

with open(path.join(path.dirname(__file__), '..', 'VERSION')) as f:
    extractor_version = list(f)[0].strip()

if sys.version_info < (3, 6, 0):
    print('Tried to install with an unsupported version of Python. '
          'extractor requires Python 3.6.0 or greater')
    sys.exit(1)

import setuptools

setuptools.setup (
    name = 'featuremine-extractor',
    version = extractor_version,
    author='Featuremine Corporation',
    author_email='support@featuremine.com',
    url='https://www.featuremine.com',
    description='Featuremine Extractor Package',
    long_description='Featuremine Extractor Package',
    classifiers=[
        'Programming Language :: Python :: 3 :: Only',
    ],
    package_data={
        'extractor': ['extractor.so', 'libextractor*', 'include', 'include/*', 'include/*/*', 'include/*/*/*']
    },
    license='COPYRIGHT (c) 2022 by Featuremine Corporation',
    packages=['extractor', 'extractor.tests', 'extractor.tools', 'extractor.tools.visualization'],
    scripts=['scripts/test-extractor-python'],
    install_requires=[
        'yamal',
        'numpy~=1.19',
        'pandas',
        'graphviz'
    ]
)
