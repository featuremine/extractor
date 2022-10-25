#!/usr/bin/env python3
"""
/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/
"""

"""
 * @file book_build.py
 * @author Ivan Gonzalez
 * @date 22 Nov 2021
 * @brief Python test for the book updates
 */
"""

import extractor as extr
from extractor import result_as_pandas
from pandas.testing import assert_frame_equal
import pandas as pd
from numpy.testing import assert_array_equal, assert_array_almost_equal
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":

    graph = extr.system.comp_graph()
    op = graph.features

    synth = op.book_play_split(os.path.join(src_dir, "data/synth_book.base.ore"), ("3_YEAR",))[0]

    book = op.book_build(synth, 2)

    accum_book = op.accumulate(book)

    # run context
    graph.stream_ctx().run()

    pd_book = result_as_pandas(accum_book)

    assert_array_equal(pd_book["Timestamp"].values,
                       pd.to_datetime(['1970-01-09T08:01:49.000000000', '1970-01-09T08:01:50.000000000',
                                       '1970-01-09T08:01:51.000000000', '1970-01-09T08:01:52.000000000',
                                       '1970-01-09T08:01:53.000000000', '1970-01-09T08:01:54.000000000',
                                       '1970-01-09T08:01:55.000000000', '1970-01-09T08:01:56.000000000',
                                       '1970-01-09T08:01:57.000000000', '1970-01-09T08:01:58.000000000',
                                       '1970-01-09T08:01:58.000000000', '1970-01-09T08:01:58.000000000',
                                       '1970-01-09T08:02:01.000000000', '1970-01-09T08:02:01.000000000',
                                       '1970-01-09T08:02:03.000000000', '1970-01-09T08:02:04.000000000',
                                       '1970-01-09T08:02:05.000000000']))

    assert_array_equal(pd_book["ask_ord_0"].values,
                       [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0])

    assert_array_equal(pd_book["ask_ord_1"].values,
                       [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

    assert_array_equal(pd_book["ask_prx_0"].values, [extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"),
                       extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0")])

    assert_array_equal(pd_book["ask_prx_1"].values,
                       [extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0")])

    assert_array_equal(pd_book["ask_shr_0"].values,
                       [extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("200.0"), extr.Decimal128("200.0"), extr.Decimal128("200.0"), extr.Decimal128("0")])

    assert_array_equal(pd_book["ask_shr_1"].values,
                       [extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0")])

    assert_array_equal(pd_book["bid_ord_0"].values,
                       [1, 2, 3, 2, 1, 1, 0, 1, 0, 1, 1, 1, 2, 2, 2, 1, 0])

    assert_array_equal(pd_book["bid_ord_1"].values,
                       [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0])

    assert_array_equal(pd_book["bid_prx_0"].values,
                       [extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0"), extr.Decimal128("0.2109375"), extr.Decimal128("0"),
                        extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0.2109375"), extr.Decimal128("0")])

    assert_array_equal(pd_book["bid_prx_1"].values,
                       [extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0")])

    assert_array_equal(pd_book["bid_shr_0"].values, [extr.Decimal128("200"), extr.Decimal128("400"), extr.Decimal128("600"), extr.Decimal128("400"), extr.Decimal128("200"), extr.Decimal128("250"),
                       extr.Decimal128("0"), extr.Decimal128("200"), extr.Decimal128("0"), extr.Decimal128("200"), extr.Decimal128("100"), extr.Decimal128("50"), extr.Decimal128("250"), extr.Decimal128("250"), extr.Decimal128("250"), extr.Decimal128("100"), extr.Decimal128("0")])

    assert_array_equal(pd_book["bid_shr_1"].values,
                       [extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0"), extr.Decimal128("0")])
