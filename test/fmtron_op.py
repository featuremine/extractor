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
@date 29 Apr 2020
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
import sys
import pandas as pd

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

index = 0


def frame_clbck(frame):
    global index
    index += 1


if __name__ == "__main__":
    extr.system.load_ext("ext_lib", sys.argv[1])

    graph = extr.system.comp_graph()

    config_path = (sys.argv[2]).encode('utf-8')
    prov = fmtron_prov_run_async(ctypes.c_char_p(config_path))

    host = "localhost:14002"
    user = "user"
    srvname = "DIRECT_FEED"
    name = "IBM.N"
    data_in = graph.features.fmtron(host, user, srvname, name, (
        ("MSG", 3, extr.Array(extr.Char, 30)),
    ))

    graph.callback(data_in, frame_clbck)
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

    assert index == 1, "Expected value is - 11, actual - %d" % index

    fmtron_prov_stop(prov)
    fmtron_prov_wait(prov)
    fmtron_prov_del(prov)
