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
@package datetime64.py
@author Andrus Suvalov
@date 11 May 2020
@brief File contains datetime64 tests
"""

from datetime import datetime, timedelta
import extractor as extr
import pytz
import pandas as pd
import numpy as np
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    ts = pd.to_datetime([31513, 854684, 1536868], unit='s')
    ts = ts.tz_localize('UTC')
    ts = ts.tz_convert('US/Pacific')

    us = pd.Series(["message0", "message1", "message3"], dtype='unicode')
    df = pd.DataFrame(data={"Timestamp": ts, "unicode_col": us})

    ddf = df.set_index("Timestamp")

    data = op.pandas_play(ddf, (("unicode_col", extr.Array(extr.Char, 30)),))

    accum = op.accumulate(data)

    graph.stream_ctx().run()

    accum_ref = graph.get_ref(accum)
    ts = ts.tz_convert(None)
    df = pd.DataFrame(data={"Timestamp": ts, "unicode_col": us})
    pd.testing.assert_frame_equal(accum_ref.as_pandas(), df)
    np.testing.assert_equal(accum_ref[0].unicode_col, us[0])
    np.testing.assert_equal(accum_ref[1].unicode_col, us[1])
    np.testing.assert_equal(accum_ref[2].unicode_col, us[2])

    ts1 = pd.to_datetime(1536868, unit='s', utc=True)
    time64 = extr.Time64(ts1)
    ts2 = pd.to_datetime(time64.as_timedelta().total_seconds(), unit='s', utc=True)
    assert ts1 == ts2

    ts1 = ts1.tz_convert('US/Pacific')
    time64 = extr.Time64(ts1)
    ts2 = pd.to_datetime(time64.as_timedelta().total_seconds(), unit='s', utc=True)
    ts2 = ts2.tz_convert('US/Pacific')
    assert ts1 == ts2

    graph = extr.system.comp_graph()
    op = graph.features

    op.constant(("pandas", extr.Time64, ts1))
    op.constant(("timedelta", extr.Time64, timedelta(seconds=1)))
    try:
        op.constant(("timedelta", extr.Time64, "invalid type"))
        assert False
    except RuntimeError:
        pass
