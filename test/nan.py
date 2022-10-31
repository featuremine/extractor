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
@package nan.py
@author Andres Rangel
@date 2 Oct 2018
@brief File contains extractor python sample
"""
import extractor as extr
from extractor import result_as_pandas
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

    nan_data_in = graph.features.nan(data_in)

    ctx = graph.stream_ctx()
    ctx.run()

    in_as_pd = result_as_pandas(data_in)
    print(in_as_pd)
    as_pd = result_as_pandas(nan_data_in)
    print(as_pd)
    if not as_pd.isnull().all().all():
        print("Not all entries are NaN")
        exit(1)
    if not in_as_pd.columns.difference(as_pd.columns).shape == (0,):
        print("Columns dont match in input and mapped nan frames")
        exit(1)
