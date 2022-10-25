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
@package result.py
@author Andres Rangel
@date 21 Sep 2018
@brief File contains extractor python sample
"""
import pandas as pd
from pandas.testing import assert_frame_equal
from datetime import datetime, timedelta
import extractor as extr
from extractor import result_as_pandas
import pytz
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":

    graph = extr.system.comp_graph()
    op = graph.features

    in_file = os.path.join(src_dir, "data/pandas_play_file.csv")
    data_in = graph.features.csv_play(in_file,
                                      (("receive", extr.Time64, ""),
                                       ("market", extr.Array(extr.Char, 32), ""),
                                          ("ticker", extr.Array(extr.Char, 16), ""),
                                          ("type", extr.Char, ""),
                                          ("bidprice", extr.Decimal64, ""),
                                          ("askprice", extr.Decimal64, ""),
                                          ("bidqty", extr.Int32, ""),
                                          ("askqty", extr.Int32, "")), name="csv_play_0")

    val_aggr = op.accumulate(data_in)

    ctx = graph.stream_ctx()
    ctx.run()

    as_pd = result_as_pandas(val_aggr)
    as_pd = as_pd.drop('Timestamp', axis=1)
    as_pd = as_pd.sort_index(axis=1)

    from_pd = pd.read_csv(in_file)
    # Timestamps in file are integers, they must be transformed to timestamps
    from_pd['receive'] = pd.to_datetime(from_pd['receive'], unit='ns')
    from_pd = from_pd.sort_index(axis=1)

    assert_frame_equal(as_pd, from_pd, check_dtype=False)
