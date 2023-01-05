"""
        COPYRIGHT (c) 2020 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.
"""

import unittest
import os

class TestExtractorComponent(unittest.TestCase):

    def test_load_module_success(self):
        import extractor as extr
        assert extr is not None
        assert extr.__version__ is not None

if __name__ == '__main__':
    unittest.main()
