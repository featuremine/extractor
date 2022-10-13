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
@package csv_tail.py
@author Andrus Suvalov
@date 24 Mar 2020
@brief File contains extractor python sample
"""

from datetime import timedelta
import extractor as extr
import subprocess
import threading
import time
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

index = 0


def test_frame(frame):
    global index
    val1 = frame.as_pandas()['val1'][0]
    val2 = frame.as_pandas()['val2'][0]
    msg = frame.as_pandas()['message'][0]

    expected_msg = "Test string %d" % (index)

    assert val1 == index, "Expected value in %d row in 'val1' column - %d, actual - %d" % (index, val1)
    assert val2 == index, "Expected value in %d row in column 'val2' - %d, actual - %d" % (index, val2)
    assert msg == expected_msg, "Expected value in %d row in 'message' column - %s, actual - %d" % (expected_msg, msg)

    index += 1


if __name__ == "__main__":
    extr.set_license(src_dir + "/test.lic")

    graph = extr.system.comp_graph()
    data_in = graph.features.csv_tail(
        str("bash " + src_dir + "/csv_tail_pipe.sh |"),
        timedelta(seconds=1),
        (("val1", extr.Int32, ""),
         ("val2", extr.Int32, ""),
         ("message", extr.Array(extr.Char, 128), "")))

    graph.callback(data_in, test_frame)
    ctx = graph.stream_ctx()

    now = extr.Time64.from_seconds(1585037531)
    end = extr.Time64.from_seconds(1585037631)

    while True:
        time.sleep(0.1)
        ctx.proc_one(now)
        now = ctx.next_time()
        if now > end:
            break
    assert index == 10, "Expected row amount is %d, actual is %d" % (10, index)
