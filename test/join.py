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
@package join.py
@author Andres Rangel
@date 27 Jan 2019
@brief File contains extractor test for join operator
"""

import extractor as extr
import pytz
import pandas as pd
import numpy as np
from pandas.testing import assert_frame_equal
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def assert_frame_not_equal(*args, **kwargs):
    try:
        assert_frame_equal(*args, **kwargs)
    except AssertionError:
        pass
    else:
        raise AssertionError


if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    ts = pd.to_datetime([1580115600], unit='s')
    one = pd.Series([3], dtype='int64')
    two = pd.Series([4], dtype='int64')

    df = pd.DataFrame(data={"one": one,
                            "two": two,
                            "ts": ts,
                            "Timestamp": ts}).set_index("Timestamp")

    pd_in = op.pandas_play(df,
                           (("ts", extr.Time64),
                            ("one", extr.Int64),
                               ("two", extr.Int64)))

    csv_in = op.csv_play(os.path.join(src_dir, "data/join.csv"),
                         (("ts", extr.Time64),
                          ("one", extr.Int64),
                             ("two", extr.Int64)))

    join = op.last(pd_in, csv_in, "name", extr.Array(extr.Char, 16), ("pd", "csv"))

    accum = op.accumulate(join)

    ctx = graph.stream_ctx()

    clean = pd.DataFrame(data={"name": [""],
                               "one": [0],
                               "ts": pd.to_datetime([0], unit='s'),
                               "two": [0]})

    assert_frame_equal(extr.result_as_pandas(join), clean)

    ctx.run()

    assert_frame_not_equal(extr.result_as_pandas(join), clean)

    ts = pd.to_datetime([1580115600, 1580137200, 1580151600, 1580166000], unit='s')
    one = pd.Series([3, 1, 9, 1], dtype='int64')
    two = pd.Series([4, 4, 6, 1], dtype='int64')
    three = pd.Series(["pd", "csv", "csv", "csv"], dtype='str')

    resultdf = pd.DataFrame(data={"Timestamp": ts,
                                  "name": three,
                                  "one": one,
                                  "ts": ts,
                                  "two": two,
                                  })

    assert_frame_equal(extr.result_as_pandas(accum), resultdf)
