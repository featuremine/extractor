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
import numpy as np
import pandas as pd
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    in_file_one = os.path.join(src_dir, "data/logical_op_file_one.csv")
    in_file_two = os.path.join(src_dir, "data/logical_op_file_two.csv")
    out_file_one = os.path.join(src_dir, "data/convert_single_one.test.csv")
    out_file_two = os.path.join(src_dir, "data/convert_single_two.test.csv")
    out_file_float_one = os.path.join(src_dir, "data/convert_float_one.test.csv")
    out_file_float_two = os.path.join(src_dir, "data/convert_float_two.test.csv")

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

    ts = pd.to_datetime([65811153, 3513518183, 6111135331], unit='s')
    s2 = pd.Series(["2", "7", "1"], dtype='str')
    s3 = pd.Series(["-36", "25", "44"], dtype='str')

    df = pd.DataFrame(data={  # Unicode string
        "unsigned": s2,
        "signed": s3,
        # Timestamps
        "Timestamp": ts}).set_index("Timestamp")
    data_in_three = op.pandas_play(
        df,
        (("signed", extr.Array(extr.Char, 10)),
         ("unsigned", extr.Array(extr.Char, 10))))

    added = op.convert(data_in_one.val1, extr.Decimal64) + op.convert(data_in_two.val1, extr.Decimal64)
    addedaggr = op.accumulate(added)
    addedfloat = data_in_one.val1 + data_in_two.val1
    addedfloataggr = op.accumulate(addedfloat)
    signed64fromstr = op.convert(data_in_three.signed, extr.Int64)
    signed32fromstr = op.convert(data_in_three.signed, extr.Int32)
    signed16fromstr = op.convert(data_in_three.signed, extr.Int16)
    signed8fromstr = op.convert(data_in_three.signed, extr.Int8)
    unsigned64fromstr = op.convert(data_in_three.unsigned, extr.Uint64)
    unsigned32fromstr = op.convert(data_in_three.unsigned, extr.Uint32)
    unsigned16fromstr = op.convert(data_in_three.unsigned, extr.Uint16)
    unsigned8fromstr = op.convert(data_in_three.unsigned, extr.Uint8)

    combined = op.combine(signed64fromstr, ("s64",), signed32fromstr, ("s32",), signed16fromstr, ("s16",), signed8fromstr, (
        "s8",), unsigned64fromstr, ("u64",), unsigned32fromstr, ("u32",), unsigned16fromstr, ("u16",), unsigned8fromstr, ("u8",))
    combinedaggr = op.accumulate(combined)

    val_one = op.convert(op.field(data_in_one, "val2"), extr.Bool)
    val_two = op.convert(op.field(data_in_two, "val2"), extr.Bool)

    out_stream_one = op.filter_if(val_one, val_one)
    out_stream_two = op.filter_if(val_two, val_two)

    val_one_float = op.convert(val_one, extr.Float32)
    val_two_float = op.convert(val_two, extr.Float64)

    op.csv_record(out_stream_one, out_file_one)
    op.csv_record(out_stream_two, out_file_two)
    op.csv_record(val_one_float, out_file_float_one)
    op.csv_record(val_two_float, out_file_float_two)

    graph.stream_ctx().run()

    extr.flush()

    af = extr.result_as_pandas(addedfloataggr)
    a = extr.result_as_pandas(addedaggr)
    straggr = extr.result_as_pandas(combinedaggr)
    straggr = straggr.drop(columns=["Timestamp"])

    df = pd.DataFrame(data={"s16": s3.astype("int16"),
                            "s32": s3.astype("int32"),
                            "s64": s3.astype("int64"),
                            "s8": s3.astype("int8"),
                            "u16": s2.astype("uint16"),
                            "u32": s2.astype("uint32"),
                            "u64": s2.astype("uint64"),
                            "u8": s2.astype("uint8")})

    pd.testing.assert_frame_equal(df, straggr)

    np.testing.assert_array_almost_equal(np.array(af['val1']),
                                         np.array(a['val1']))

    extr.assert_numdiff(os.path.join(src_dir, 'data/convert_single_one.base.csv'),
                        os.path.join(src_dir, 'data/convert_single_one.test.csv'))
    extr.assert_numdiff(os.path.join(src_dir, 'data/convert_single_two.base.csv'),
                        os.path.join(src_dir, 'data/convert_single_two.test.csv'))
    extr.assert_numdiff(os.path.join(src_dir, 'data/convert_float_one.base.csv'),
                        os.path.join(src_dir, 'data/convert_float_one.test.csv'))
    extr.assert_numdiff(os.path.join(src_dir, 'data/convert_float_two.base.csv'),
                        os.path.join(src_dir, 'data/convert_float_two.test.csv'))
