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
@package pandas_play.py
@author Andres Rangel
@date 13 Mar 2019
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
import pytz
import pandas as pd
import numpy as np
import tempfile
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    with tempfile.TemporaryDirectory() as tmp:
        tmp_file = 'tmp_file_pandas_play_test'
        tmp_file_2 = 'tmp_file_pandas_play_test_2'
        graph = extr.system.comp_graph()
        op = graph.features

        ts = pd.to_datetime([65811153, 3513518183, 6111135331], unit='s')
        f32 = pd.Series([514541811.9, 9.999945, 3.4458], dtype='float32')
        f64 = pd.Series([3.64, 0.664, 9.3645], dtype='float64')
        d128 = pd.Series([extr.Decimal128("3.64"), extr.Decimal128("0.664"), extr.Decimal128("9.3645")], dtype='object')
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
            # Decimal128
            "decimal128_col": d128,
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

        data_in_one = op.pandas_play(
            df,
            (("binstring_col", extr.Array(extr.Char, 16)),
             ("bool_col", extr.Bool),
             ("decimal128_col", extr.Decimal128),
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

        empty_frame = pd.DataFrame(
            data={
                "val1": pd.to_datetime(
                    [],
                    unit="s"),
                "val2": [],
                "val3": []}).set_index("val1")

        data_in_two = op.pandas_play(empty_frame, (("val2", extr.Float64),))

        data_in_three = op.pandas_play(
            df,
            (("binstring_col", extr.Array(extr.Char, 16)),
             ("bool_col", extr.Bool),
             ("decimal128_col", extr.Decimal128),
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
        op.csv_record(data_in_three, tmp_file_2)
        op.csv_record(data_in_one, tmp_file)

        f = open(os.path.join(src_dir, "data/callbacks_string_repr.test.log"), 'w')

        def frame_callback(frame):
            f.write("frame_start:\n" + str(frame))
            f.write("\nframe_end\n")

            for idx in range(0, len(frame)):

                sf = frame[idx]
                f.write("sub_frame_start:\n" + str(sf))
                f.write("\nsub_frame_end\n")

        for name, o in graph:
            graph.callback(o, frame_callback)

        graph.stream_ctx().run()

        frame_ref = graph.get_ref(data_in_one)

        f.write("frame_repr:\n" + str(frame_ref) + "\nframe_repr_end\n")
        f.close()
        extr.assert_numdiff(os.path.join(src_dir, "data/callbacks_string_repr.test.log"),
                            os.path.join(src_dir, "data/callbacks_string_repr.base.log"))

        extr.flush()

        df = df.reset_index()

        df = df.drop(columns=["Timestamp"])

        extr_df = pd.read_csv(tmp_file, parse_dates=True, sep=',', header=0,
                              dtype={"binstring_col": "S16",
                                     "bool_col": "bool",
                                     "decimal128_col": "str",
                                     "decimal_from_float32_col": "float32",
                                     "decimal_from_float64_col": "float64",
                                     "float32_col": "float32",
                                     "float64_col": "float64",
                                     "float64_from_float32_col": "float32",
                                     "int16_col": "int16",
                                     "int16_from_int8_col": "int8",
                                     "int32_col": "int32",
                                     "int32_from_int16_col": "int16",
                                     "int32_from_int8_col": "int8",
                                     "int64_col": "int64",

                                     "int64_from_int16_col": "int16",
                                     "int64_from_int32_col": "int32",
                                     "int64_from_int8_col": "int8",
                                     "int8_col": "int8",
                                     "string_col": "str",
                                     "timestamp_col": "int64",
                                     "uint16_col": "uint16", "uint16_from_uint8_col": "uint8",
                                     "uint32_col": "uint32", "uint32_from_uint16_col": "uint16",
                                     "uint32_from_uint8_col": "uint8",
                                     "uint64_col": "uint64", "uint64_from_uint16_col": "uint16",
                                     "uint64_from_uint32_col": "uint32", "uint64_from_uint8_col": "uint8",
                                     "uint8_col": "uint8"},
                                converters= {"decimal128_col": lambda val: extr.Decimal128(val)})

        extr_df["timestamp_col"] = pd.to_datetime(extr_df["timestamp_col"], unit='ns')

        pd.testing.assert_frame_equal(df, extr_df)

        extr_df_2 = pd.read_csv(tmp_file_2, parse_dates=True, sep=',', header=0,
                                dtype={"binstring_col": "S16",
                                       "bool_col": "bool",
                                       "decimal128_col": "str",
                                       "int64_from_int16_col": "int16",
                                       "int64_from_int32_col": "int32",
                                       "int64_from_int8_col": "int8",
                                       "int8_col": "int8",
                                       "string_col": "str",
                                       "timestamp_col": "int64",
                                       "uint16_col": "uint16", "uint16_from_uint8_col": "uint8",
                                       "uint32_col": "uint32", "uint32_from_uint16_col": "uint16",
                                       "uint32_from_uint8_col": "uint8",
                                       "uint64_col": "uint64", "uint64_from_uint16_col": "uint16",
                                       "uint64_from_uint32_col": "uint32", "uint64_from_uint8_col": "uint8",
                                       "uint8_col": "uint8"},
                                converters= {"decimal128_col": lambda val: extr.Decimal128(val)})

        df_2 = df.drop([
            "decimal_from_float32_col",
            "decimal_from_float64_col",
            "float32_col",
            "float64_col",
            "float64_from_float32_col",
            "int16_col",
            "int16_from_int8_col",
            "int32_col",
            "int32_from_int16_col",
            "int32_from_int8_col",
            "int64_col"
        ], axis=1)
        extr_df_2["timestamp_col"] = pd.to_datetime(extr_df["timestamp_col"], unit='ns')
        pd.testing.assert_frame_equal(df_2, extr_df_2)
