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
@package substr.py
@author Andres Rangel
@date 14 Oct 2019
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

    graph = extr.system.comp_graph()
    op = graph.features

    ts = pd.to_datetime([65811153, 3513518183, 6111135331], unit='s')
    s = pd.Series(["MaybeFalse", "MaybeTrue", "MaybeFalse"], dtype='str')
    s2 = pd.Series(["False", "True", "False"], dtype='str')
    res = pd.Series(["Maybe", "Maybe", "Maybe"], dtype='str')

    df = pd.DataFrame(data={  # Unicode string
        "string_col": s,
        # Timestamps
        "ts": ts,
        # Timestamps
        "Timestamp": ts}).set_index("Timestamp")

    df_two = pd.DataFrame(data={  # Unicode string
        "string_col": s,
        # Timestamps
        "smaller_string_col": s2,
        # Timestamps
        "Timestamp": ts}).set_index("Timestamp")

    res_df = pd.DataFrame(data={"string_col": res})
    res_df_two = pd.DataFrame(data={"smaller_string_col": s2, "string_col": res})

    data_in_one = op.pandas_play(
        df,
        (("string_col", extr.Array(extr.Char, 10)),
         ("ts", extr.Time64)))

    data_in_two = op.pandas_play(
        df_two,
        (("string_col", extr.Array(extr.Char, 10)),
         ("smaller_string_col", extr.Array(extr.Char, 5))))

    data_in_three = op.constant(("string_col", extr.Array(extr.Char, 10), "0123456789"))

    try:
        # non string column in frame
        data_out_zero = op.substr(data_in_one, -10, -6)
    except Exception as e:
        pass
    else:
        exit(1)

    try:
        # bad index value
        data_out_zero = op.substr(data_in_one.string_col, -20, -6)
    except Exception as e:
        pass
    else:
        exit(1)

    try:
        # inverted index
        data_out_zero = op.substr(data_in_one.string_col, -6, -10)
    except Exception as e:
        pass
    else:
        exit(1)

    data_out_one = op.substr(data_in_one.string_col, 0, 4)

    data_out_two = op.substr(data_in_one.string_col, -10, -6)

    data_out_three = op.substr(data_in_two, 0, 4)

    accum_out_one = op.accumulate(data_out_one)

    accum_out_two = op.accumulate(data_out_two)

    accum_out_three = op.accumulate(data_out_three)

    graph.stream_ctx().run()

    extr_df = extr.result_as_pandas(accum_out_one)

    extr_df = extr_df.drop(columns=["Timestamp"])

    pd.testing.assert_frame_equal(res_df, extr_df)

    extr_df = extr.result_as_pandas(accum_out_two)

    extr_df = extr_df.drop(columns=["Timestamp"])

    pd.testing.assert_frame_equal(res_df, extr_df)

    extr_df = extr.result_as_pandas(accum_out_three)

    extr_df = extr_df.drop(columns=["Timestamp"])

    pd.testing.assert_frame_equal(res_df_two, extr_df)
