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
@package custom.py
@author Maxim Trokhimtchouk
@date 22 Apr 2018
@brief File contains extractor python sample
"""

import extractor as extr
import numpy as np
import pandas as pd
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

# Tests for assignment of constant to field dimension


class PyClass(object):
    def __init__(self, inputs, field_names, dims=1):
        self.field_names = field_names
        self.fields = len(field_names)
        self.dimy = dims
        if len(inputs) != 2:
            raise RuntimeError("Invalid number of arguments passed, expected two")
        if len(inputs[0]) != len(inputs[1]):
            raise RuntimeError("Inputs must have the same number of fields")
        if len(inputs[0]) != len(self.field_names):
            raise RuntimeError("Incorrect number of provided output field names\n"
                               "expecting {0}, provided {1}".format(len(self.field_names)),
                               len(inputs[0]))
        for idx in range(len(inputs[0])):
            if inputs[0][idx][0] != inputs[1][idx][0]:
                raise RuntimeError("Expected fields in frame to be of same type.\n"
                                   "{0} and {1} are not equal".format(inputs[0][idx][0],
                                                                      inputs[1][idx][0]))
        self.type = tuple([dims, tuple([(target_type[0], inp)
                                        for target_type, inp in zip(inputs[0], self.field_names)])])

    def init(self, result, argvone, argvtwo):
        for y in range(self.dimy):
            result[y].result_field_name = argvone[0].val2 + argvtwo[0].val2 + y
        return True

    def exec(self, result, argvone, argvtwo):
        for y in range(self.dimy):
            result[y].result_field_name = argvone[0].val2 + argvtwo[0].val2 + y
        return True

# Tests for no args and assignment of field and constant (field is 1dimensional) to to field


class PyClassSwitch(object):
    def __init__(self, inputs):
        self.field_names = ("val2",)
        self.fields = len(self.field_names)
        if len(inputs) != 2:
            raise RuntimeError("Invalid number of arguments passed, expected two")
        if len(inputs[0]) != len(inputs[1]):
            raise RuntimeError("Inputs must have the same number of fields")
        if len(inputs[0]) != len(self.field_names):
            raise RuntimeError("Incorrect number of provided output field names\n"
                               "expecting {0}, provided {1}".format(len(self.field_names)),
                               len(inputs[0]))
        for idx in range(len(inputs[0])):
            if inputs[0][idx][0] != inputs[1][idx][0]:
                raise RuntimeError("Expected fields in frame to be of same type.\n"
                                   "{0} and {1} are not equal".format(inputs[0][idx][0],
                                                                      inputs[1][idx][0]))
        self.type = tuple([1, tuple([(target_type[0], inp) for target_type, inp in zip(inputs[0], self.field_names)])])
        self.first = True

    def init(self, result, *argv):
        for field in self.field_names:
            setattr(result[0], field, getattr(argv[0][0], field))
        return True

    def exec(self, result, *argv):
        for field in self.field_names:
            setattr(result[0], field, getattr(argv[0][0], field) if self.first else getattr(argv[1][0], field) + 1)
        self.first = not self.first
        return True

# Tests for false executions


class PyClassFilter(object):
    def __init__(self, inputs):
        self.field_names = ("val2",)
        self.fields = len(self.field_names)
        if len(inputs) != 1:
            raise RuntimeError("Invalid number of arguments passed, expected one")
        if len(inputs[0]) != len(self.field_names):
            raise RuntimeError("Incorrect number of provided output field names\n"
                               "expecting {0}, provided {1}".format(len(self.field_names)),
                               len(inputs[0]))
        self.type = tuple([1, tuple([(target_type[0], inp) for target_type, inp in zip(inputs[0], self.field_names)])])
        self.first = True

    def init(self, result, *argv):
        for field in self.field_names:
            setattr(result[0], field, getattr(argv[0][0], field))
        return True

    def exec(self, result, *argv):
        ret = self.first
        self.first = not self.first
        if ret:
            for field in self.field_names:
                setattr(result[0], field, getattr(argv[0][0], field))
        return ret


class PyCharArray(object):
    def __init__(self, inputs):
        if len(inputs) != 1:
            raise RuntimeError("Invalid number of arguments passed, expected one")
        self.field_names = [inp[1] for inp in inputs[0]]
        self.type = tuple([1, inputs[0]])

    def init(self, result, argv):
        for field in self.field_names:
            setattr(result[0], field, getattr(argv[0], field))
        return True

    def exec(self, result, argv):
        for field in self.field_names:
            setattr(result[0], field, getattr(argv[0], field))
        return True


if __name__ == "__main__":
    sys = extr.system
    sys.extend(PyClass, "pyclassop")
    graph = sys.comp_graph()
    op = graph.features

    in_file_one = os.path.join(src_dir, "data/arithmetical_op_file_one.csv")
    in_file_two = os.path.join(src_dir, "data/arithmetical_op_file_two.csv")

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

    ts = pd.to_datetime([65811153, 3513518183, 6111135331], unit='s')
    f32 = pd.Series([514541811.9, 9.999945, 3.4458], dtype='float32')
    f64 = pd.Series([3.64, 0.664, 9.3645], dtype='float64')
    i16 = pd.Series([-943, -153, 536], dtype='int16')
    i32 = pd.Series([-64883, 4, -82], dtype='int32')
    i64 = pd.Series([3, 0, -9], dtype='int64')
    i8 = pd.Series([22, 11, 38], dtype='int8')
    ui16 = pd.Series([943, 153, 536], dtype='uint16')
    ui32 = pd.Series([264883, 554, 25582], dtype='uint32')
    ui64 = pd.Series([322235, 0, 97625], dtype='uint64')
    ui8 = pd.Series([21, 54, 11], dtype='uint8')
    b = pd.Series([True, False, True], dtype='bool')
    s16 = pd.Series([b"True", b"False", b"True"], dtype='S16')
    s = pd.Series(["MaybeFalse", "MaybeTrue", "MaybeFalse"], dtype='str')

    df = pd.DataFrame(data={  # Binary string
        "binstring_col": s16,
        # Bool
        "bool_col": b,
        # Decimal
        "decimal_from_float32_col": f32,
        "decimal_from_float64_col": f64,
        # Float 32
        "float32_col": f32,
        # Float 64
        "float64_col": f64,
        "float64_from_float32_col": f32,
        # Int 16
        "int16_col": i16, "int16_from_int8_col": i8,
        # Int 32
        "int32_col": i32, "int32_from_int16_col": i16,
        "int32_from_int8_col": i8,
        # Int 64
        "int64_col": i64, "int64_from_int16_col": i16,
        "int64_from_int32_col": i32, "int64_from_int8_col": i8,
        # Int8
        "int8_col": i8,
        # Unicode string
        "string_col": s,
        # Timestamps
        "Timestamp": ts,
        "timestamp_col": ts,
        # Uint 16
        "uint16_col": ui16, "uint16_from_uint8_col": ui8,
        # Uint 32
        "uint32_col": ui32, "uint32_from_uint16_col": ui16,
        "uint32_from_uint8_col": ui8,
        # Uint 64
        "uint64_col": ui64, "uint64_from_uint16_col": ui16,
        "uint64_from_uint32_col": ui32, "uint64_from_uint8_col": ui8,
        # Uint 8
        "uint8_col": ui8}).set_index("Timestamp")

    data_in_three = op.pandas_play(
        df,
        (("binstring_col", extr.Array(extr.Char, 16)),
         ("bool_col", extr.Bool),
         ("decimal_from_float32_col", extr.Decimal64),
         ("decimal_from_float64_col", extr.Decimal64),
         ("float32_col", extr.Float32),
         ("float64_col", extr.Float64),
         ("float64_from_float32_col", extr.Float64),
         ("int16_col", extr.Int16),
         ("int16_from_int8_col", extr.Int16),
         ("int32_col", extr.Int32),
         ("int32_from_int16_col", extr.Int32),
         ("int32_from_int8_col", extr.Int32),
         ("int64_col", extr.Int64),
         ("int64_from_int16_col", extr.Int64),
         ("int64_from_int32_col", extr.Int64),
         ("int64_from_int8_col", extr.Int64),
         ("int8_col", extr.Int8),
         ("string_col", extr.Array(extr.Char, 16)),
         ("timestamp_col", extr.Time64),
         ("uint16_col", extr.Uint16),
         ("uint16_from_uint8_col", extr.Uint16),
         ("uint32_col", extr.Uint32),
         ("uint32_from_uint16_col", extr.Uint32),
         ("uint32_from_uint8_col", extr.Uint32),
         ("uint64_col", extr.Uint64),
         ("uint64_from_uint32_col", extr.Uint64),
         ("uint64_from_uint16_col", extr.Uint64),
         ("uint64_from_uint8_col", extr.Uint64),
         ("uint8_col", extr.Uint8)))

    # test char
    char = pd.Series(["A", "B", "C"], dtype='str')

    test_dataframe = pd.DataFrame(data={
        "char": char,
        "timestamp": ts
    }).set_index("timestamp")

    test_data = op.pandas_play(
        test_dataframe,
        (
            ("char", extr.Char),
        ))

    test_char = op.custom(test_data, PyCharArray)
    accum_test = op.accumulate(test_char)
    # end test char

    val_one = op.field(data_in_one, "val2")
    val_two = op.field(data_in_two, "val2")

    custom = op.custom(data_in_one.val2, data_in_two.val2, PyClass, ("result_field_name",), 5)
    customswitch = op.custom(data_in_one.val2, data_in_two.val2, PyClassSwitch)
    customfilter = op.custom(data_in_one.val2, PyClassFilter)
    customstr = op.custom(data_in_three, PyCharArray)

    pyclassop = op.pyclassop(data_in_one.val2, data_in_two.val2, ("result_field_name",))

    accum = op.accumulate(pyclassop)
    accumswitch = op.accumulate(customswitch)
    accumfilter = op.accumulate(customfilter)
    accumstr = op.accumulate(customstr)

    graph.stream_ctx().run()

    np.testing.assert_array_equal(extr.result_as_pandas(custom)['result_field_name'],
                                  np.array([28, 29, 30, 31, 32]))
    np.testing.assert_array_equal(extr.result_as_pandas(accum)['result_field_name'],
                                  np.array([33, 15, 18, 28]))
    np.testing.assert_array_equal(extr.result_as_pandas(accumswitch)['val2'],
                                  np.array([21, 6, 15, 7]))
    np.testing.assert_array_equal(extr.result_as_pandas(accumswitch)['val2'],
                                  np.array([21, 6, 15, 7]))
    np.testing.assert_array_equal(extr.result_as_pandas(accumfilter)['val2'],
                                  np.array([21, 15]))
    np.testing.assert_array_equal(extr.result_as_pandas(accum_test)['char'],
                                  np.array(["A", "B", "C"]))

    df = df.reset_index()
    df['binstring_col'] = df['binstring_col'].str.decode(encoding='UTF-8')

    outdf = extr.result_as_pandas(accumstr)

    pd.testing.assert_frame_equal(outdf, df, check_dtype=False)
