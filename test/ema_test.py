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
@package ema_test.py
@author Andres Rangel
@date 4 Feb 2021
@brief File contains extractor python sample
"""

import extractor as extr
import pandas as pd
from datetime import timedelta, datetime
import pytz
import numpy as np
import os
import sys

src_dir = os.path.dirname(os.path.realpath(__file__))


def epoch_delta(date):
    return date - pytz.timezone("UTC").localize(datetime(1970, 1, 1))


def New_York_time(year, mon, day, h=0, m=0, s=0):
    return epoch_delta(pytz.timezone("America/New_York").
                       localize(datetime(year, mon, day, h, m, s)))


if __name__ == "__main__":
    extr.system.load_ext("ema", sys.argv[1])
    graph = extr.system.comp_graph()
    op = graph.features

    ts = pd.to_datetime([65811153, 351351818, 611113533], unit='s')
    bid = pd.Series([3.64, 9.63, 55.42], dtype='float64')

    df = pd.DataFrame(data={"bidprice": bid,
                            "receive": ts}).set_index("receive")

    bid_prices = op.pandas_play(df, (("bidprice", extr.Float64),), name="mp_play_0")

    # One input
    one = op.ema(bid_prices, 0.99)
    one_aggr = op.accumulate(one)
    one_frame = graph.get_ref(one_aggr)

    # Two inputs, both with a single float64 field
    two = op.ema(bid_prices, bid_prices, 0.99)
    two_aggr = op.accumulate(two)
    two_frame = graph.get_ref(two_aggr)

    t = op.timer(timedelta(seconds=300000000), name='timer1')
    # Two inputs, first without float64 field
    three = op.ema(t, bid_prices, 0.99)
    three_aggr = op.accumulate(three)
    three_frame = graph.get_ref(three_aggr)

    # Multiple inputs, first without float64 field, others with a single float64 field
    four = op.ema(t, bid_prices, bid_prices, bid_prices, 0.99)
    four_aggr = op.accumulate(four)
    four_frame = graph.get_ref(four_aggr)

    try:
        one = op.ema(bid_prices)
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Correct arguments, no params did not fail as expected")

    try:
        one = op.ema(0.99)
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Zero arguments, correct param did not fail as expected")

    try:
        one = op.ema(bid_prices, "0.99")
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Incorrect type for alpha did not fail as expected")

    try:
        one = op.ema(t, bid_prices, "0.99")
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Incorrect type for alpha with secondary frame did not fail as expected")

    try:
        one = op.ema(bid_prices, 0.99, "otherparam")
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Incorrect number of parameters did not fail as expected")

    try:
        one = op.ema(t, bid_prices, 0.99, "otherparam")
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Incorrect number of parameters with secondary frame did not fail as expected")

    try:
        df = pd.DataFrame(data={"bidprice": bid,
                                "othercolumn": bid,
                                "receive": ts}).set_index("receive")

        invalid_bid_prices = op.pandas_play(
            df, (("bidprice", extr.Float64), ("othercolumn", extr.Float64)), name="mp_play_0")
        one = op.ema(invalid_bid_prices, 0.99)
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Incorrect number of fields in frame")

    try:
        df = pd.DataFrame(data={"bidprice": bid,
                                "othercolumn": bid,
                                "receive": ts}).set_index("receive")

        invalid_bid_prices = op.pandas_play(
            df, (("bidprice", extr.Float64), ("othercolumn", extr.Float64)), name="mp_play_0")
        one = op.ema(t, invalid_bid_prices, 0.99)
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Incorrect number of fields in secondary frame")

    try:
        bid = pd.Series([3, 9, 55], dtype='int64')
        df = pd.DataFrame(data={"bidprice": bid,
                                "receive": ts}).set_index("receive")

        invalid_bid_prices = op.pandas_play(df, (("bidprice", extr.Float64),), name="mp_play_0")
        one = op.ema(invalid_bid_prices, 0.99)
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Incorrect type of field in frame")

    try:
        bid = pd.Series([3, 9, 55], dtype='int64')
        df = pd.DataFrame(data={"bidprice": bid,
                                "receive": ts}).set_index("receive")

        invalid_bid_prices = op.pandas_play(df, (("bidprice", extr.Float64),), name="mp_play_0")
        one = op.ema(t, invalid_bid_prices, 0.99)
    except RuntimeError:
        pass
    else:
        raise RuntimeError("Incorrect type of field in frame in secondary frame")

    graph.stream_ctx().run_to(New_York_time(2017, 10, 18, 16))

    np.testing.assert_almost_equal(one_frame[0].result, 3.6036)
    np.testing.assert_almost_equal(one_frame[1].result, 9.569736)
    np.testing.assert_almost_equal(one_frame[2].result, 54.96149736)

    np.testing.assert_almost_equal(two_frame[0].result, 3.6036)
    np.testing.assert_almost_equal(two_frame[1].result, 9.569736)
    np.testing.assert_almost_equal(two_frame[2].result, 54.96149736)

    assert len(three_frame) == 5
    np.testing.assert_almost_equal(three_frame[0].result, 3.639636)
    np.testing.assert_almost_equal(three_frame[1].result, 9.629400964)
    np.testing.assert_almost_equal(three_frame[2].result, 55.41542094)
    np.testing.assert_almost_equal(three_frame[3].result, 55.41995421)
    np.testing.assert_almost_equal(three_frame[4].result, 55.41999954)

    assert len(four_frame) == 5
    np.testing.assert_almost_equal(four_frame[0].result, 10.918908)
    np.testing.assert_almost_equal(four_frame[1].result, 28.88820289)
    np.testing.assert_almost_equal(four_frame[2].result, 166.2462628)
    np.testing.assert_almost_equal(four_frame[3].result, 166.2598626)
    np.testing.assert_almost_equal(four_frame[4].result, 166.2599986)
