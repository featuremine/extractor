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
import pytz
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    in_file_one = os.path.join(src_dir, "data/arithmetical_op_file_one.csv")
    in_file_two = os.path.join(src_dir, "data/arithmetical_op_file_two.csv")
    out_file_one = os.path.join(src_dir, "data/add.test.csv")

    data_in_one = op.csv_play(
        in_file_one,
        (("timestamp", extr.Time64, ""),
         ("val1", extr.Decimal64, ""),
         ("val2", extr.Int32, "")))
    data_in_two = op.csv_play(
        in_file_two,
        (("timestamp", extr.Time64, ""),
         ("val1", extr.Decimal64, ""),
         ("val2", extr.Int32, "")))

    val_one = op.field(data_in_one, "val2")
    val_two = op.field(data_in_two, "val2")

    out_stream_one = op.add(val_one, val_two)
    out_stream_two = op.add(val_one, val_one)

    true = op.constant(('a', extr.Bool, True))
    true_ref = graph.get_ref(true)

    false = op.constant(('a', extr.Bool, False))
    false_ref = graph.get_ref(false)

    int16 = op.constant(('a', extr.Int16, -16))
    int16_ref = graph.get_ref(int16)

    uint16 = op.constant(('a', extr.Uint16, 16))
    uint16_ref = graph.get_ref(uint16)

    signed_zero = op.constant(('a', extr.Int16, 0))
    signed_zero_ref = graph.get_ref(signed_zero)

    unsigned_zero = op.constant(('a', extr.Uint16, 0))
    unsigned_zero_ref = graph.get_ref(unsigned_zero)

    op.csv_record(out_stream_one, out_file_one)

    graph.stream_ctx().run()

    extr.flush()

    extr.assert_base(os.path.join(src_dir, 'data/add.base.csv'), os.path.join(src_dir, 'data/add.test.csv'))

    assert true_ref[0].a
    assert false_ref[0].a == False
    assert int16_ref[0].a == -16
    assert uint16_ref[0].a == 16
    assert signed_zero_ref[0].a == 0
    assert unsigned_zero_ref[0].a == 0
