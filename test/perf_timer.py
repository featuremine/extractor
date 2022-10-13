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
@package perf_timer.py
@author Andres Rangel
@date 17 Jan 2019
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
import pytz
from math import isclose
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()
    op = graph.features

    in_file_one = os.path.join(src_dir, "data/arithmetical_op_file_one.csv")
    out_file_one = os.path.join(src_dir, "data/ln.test.csv")

    data_in_one = op.csv_play(
        in_file_one,
        (("timestamp", extr.Time64, ""),
         ("val1", extr.Float64, ""),
         ("val2", extr.Int32, "")))

    val_one = op.field(data_in_one, "val1")

    start = op.perf_timer_start(val_one, "ln")

    out_stream_one = op.ln(start)

    stop = op.perf_timer_stop(out_stream_one, "ln")

    record_start = op.perf_timer_start(stop, "record")

    record = op.csv_record(record_start, out_file_one)

    stop = op.perf_timer_stop(record, "record")

    graph.stream_ctx().run()

    if (isclose(extr.system.sample_value("ln"), 0)):
        exit(1)

    if (isclose(extr.system.sample_value("record"), 0)):
        exit(1)

    try:
        print(extr.system.sample_value("non_existing"))
        exit(1)
    except RuntimeError:
        pass

    extr.flush()

    extr.assert_numdiff(os.path.join(src_dir, 'data/ln.base.csv'), os.path.join(src_dir, 'data/ln.test.csv'))
