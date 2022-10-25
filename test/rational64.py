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
@package rational64.py
@author Andres Rangel
@date 8 Jan 2019
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import filecmp
import sys
import extractor as extr
from extractor import result_as_pandas
from pandas.testing import assert_frame_equal
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":

    graph = extr.system.comp_graph()
    op = graph.features

    csv_data_in = op.csv_play(os.path.join(src_dir, "data/rational_input.csv"),
                              (("timestamp", extr.Time64, ""),
                               ("val1", extr.Rational64, ""),
                                  ("val2", extr.Decimal64, "")), name="csv_play_0")

    decimal_data = op.field(csv_data_in, "val2")

    rational_one = op.convert(decimal_data, extr.Rational64)
    decimal_double = op.convert(decimal_data, extr.Float64)
    rational_double = op.convert(rational_one, extr.Float64)

    mp_data_in = op.mp_play(os.path.join(src_dir, "data/rational.base.mp"),
                            (("timestamp", extr.Time64, ""),
                             ("val1", extr.Rational64, ""),
                                ("val2", extr.Decimal64, "")), name="mp_play_0")

    csv_data_out = op.csv_record(csv_data_in,
                                 os.path.join(src_dir, "data/rational.test.csv"), name="csv_record_0")

    mp_data_out = op.mp_record(csv_data_in,
                               os.path.join(src_dir, "data/rational.test.mp"), name="mp_record_0")

    aggr = op.accumulate(csv_data_in)
    mp_aggr = op.accumulate(mp_data_in)
    d_aggr = op.accumulate(decimal_double)
    r_aggr = op.accumulate(rational_double)

    graph.stream_ctx().run()

    as_pd = result_as_pandas(aggr)
    mp_as_pd = result_as_pandas(mp_aggr)
    d_pd = result_as_pandas(d_aggr)
    r_pd = result_as_pandas(r_aggr)

    assert_frame_equal(as_pd, mp_as_pd)

    assert_frame_equal(d_pd, r_pd)

    extr.flush()

    extr.assert_base(os.path.join(src_dir, 'data/rational.base.csv'), os.path.join(src_dir, 'data/rational.test.csv'))
    extr.assert_base(os.path.join(src_dir, 'data/rational.base.mp'), os.path.join(src_dir, 'data/rational.test.mp'))
