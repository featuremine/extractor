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
@package csv_play.py
@author Andres Rangel
@date 8 Jul 2018
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import filecmp
import sys
import extractor as extr
import time
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    data_in = graph.features.csv_play(str("cat " + src_dir + "/data/csv_play_file.csv |"),
                                      (("timestamp", extr.Time64, ""),
                                       ("val1", extr.Int32, ""),
                                          ("val2", extr.Uint16, ""),
                                          ("unicode", extr.Array(extr.Char, 30), "")), name="csv_play_0")
    data_out = graph.features.csv_record(data_in,
                                         src_dir + "/data/csv_play_pipe_py.test.csv", name="csv_record_0")
    data_out_two = graph.features.csv_record(
        data_in,
        str("| cat > " + src_dir + "/data/csv_play_pipe_py.pipe_test.csv"),
        name="csv_record_1")

    ctx = graph.stream_ctx()
    now = ctx.next_time()
    while True:
        ctx.proc_one(now)
        now = ctx.next_time()

        if now == extr.Time64.end_time():
            break

    extr.flush()
    time.sleep(1)
    extr.flush()

    extr.assert_base(src_dir + '/data/csv_play_file_py.base.csv',
                     src_dir + '/data/csv_play_pipe_py.test.csv')
    extr.assert_base(src_dir + '/data/csv_play_file_py.base.csv',
                     src_dir + '/data/csv_play_pipe_py.pipe_test.csv')
