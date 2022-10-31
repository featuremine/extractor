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
@package time_lag.py
@author Federico Ravchina
@date 17 Mar 2021
@brief File contains tests for time_lag
"""

from datetime import datetime, timedelta
import extractor as extr
from extractor import result_as_pandas
import pandas as pd
import numpy as np
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    ts1 = pd.to_datetime([10, 20, 30], unit='ms')
    val1 = pd.Series([101, 102, 103], dtype='int64')
    df1 = pd.DataFrame(data={"val1": val1,
                             "Timestamp": ts1}).set_index("Timestamp")

    ts2 = pd.to_datetime([15, 25, 35, 35], unit='ms')
    val2 = pd.Series([201, 202, 203, 204], dtype='int64')
    df2 = pd.DataFrame(data={"val1": val2,
                             "Timestamp": ts2}).set_index("Timestamp")

    data_in_one = op.pandas_play(
        df1,
        (("val1", extr.Int64),))

    data_in_two = op.pandas_play(
        df2,
        (("val1", extr.Int64),))

    delayed_in_two = op.time_lag(data_in_two, timedelta(milliseconds=40), timedelta(milliseconds=10))

    join = op.join(data_in_one, delayed_in_two, "name", extr.Array(extr.Char, 16), ("one", "two"))

    one = op.accumulate(join)

    graph.stream_ctx().run()

    as_pd_one = result_as_pandas(one)

    np.testing.assert_array_equal(as_pd_one['Timestamp'],
                                  np.array([10, 20, 30, 15 + 40, 25 + 40, 35 + 40], dtype=np.dtype('datetime64[ms]')))

    np.testing.assert_array_equal(as_pd_one['val1'],
                                  np.array([101, 102, 103, 201, 202, 203]))

    graph = extr.system.comp_graph()
    op = graph.features

    ts3 = pd.to_datetime([10, 20, 30, 30, 30, 30, 30, 30, 30, 30], unit='ms')
    val3 = pd.Series([101, 102, 103, 104, 105, 106, 107, 108, 109, 110], dtype='int64')
    df3 = pd.DataFrame(data={"val1": val3,
                             "Timestamp": ts3}).set_index("Timestamp")

    data_in_three = op.pandas_play(
        df3,
        (("val1", extr.Int64),))

    delayed_in_three = op.time_lag(data_in_three, timedelta(milliseconds=100), timedelta(0))
    delayed_in_four = op.time_lag(data_in_three, timedelta(milliseconds=1), timedelta(milliseconds=1))

    two = op.accumulate(delayed_in_three)
    three = op.accumulate(delayed_in_four)

    graph.stream_ctx().run()

    as_pd_two = result_as_pandas(two)
    as_pd_three = result_as_pandas(three)

    np.testing.assert_array_equal(as_pd_two['Timestamp'], np.array(
        [(x + 100) for x in [10, 20, 30, 30, 30, 30, 30, 30, 30, 30]], dtype=np.dtype('datetime64[ms]')))

    np.testing.assert_array_equal(as_pd_two['val1'],
                                  np.array([101, 102, 103, 104, 105, 106, 107, 108, 109, 110]))

    np.testing.assert_array_equal(as_pd_three['Timestamp'],
                                  np.array([(x + 1) for x in [10, 20, 30]], dtype=np.dtype('datetime64[ms]')))

    np.testing.assert_array_equal(as_pd_three['val1'],
                                  np.array([101, 102, 103]))
