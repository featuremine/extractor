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
@date 23 Apr 2020
@brief File contains extractor python sample
"""

from ctypes import *
from datetime import timedelta
import extractor as extr
import numpy as np
import subprocess
import threading
import time
import os
import pandas as pd


def clbck(frame):
    print(frame)


if __name__ == "__main__":

    graph = extr.system.comp_graph()

    host = "10.185.130.129"
    user = "qsemble"
    srvname = "ELEKTRON_EDGE"
    name = "IBM.N"

    IBM = graph.features.fmtron(host, user, srvname, name, (
        ("BID", 22, extr.Float64),
        ("ASK", 25, extr.Float64),
        ("BIDSIZE", 30, extr.Float64),
        ("ASKSIZE", 31, extr.Float64)))

    graph.callback(IBM, clbck)

    ctx = graph.stream_ctx()
    ctx.run_live()
