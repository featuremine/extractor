
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
@package op_overload.py
@author Andres Rangel
@date 17 Jan 2019
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
from extractor import result_as_pandas
import pytz
import operator
from pandas.testing import assert_frame_equal
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def validate(op, string_op, op_one):
    ops = {
        '+': [operator.add, op.add],
        '-': [operator.sub, op.diff],
        '*': [operator.mul, op.mult],
        '/': [operator.truediv, op.divide],
        '**': [operator.pow, op.pow]
    }

    try:
        ops[string_op][0](op_one, "String")
        exit(1)
    except TypeError:
        pass

    overloaded_stream = ops[string_op][0](op_one, op_one)
    standard_stream = ops[string_op][1](op_one, op_one)
    overloaded_stream = op.accumulate(overloaded_stream)
    standard_stream = op.accumulate(standard_stream)

    return [overloaded_stream, standard_stream]


if __name__ == "__main__":
    graph = extr.system.comp_graph()
    other_graph = extr.system.comp_graph()
    op = graph.features
    other_op = graph.features

    in_file_one = os.path.join(src_dir, "data/arithmetical_op_file_one.csv")

    data_in_one = op.csv_play(
        in_file_one,
        (("timestamp", extr.Time64, ""),
         ("val1", extr.Float64, ""),
         ("val2", extr.Int64, "")))

    val_one = data_in_one.val1

    ops_test = ["+", "-", "*", "/", "**"]

    out_streams = []

    for test in ops_test:
        test_stream = validate(op, test, val_one)
        out_streams.append(test_stream)

    try:
        test_stream = 0 and val_one
        if test_stream != 0:
            exit(1)
    except TypeError:
        pass

    try:
        test_stream = val_one and "String"
        if test_stream != "String":
            exit(1)
    except TypeError:
        pass

    try:
        test_stream = 0 or val_one
        if not isinstance(test_stream, extr.Feature):
            exit(1)
    except TypeError:
        pass

    try:
        test_stream = val_one or "String"
        if not isinstance(test_stream, extr.Feature):
            exit(1)
    except TypeError:
        pass

    and_stream = op.logical_and(op.convert(val_one, extr.Bool), op.convert(val_one, extr.Bool))
    overloaded_and_stream = op.convert(val_one, extr.Bool) and op.convert(val_one, extr.Bool)
    and_stream = op.accumulate(and_stream)
    overloaded_and_stream = op.accumulate(overloaded_and_stream)

    or_stream = op.logical_or(op.convert(val_one, extr.Bool), op.convert(val_one, extr.Bool))
    overloaded_or_stream = op.convert(val_one, extr.Bool) or op.convert(val_one, extr.Bool)
    or_stream = op.accumulate(or_stream)
    overloaded_or_stream = op.accumulate(overloaded_or_stream)

    graph.stream_ctx().run()

    for out in out_streams:
        assert_frame_equal(result_as_pandas(out[0]), result_as_pandas(out[1]))

    assert_frame_equal(result_as_pandas(and_stream), result_as_pandas(overloaded_and_stream))
    assert_frame_equal(result_as_pandas(or_stream), result_as_pandas(overloaded_or_stream))
