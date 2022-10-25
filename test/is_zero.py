#!/usr/bin/python3
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
@package is_zero.py
@author Andres Rangel
@date 2 Oct 2018
@brief File contains extractor python sample
"""
import extractor as extr
from datetime import timedelta
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":

    graph = extr.system.comp_graph()
    op = graph.features

    in_file = os.path.join(src_dir, "data/is_zero_op_file.csv")
    out_file = os.path.join(src_dir, "data/is_zero.test.csv")

    data_in = op.csv_play(
        in_file,
        (("timestamp", extr.Time64, ""),
         ("val1", extr.Float64, ""),
         ("val2", extr.Int32, "")))

    zero_data = graph.features.is_zero(data_in)

    zero_data_ref = graph.get_ref(zero_data)

    zero_float = op.constant(("zerofloat", extr.Float64, 0.0))
    zero_float_data = graph.features.is_zero(zero_float)
    zero_float_data_ref = graph.get_ref(zero_float_data)

    not_zero_float = op.constant(("notzerofloat", extr.Float64, 1.1))
    not_zero_float_data = graph.features.is_zero(not_zero_float)
    not_zero_float_data_ref = graph.get_ref(not_zero_float_data)

    zero_int = op.constant(("zeroint", extr.Int64, 0))
    zero_int_data = graph.features.is_zero(zero_int)
    zero_int_data_ref = graph.get_ref(zero_int_data)

    not_zero_int = op.constant(("notzeroint", extr.Int64, 1))
    not_zero_int_data = graph.features.is_zero(not_zero_int)
    not_zero_int_data_ref = graph.get_ref(not_zero_int_data)

    zero_ts = op.constant(("zerots", extr.Time64, timedelta(seconds=0)))
    zero_ts_data = graph.features.is_zero(zero_ts)
    zero_ts_data_ref = graph.get_ref(zero_ts_data)

    not_zero_ts = op.constant(("notzerots", extr.Time64, timedelta(seconds=1)))
    not_zero_ts_data = graph.features.is_zero(not_zero_ts)
    not_zero_ts_data_ref = graph.get_ref(not_zero_ts_data)

    zero_dec = op.constant(("zerodec", extr.Decimal64, 0.0))
    zero_dec_data = graph.features.is_zero(zero_dec)
    zero_dec_data_ref = graph.get_ref(zero_dec_data)

    not_zero_dec = op.constant(("notzerodec", extr.Decimal64, 1.0))
    not_zero_dec_data = graph.features.is_zero(not_zero_dec)
    not_zero_dec_data_ref = graph.get_ref(not_zero_dec_data)

    op.csv_record(zero_data, out_file)

    ctx = graph.stream_ctx()

    ctx.run()

    assert zero_data_ref[0].timestamp == False
    assert zero_data_ref[0].val1
    assert zero_data_ref[0].val2

    assert zero_float_data_ref[0].zerofloat
    assert not_zero_float_data_ref[0].notzerofloat == False

    assert zero_int_data_ref[0].zeroint
    assert not_zero_int_data_ref[0].notzeroint == False

    assert zero_ts_data_ref[0].zerots
    assert not_zero_ts_data_ref[0].notzerots == False

    assert zero_dec_data_ref[0].zerodec
    assert not_zero_dec_data_ref[0].notzerodec == False

    extr.flush()

    extr.assert_base(os.path.join(src_dir, 'data/is_zero.base.csv'), os.path.join(src_dir, 'data/is_zero.test.csv'))
