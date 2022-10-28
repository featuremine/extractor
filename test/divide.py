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
@package divide.py
@author Andres Rangel
@date 18 Jan 2019
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
    out_file_one = os.path.join(src_dir, "data/divide.test.csv")

    data_in_one = op.csv_play(
        in_file_one,
        (("timestamp", extr.Time64, ""),
         ("val1", extr.Float64, ""),
         ("val2", extr.Int32, "")))
    data_in_two = op.csv_play(
        in_file_two,
        (("timestamp", extr.Time64, ""),
         ("val1", extr.Float64, ""),
         ("val2", extr.Int32, "")))

    val_one = op.convert(op.field(data_in_one, "val2"), extr.Rational64)
    val_two = op.convert(op.field(data_in_two, "val2"), extr.Rational64)

    out_stream_one = op.divide(val_one, val_two)

    op.csv_record(out_stream_one, out_file_one)

    graph.stream_ctx().run()

    extr.flush()

    extr.assert_base(os.path.join(src_dir, 'data/divide.base.csv'), os.path.join(src_dir, 'data/divide.test.csv'))
