
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


def validate(op, string_op, op_one, op_two):
    print("validating", string_op)
    ops = {
        '+': [operator.add, op.add],
        '-': [operator.sub, op.diff],
        '*': [operator.mul, op.mult],
        '/': [operator.truediv, op.divide],
        '**': [operator.pow, op.pow],
        '==': [operator.eq, op.equal],
        '!=': [operator.ne, op.not_equal],
        '>': [operator.gt, op.greater],
        '>=': [operator.ge, op.greater_equal],
        '<': [operator.lt, op.less],
        '<=': [operator.le, op.less_equal]
    }

    if string_op != '==' and string_op != '!=':
        try:
            ops[string_op][0](op_one, "String")
            exit(1)
        except TypeError:
            pass

    print("building streams", string_op)
    overloaded_stream = ops[string_op][0](op_one, op_two)
    standard_stream = ops[string_op][1](op_one, op_two)
    overloaded_stream = op.accumulate(overloaded_stream)
    standard_stream = op.accumulate(standard_stream)

    return [overloaded_stream, standard_stream]


def validate_cond(op, string_op, op_one, op_two):
    ops = {
        '&': [operator.and_, op.logical_and],
        '|': [operator.or_, op.logical_or]
    }

    try:
        ops[string_op][0](op_one, "String")
        exit(1)
    except TypeError:
        pass

    overloaded_stream = ops[string_op][0](op_one, op_two)
    standard_stream = ops[string_op][1](op_one, op_two)
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

    ops_test = ["+", "-", "*", "/", "**", "==", "!=", ">", ">=", "<", "<="]

    cond_ops_test = ["&", "|"]

    out_streams = []

    for test in ops_test:
        test_stream = validate(op, test, val_one, val_one)
        out_streams.append(test_stream)

    for test in ops_test:
        test_stream = validate(op, test, val_one, val_one - val_one)
        out_streams.append(test_stream)

    for test in cond_ops_test:
        test_stream = validate_cond(op, test, op.convert(val_one, extr.Bool), op.convert(val_one, extr.Bool))
        out_streams.append(test_stream)

    for test in cond_ops_test:
        test_stream = validate_cond(op, test, op.convert(val_one, extr.Bool), op.logical_not(op.convert(val_one, extr.Bool)))
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

    graph.stream_ctx().run()

    for out in out_streams:
        assert_frame_equal(result_as_pandas(out[0]), result_as_pandas(out[1]))
