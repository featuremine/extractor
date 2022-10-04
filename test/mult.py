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
from extractor import result_as_pandas
import pytz
import pandas as pd
import numpy as np
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()
    op = graph.features

    ts = pd.to_datetime([65811153, 3513518183, 6111135331], unit='s')
    val1 = pd.Series([514541811.9, 9.999945, 3.4458], dtype='float64')
    val2 = pd.Series([3.64, 0.664, 9.3645], dtype='float64')
    val3 = pd.Series([5233.4, 6.24, 9.554], dtype='float64')
    val4 = pd.Series([21.002, 880.948, 0.55], dtype='float64')

    df = pd.DataFrame(data={"val1": val1,
                            "val2": val2,
                            "Timestamp": ts}).set_index("Timestamp")

    df2 = pd.DataFrame(data={"val1": val3,
                             "val2": val4,
                             "Timestamp": ts}).set_index("Timestamp")

    data_in_one = op.pandas_play(
        df,
        (("val1", extr.Float64),
         ("val2", extr.Float64)))

    data_in_two = op.pandas_play(
        df2,
        (("val1", extr.Float64),
         ("val2", extr.Float64)))

    val_one = op.field(data_in_one, "val1")
    val_two = op.field(data_in_two, "val1")

    out_stream_one = op.mult(val_one, val_two)
    one = op.accumulate(out_stream_one)

    out_stream_two = op.mult(data_in_one, data_in_two)
    two = op.accumulate(out_stream_two)

    out_stream_three = op.mult(val_one, data_in_two)
    three = op.accumulate(out_stream_three)

    out_stream_four = op.mult(data_in_one, val_two)
    four = op.accumulate(out_stream_four)

    out_stream_five = data_in_one * 5.0
    five = op.accumulate(out_stream_five)

    out_stream_six = val_one * 3.5
    six = op.accumulate(out_stream_six)

    out_stream_seven = 5.0 * data_in_one
    seven = op.accumulate(out_stream_seven)

    out_stream_eight = 3.5 * val_one
    eight = op.accumulate(out_stream_eight)

    graph.stream_ctx().run()

    as_pd_one = result_as_pandas(one)
    np.testing.assert_array_almost_equal(np.array(as_pd_one['val1']),
                                         np.array(val1 * val3))

    as_pd_two = result_as_pandas(two)

    res_two = pd.DataFrame(df.values * df2.values, columns=df.columns, index=df.index)

    np.testing.assert_array_almost_equal(as_pd_two['val1'], res_two['val1'])
    np.testing.assert_array_almost_equal(as_pd_two['val2'], res_two['val2'])

    as_pd_three = result_as_pandas(three)

    res_three = df2.mul(np.array(val1), axis=0)

    np.testing.assert_array_almost_equal(as_pd_three['val1'], res_three['val1'])
    np.testing.assert_array_almost_equal(as_pd_three['val2'], res_three['val2'])

    as_pd_four = result_as_pandas(four)

    res_four = df.mul(np.array(val3), axis=0)

    np.testing.assert_array_almost_equal(as_pd_four['val1'], res_four['val1'])
    np.testing.assert_array_almost_equal(as_pd_four['val2'], res_four['val2'])

    as_pd_five = result_as_pandas(five)
    np.testing.assert_array_almost_equal(np.array(as_pd_five['val1']),
                                         np.array(val1 * 5.0))
    np.testing.assert_array_almost_equal(np.array(as_pd_five['val2']),
                                         np.array(val2 * 5.0))

    as_pd_six = result_as_pandas(six)
    np.testing.assert_array_almost_equal(np.array(as_pd_six['val1']),
                                         np.array(val1 * 3.5))

    as_pd_seven = result_as_pandas(seven)
    np.testing.assert_array_almost_equal(np.array(as_pd_seven['val1']),
                                         np.array(5.0 * val1))
    np.testing.assert_array_almost_equal(np.array(as_pd_seven['val2']),
                                         np.array(5.0 * val2))

    as_pd_eight = result_as_pandas(eight)
    np.testing.assert_array_almost_equal(np.array(as_pd_eight['const']),
                                         np.array(3.5 * val1))
