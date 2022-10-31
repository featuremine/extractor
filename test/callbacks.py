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
@package callbacks.py
@author Andres Rangel
@date 10 May 2019
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
from extractor import result_as_pandas
import pytz
import pandas as pd
import numpy as np
from numpy.testing import assert_equal
from graph_dump import *
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

ts = pd.to_datetime([65811153, 3513518183, 6111135331], unit='s')
val1 = pd.Series([514541811.9, 9.999945, 3.4458], dtype='float64')
val2 = pd.Series([3.64, 0.664, 9.3645], dtype='float64')
val3 = pd.Series([5233.4, 6.24, 9.554], dtype='float64')
val4 = pd.Series([21.002, 880.948, 0.55], dtype='float64')

df = pd.DataFrame(data={"val1": val1,
                        "val2": val2,
                        "receive": ts,
                        "Timestamp": ts}).set_index("Timestamp")

method_clbck_frames = []
lambda_clbck_frames = []

method_clbck_frames_w_index = []
lambda_clbck_frames_w_index = []


def method_clbck(frame):
    assert_equal(frame.fields(), ["receive", "val1", "val2"])
    global method_clbck_frames
    global method_clbck_frames_w_index
    method_clbck_frames.append(frame.as_pandas())
    method_clbck_frames_w_index.append(frame.as_pandas('receive'))


def fill_graph_out_of_scope(graph):
    global lambda_clbck_frames
    global lambda_clbck_frames_w_index
    op = graph.features
    data_in_two = op.pandas_play(df,
                                 (("receive", extr.Time64),
                                  ("val1", extr.Float64),
                                     ("val2", extr.Float64)))

    graph.callback(
        data_in_two, lambda frame: lambda_clbck_frames.append(
            frame.as_pandas()) or lambda_clbck_frames_w_index.append(
            frame.as_pandas('receive')))


if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    data_in = op.pandas_play(df,
                             (("receive", extr.Time64),
                              ("val1", extr.Float64),
                                 ("val2", extr.Float64)))

    graph.callback(data_in, method_clbck)

    fill_graph_out_of_scope(graph)

    names = []

    for name, o in graph:
        names.append(name)

    assert_equal(names, ["pandas_play_0", "pandas_play_1"])

    data_ref = graph.get_ref(data_in)
    graph_dump(graph, os.path.join(src_dir, "callbacks.test.log"))
    graph.stream_ctx().run()

    assert_equal(data_ref.fields(), ["receive", "val1", "val2"])

    mdf = pd.concat(method_clbck_frames)
    ldf = pd.concat(lambda_clbck_frames)

    pd.testing.assert_frame_equal(mdf, ldf)

    midf = pd.concat(method_clbck_frames_w_index)
    lidf = pd.concat(lambda_clbck_frames_w_index)

    pd.testing.assert_frame_equal(midf, lidf)
    extr.assert_numdiff(os.path.join(src_dir, "callbacks.test.log"), os.path.join(src_dir, "callbacks.base.log"))
