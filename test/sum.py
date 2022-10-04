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
@package sum.py
@author Andres Rangel
@date 18 Dec 2018
@brief File contains extractor python sample
"""

import extractor as extr
from extractor import result_as_pandas
import numpy as np
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def zero_fill_nan(frame, op):
    float_frame = op.convert(frame, extr.Float64)
    return op.cond(op.is_zero(frame), op.nan(float_frame), float_frame)


def sum_test(value_gen):
    graph = extr.system.comp_graph()
    op = graph.features

    trades_in = op.csv_play(
        trade_file,
        (("receive", extr.Time64, ""),
         ("val1", extr.Int64, ""),
         ("val2", extr.Int64, ""),
         ("val3", extr.Int64, ""),
         ("val4", extr.Int64, ""),
         ("val5", extr.Int64, "")))

    fields = [value_gen(op.unique(op.field(trades_in, "val" + str(x))), op) for x in range(1, 6)]
    aggr = op.accumulate(op.sum(*fields))

    graph.stream_ctx().run()

    as_pd = result_as_pandas(aggr)
    as_pd = as_pd['val1']
    return as_pd


def unique(value, op):
    return value


def unique_nan(value, op):
    return zero_fill_nan(value, op)


if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))

    trade_file = os.path.join(src_dir, "data/sum_input.csv")
    np.testing.assert_array_equal(sum_test(unique),
                                  np.array([16, 12, 21, 21, 124, 0, 321, 197, 225, 62, 113, 96, 98]))
    np.testing.assert_array_equal(sum_test(unique_nan),
                                  np.array([16, 12, 21, 21, 124, 0, 321, 197, 225, 62, 113, 96, 98]))
