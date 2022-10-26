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
@package split_by.py
@author Federico Ravchina
@date 22 Nov 2021
@brief File contains extractor python sample
"""

from datetime import timedelta
import extractor as extr
import os
from extractor import result_as_pandas
import numpy as np
import pandas as pd

src_dir = os.path.dirname(os.path.realpath(__file__))


def run(
    in_data_array=[],
    expected_output=[],
    with_time_lag=False,
):
    graph = extr.system.comp_graph()
    op = graph.features

    ts1_data = pd.to_datetime(
        [x[0] for x in in_data_array], unit='ms')

    val1_data = pd.Series(
        [x[1] for x in in_data_array], dtype='int64')

    val2_data = pd.Series(
        [x[2] for x in in_data_array], dtype='str')

    df = pd.DataFrame(
        data={
            "val1": val1_data,
            "val2": val2_data,
            "Timestamp": ts1_data,
        }).set_index("Timestamp")

    data_in = op.pandas_play(
        df,
        (
            ("val1", extr.Int64),
            ("val2", extr.Array(extr.Char, 32)),
        )
    )

    [m, inputs] = extr.system.module(1)
    m_op = m.features
    val_one = m_op.field(inputs[0], "val1")
    if with_time_lag:
        time_lag_out = m_op.time_lag(val_one, timedelta(milliseconds=1000), timedelta(0))
        m.declare_outputs(time_lag_out)
    else:
        m.declare_outputs(val_one)

    output = op.split_by(data_in, m, "val2", name="split_1")

    one = op.accumulate(output)

    graph.stream_ctx().run()
    extr.flush()

    as_pd_one = result_as_pandas(one)

    np.testing.assert_array_equal(as_pd_one['val1'], np.array(
        [x[1] for x in expected_output]))

    np.testing.assert_array_equal(as_pd_one['val2'], np.array(
        [x[2] for x in expected_output]))

    np.testing.assert_array_equal(as_pd_one['Timestamp'], np.array(
        [x[0] for x in expected_output], dtype=np.dtype('datetime64[ms]')))


if __name__ == "__main__":
    run(
        in_data_array=[
            [10, 1, 'TICKER_ONE'],
            [10, 2, 'TICKER_ONE'],
            [10, 3, 'TICKER_TWO'],

            [20, 4, 'TICKER_ONE'],
            [20, 5, 'TICKER_TWO'],

            [30, 6, 'TICKER_TWO'],
            [30, 7, 'TICKER_TWO'],
            [30, 8, 'TICKER_ONE'],
            [30, 9, 'TICKER_TWO'],
        ],
        expected_output=[
            [10, 1, 'TICKER_ONE'],
            [10, 2, 'TICKER_ONE'],
            [10, 3, 'TICKER_TWO'],

            [20, 4, 'TICKER_ONE'],
            [20, 5, 'TICKER_TWO'],

            [30, 6, 'TICKER_TWO'],
            [30, 7, 'TICKER_TWO'],
            [30, 8, 'TICKER_ONE'],
            [30, 9, 'TICKER_TWO'],
        ],
        with_time_lag=False,
    )
    run(
        in_data_array=[
            [10, 1, 'TICKER_ONE'],
            [10, 2, 'TICKER_ONE'],
            [10, 3, 'TICKER_TWO'],

            [20, 4, 'TICKER_ONE'],
            [20, 5, 'TICKER_TWO'],

            [30, 6, 'TICKER_TWO'],
            [30, 7, 'TICKER_TWO'],
            [30, 8, 'TICKER_ONE'],
            [30, 9, 'TICKER_TWO'],
        ],
        expected_output=[
            [10 + 1000, 1, 'TICKER_ONE'],
            [10 + 1000, 2, 'TICKER_ONE'],
            [10 + 1000, 3, 'TICKER_TWO'],

            [20 + 1000, 4, 'TICKER_ONE'],
            [20 + 1000, 5, 'TICKER_TWO'],

            [30 + 1000, 8, 'TICKER_ONE'],
            [30 + 1000, 6, 'TICKER_TWO'],
            [30 + 1000, 7, 'TICKER_TWO'],
            [30 + 1000, 9, 'TICKER_TWO'],
        ],
        with_time_lag=True,
    )

    run(
        in_data_array=[
            [10, 1, 'TICKER_ONE'],
            [10, 2, 'TICKER_ONE'],
            [10, 3, 'TICKER_TWO'],

            [20, 4, 'TICKER_THREE'],
            [20, 5, 'TICKER_ONE'],
            [20, 6, 'TICKER_ONE'],

            [30, 7, 'TICKER_TWO'],
            [30, 8, 'TICKER_THREE'],
            [30, 9, 'TICKER_TWO'],
            [30, 10, 'TICKER_ONE'],
            [30, 11, 'TICKER_TWO'],
        ],
        expected_output=[
            [10 + 1000, 1, 'TICKER_ONE'],
            [10 + 1000, 2, 'TICKER_ONE'],
            [10 + 1000, 3, 'TICKER_TWO'],

            [20 + 1000, 5, 'TICKER_ONE'],
            [20 + 1000, 6, 'TICKER_ONE'],
            [20 + 1000, 4, 'TICKER_THREE'],

            [30 + 1000, 10, 'TICKER_ONE'],
            [30 + 1000, 7, 'TICKER_TWO'],
            [30 + 1000, 9, 'TICKER_TWO'],
            [30 + 1000, 11, 'TICKER_TWO'],
            [30 + 1000, 8, 'TICKER_THREE'],
        ],
        with_time_lag=True,
    )
