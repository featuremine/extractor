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
stop_event = threading.Event()
path = src_dir + "/data/csv_tail_file.csv"


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


def run_bash():
    proc = subprocess.Popen(["bash", src_dir + "/csv_tail.sh", src_dir + "/data/csv_tail_file.csv"])
    proc.wait()


def append_lines():
    global stop_event
    global path
    run_bash()  # run bash script to append new lines
    for i in range(10, 20):
        f = open(path, "a+")
        f.write("%d,%d,Test string %d\n" % (i, i, i))
        f.close()
        time.sleep(0.1)
    stop_event.set()


def remove_test_data():
    if (os.path.exists(path)):
        os.remove(path)


if __name__ == "__main__":

    remove_test_data()
    with open(path, "w") as f:
        f.write("val1,val2,message\n")

    graph = extr.system.comp_graph()
    data_in = graph.features.csv_tail(
        path, timedelta(seconds=1),
        (("val1", extr.Int32, ""),
         ("val2", extr.Int32, ""),
         ("message", extr.Array(extr.Char, 128), "")))

    graph.callback(data_in, test_frame)
    ctx = graph.stream_ctx()

    th = threading.Thread(target=append_lines)
    th.start()

    now = extr.Time64.from_seconds(1585037531)
    while True:
        event_set = stop_event.wait(0.1)
        ctx.proc_one(now)
        now = ctx.next_time()
        if event_set:
            break

    th.join()

    if index != 20:
        with open(path, "r") as f:
            context = f.read()
        remove_test_data()
        assert index == 20, "Expected row amount is %d, actual is %d.\nFile context:\n%s" % (20, index, context)

    remove_test_data()
