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
@package ext_test.py
@author Maxim Trokhimtchouk
@date 30 Nov 2017
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
import os
import sys

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    extr.system.load_ext("ext_lib", sys.argv[1])
    #extr.system.load_ext("ext_lib", src_dir + "libext_lib.so")
    graph = extr.system.comp_graph()
    graph.features.timer(timedelta(seconds=5), name='timer1')
    ctx = graph.stream_ctx()
    now = extr.Time64.from_seconds(1400000001)
    counter = 0
    while True:
        ctx.proc_one(now)
        now = ctx.next_time()
        counter = counter + 1
        print(counter)
        if now >= extr.Time64.from_seconds(1400000102):
            break
