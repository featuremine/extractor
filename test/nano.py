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
@package convert.py
@author Maxim Trokhimtchouk
@date 22 Apr 2018
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
import pytz
import pandas as pd
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()
    op = graph.features

    in_file_one = os.path.join(src_dir, "data/arithmetical_op_file_one.csv")
    out_file_one = os.path.join(src_dir, "data/nano.test.csv")

    data_in_one = op.csv_play(
        in_file_one,
        (("timestamp", extr.Time64, ""),
         ("val1", extr.Float64, ""),
         ("val2", extr.Int64, "")))

    val_one = op.field(data_in_one, "timestamp")

    aggr_val_one = op.accumulate(val_one)

    aggr_val_one = graph.get_ref(aggr_val_one)

    out_stream_one = op.nano(val_one)

    reverted_val_one = op.nano(out_stream_one)

    aggr_reverted_val_one = op.accumulate(reverted_val_one)

    aggr_reverted_val_one = graph.get_ref(aggr_reverted_val_one)

    op.csv_record(out_stream_one, out_file_one)

    val_composed = op.fields(data_in_one, ("timestamp", "val2"))
    aggr_val_composed = op.accumulate(val_composed)
    aggr_val_composed = graph.get_ref(aggr_val_composed)

    nanos_val_composed = op.nano(val_composed)
    nanos_val_reverted = op.nano(nanos_val_composed)

    nanos_val_reverted = op.accumulate(nanos_val_reverted)
    nanos_val_reverted = graph.get_ref(nanos_val_reverted)

    graph.stream_ctx().run()

    pd.testing.assert_frame_equal(aggr_val_one.as_pandas(), aggr_reverted_val_one.as_pandas())
    pd.testing.assert_frame_equal(aggr_val_composed.as_pandas(), nanos_val_reverted.as_pandas())

    extr.flush()

    extr.assert_base(os.path.join(src_dir, 'data/nano.base.csv'), os.path.join(src_dir, 'data/nano.test.csv'))
