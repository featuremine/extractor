"""
       COPYRIGHT (c) 2017 by Featuremine Corporation.
       This software has been provided pursuant to a License Agreement
       containing restrictions on its use.  This software contains
       valuable trade secrets and proprietary information of
       Featuremine Corporation and is protected by law.  It may not be
       copied or distributed in any form or medium, disclosed to third
       parties, reverse engineered or used in any manner not provided
       for in said License Agreement except with the prior written
       authorization from Featuremine Corporation.
"""

"""
 @file /test/csv_play_wrong.py
 @author Vitaut Tryputsin
 @date 20 Jan 2020
"""


from datetime import datetime, timedelta
import filecmp
import sys
import extractor as extr
from numpy.testing import assert_equal
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    data_in = graph.features.csv_play(src_dir + "/data/csv_play_file_wrong.base.csv",
                                      (("timestamp", extr.Time64, ""),
                                       ("val1", extr.Int32, ""),
                                          ("val2", extr.Uint16, "")), name="csv_play_0")
    try:
        ctx = graph.stream_ctx()
        raise RuntimeError("no errors were found")
    except Exception as e:
        e_str = str(e)
        res = e_str.find('unable to parse value in row 1 in column 5 with the name timestamp')
        assert_equal(False, res is -1)
