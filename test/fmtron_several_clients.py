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
@package fmtron_sys.py
@author Andrus Suvalov
@date 13 Apr 2020
@brief File contains extractor python sample
"""

import ctypes
from datetime import timedelta
import extractor as extr
import numpy as np
import subprocess
import threading
import time
import os
import pandas as pd
import threading
import sys

### C API types and functions ###


class fmtron_prov(ctypes.Structure):
    _fields_ = [
        ("th", ctypes.c_void_p),
        ("stop", ctypes.c_bool)]


fmtron_lib = ctypes.cdll.LoadLibrary("libfmtron_prov.so")
fmtron_prov_run_async = fmtron_lib.fmtron_prov_run_async
fmtron_prov_run_async.argtypes = [ctypes.c_char_p]
fmtron_prov_run_async.restype = ctypes.POINTER(fmtron_prov)

fmtron_prov_stop = fmtron_lib.fmtron_prov_stop
fmtron_prov_stop.argtypes = [ctypes.POINTER(fmtron_prov)]
fmtron_prov_stop.restype = None

fmtron_prov_wait = fmtron_lib.fmtron_prov_wait
fmtron_prov_wait.argtypes = [ctypes.POINTER(fmtron_prov)]
fmtron_prov_wait.restype = None

fmtron_prov_del = fmtron_lib.fmtron_prov_del
fmtron_prov_del.argtypes = [ctypes.POINTER(fmtron_prov)]
fmtron_prov_del.restype = None
#################################

src_dir = os.path.dirname(os.path.realpath(__file__))

ibm_index = 0
goog_index = 0


def ibm_callback(frame):
    global ibm_index
    bid = frame.as_pandas()['BID'][0]
    bidsize = frame.as_pandas()['BIDSIZE'][0]
    expected_bid = 39.90 + 0.01 * ibm_index
    expected_bidsize = 9.0 + 1.0 * ibm_index
    assert np.isclose(expected_bid, bid), "Expected value of BID - %5.2f, actual - %5.2f" % (expected_bid, bid)
    assert np.isclose(expected_bidsize,
                      bidsize), "Expected value of BIDSIZE - %5.2f, actual - %5.2f" % (expected_bidsize, bidsize)
    ibm_index += 1


def goog_callback(frame):
    global goog_index
    bid = frame.as_pandas()['BID'][0]
    bidsize = frame.as_pandas()['BIDSIZE'][0]
    expected_bid = 49.90 + 0.01 * goog_index
    expected_bidsize = 9.0 + 1.0 * goog_index
    assert np.isclose(expected_bid, bid), "Expected value of BID - %5.2f, actual - %5.2f" % (expected_bid, bid)
    assert np.isclose(expected_bidsize,
                      bidsize), "Expected value of BIDSIZE - %5.2f, actual - %5.2f" % (expected_bidsize, bidsize)
    goog_index += 1


if __name__ == "__main__":
    extr.system.load_ext("ext_lib", sys.argv[1])

    graph = extr.system.comp_graph()

    config_path = (sys.argv[2]).encode('utf-8')
    prov = fmtron_prov_run_async(ctypes.c_char_p(config_path))

    host = "localhost:14002"
    user = "user"
    srvname = "DIRECT_FEED"
    op = graph.features

    GOOG = op.fmtron(host, user, srvname, "GOOG", (
        ("BID", 22, extr.Float64),
        ("BIDSIZE", 30, extr.Float64)))

    IBM = op.fmtron(host, user, srvname, "IBM.N", (
        ("BID", 22, extr.Float64),
        ("BIDSIZE", 30, extr.Float64)))

    graph.callback(IBM, ibm_callback)
    graph.callback(GOOG, goog_callback)
    ctx = graph.stream_ctx()

    now = extr.Time64.from_seconds(1585037531)
    count = 0

    while True:
        time.sleep(0.1)
        ctx.proc_one(now)
        now = ctx.next_time()
        count += 1
        if count > 100:
            break

    fmtron_prov_stop(prov)
    fmtron_prov_wait(prov)
    fmtron_prov_del(prov)
