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
@package csv_play_pipe_empty_fail.py
@author Andres Rangel
@date 8 Jul 2018
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import filecmp
import sys
import extractor as extr
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    extr.set_license(src_dir + "/test.lic")
    graph = extr.system.comp_graph()
    data_in = graph.features.csv_play(str("cat " + src_dir + "/data/csv_play_file_empty_fail.csv |"),
                                      (("timestamp", extr.Time64, ""),
                                       ("val1", extr.Int32, ""),
                                          ("val2", extr.Uint16, "")), name="csv_play_0")

    try:
        graph.stream_ctx().run()
    except RuntimeError:
        exit(0)

    exit(1)
