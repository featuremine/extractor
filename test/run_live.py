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
@package run_live.py
@author Federico Ravchina
@date 22 Nov 2021
@brief File contains extractor python sample
"""


import extractor as extr
import os
from extractor import result_as_pandas
import numpy as np
import pandas as pd
from time import time


def time_ms():
    return int(time() * 1000)


src_dir = os.path.dirname(os.path.realpath(__file__))


def run_live_test(
        in_data_array=[],
        expected_output=[],
        total_time=0,
        subject=None,
):

    graph = extr.system.comp_graph()
    op = graph.features

    now = time_ms()

    data = {}
    values = []

    for key in in_data_array[0].keys():
        if key == 'Timestamp':
            data[key] = pd.to_datetime([x[key] + now for x in in_data_array], unit='ms')
        elif key == 'time':
            data[key] = pd.to_datetime([x[key] + now for x in in_data_array], unit='ms')
            values.append((key, extr.Time64))
        else:
            data[key] = pd.Series([x[key] for x in in_data_array], dtype='int64')
            values.append((key, extr.Int64))

    data_in = op.pandas_play(
        pd.DataFrame(
            data=data).set_index("Timestamp"),
        tuple(values)
    )

    stop_event = op.pandas_play(
        pd.DataFrame(
            data={
                "val1": pd.Series([0], dtype='int64'),
                "Timestamp": pd.to_datetime([now + total_time], unit='ms'),
            }).set_index("Timestamp"),
        (
            ("val1", extr.Int64),
        )
    )

    subject_output = subject(op, data_in)

    subject_acc = op.accumulate(subject_output)
    data_in_acc = op.accumulate(data_in)

    def stop_cb(f):
        raise RuntimeError('stopping context intentionally from test')

    graph.callback(stop_event, stop_cb)

    try:
        graph.stream_ctx().run_live()
    except RuntimeError as e:
        assert len(e.args) == 1
        assert e.args[0] == 'stopping context intentionally from test'

    extr.flush()

    subject_pd = result_as_pandas(subject_acc)
    data_in_pd = result_as_pandas(data_in_acc)

    offset = data_in_pd['Timestamp'][0].to_pydatetime().timestamp()

    for key in expected_output[0].keys():
        if key == 'Timestamp':
            np.testing.assert_allclose(
                [x.to_pydatetime().timestamp() - offset for x in subject_pd['Timestamp']],
                [x['Timestamp'] for x in expected_output],
                rtol=0.00,
                atol=0.05)
        elif key == 'time' or key == 'heartbeat':
            np.testing.assert_allclose(
                [x.to_pydatetime().timestamp() - offset for x in subject_pd[key]],
                [x[key] for x in expected_output],
                rtol=0.00,
                atol=0.05)
        else:
            np.testing.assert_array_equal(
                [x for x in subject_pd[key]],
                [x[key] for x in expected_output])
