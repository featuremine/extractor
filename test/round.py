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
@package round.py
@author Andres Rangel
@date 10 May 2019
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
    graph = extr.system.comp_graph()
    op = graph.features

    ts = pd.to_datetime([65811153, 3513518183, 6111135331], unit='s')
    val1 = pd.Series([514541811.954486, 9.999945, 3.44], dtype='float64')
    val2 = pd.Series([3.64257588, 0.445757664, 9.387757645], dtype='float64')

    df = pd.DataFrame(data={"val1": val1,
                            "val2": val2,
                            "Timestamp": ts}).set_index("Timestamp")

    data_in = op.pandas_play(
        df,
        (("val1", extr.Float64),
         ("val2", extr.Float64)))

    rounded_40000 = op.round(data_in, 40000)
    rounded_512 = op.round(data_in, 512)
    rounded_256 = op.round(data_in, 256)
    rounded_4 = op.round(data_in, 4)
    rounded_2 = op.round(data_in, 2)
    rounded_1 = op.round(data_in, 1)
    rounded_1_128 = op.round(data_in, 1, extr.Decimal128)

    try:
        rounded_fail = op.round(data_in, 3)
        exit(1)
    except RuntimeError as e:
        pass

    fourtyk = op.accumulate(rounded_40000)
    fivehoundredtwelve = op.accumulate(rounded_512)
    twohoundredfiftysix = op.accumulate(rounded_256)
    four = op.accumulate(rounded_4)
    two = op.accumulate(rounded_2)
    one = op.accumulate(rounded_1)
    one128 = op.accumulate(rounded_1_128)

    graph.stream_ctx().run()

    as_pd = result_as_pandas(fourtyk)
    np.testing.assert_array_almost_equal(np.array(as_pd['val1']),
                                         np.array([514541811.954474, 9.999950, 3.439999]))

    as_pd = result_as_pandas(fivehoundredtwelve)
    np.testing.assert_array_almost_equal(np.array(as_pd['val1']),
                                         np.array([514541811.955078, 10.0, 3.439453]))

    as_pd = result_as_pandas(twohoundredfiftysix)
    np.testing.assert_array_almost_equal(np.array(as_pd['val1']),
                                         np.array([514541811.953125, 10.0, 3.441406]))

    as_pd = result_as_pandas(four)
    np.testing.assert_array_almost_equal(np.array(as_pd['val1']),
                                         np.array([514541812.0, 10.0, 3.5]))
    as_pd = result_as_pandas(two)
    np.testing.assert_array_almost_equal(np.array(as_pd['val1']),
                                         np.array([514541812.0, 10.0, 3.5]))

    as_pd = result_as_pandas(one)
    np.testing.assert_array_almost_equal(np.array(as_pd['val1']),
                                         np.array([514541812.0, 10.0, 3.0]))

    as_pd = result_as_pandas(one128)
    np.testing.assert_array_equal(np.array(as_pd['val1']),
                                  np.array([extr.Decimal128(514541812), extr.Decimal128(10), extr.Decimal128(3)]))

    ts = pd.to_datetime([65811153, 3513518183, 6111135331], unit='s')
    val1 = pd.Series([11.44, 200.06, 399.55], dtype='float64')
    val2 = pd.Series([0.5, 0.499, -0.5], dtype='float64')

    df = pd.DataFrame(data={"val1": val1,
                            "val2": val2,
                            "Timestamp": ts}).set_index("Timestamp")

    data_in = op.pandas_play(
        df,
        (("val1", extr.Decimal64),
         ("val2", extr.Decimal64)))

    rounded_dec = op.round(data_in)

    dec = op.accumulate(rounded_dec)

    graph.stream_ctx().run()

    as_pd = result_as_pandas(dec)
    np.testing.assert_array_equal(np.array(as_pd['val1']),
                                  np.array([11, 200, 400]))

    np.testing.assert_array_equal(np.array(as_pd['val2']),
                                  np.array([1, 0, -1]))
