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
@package live_batch.py
@author Andres Rangel
@date 13 Mar 2019
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
from collections import namedtuple
import time
import extractor as extr
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

result = namedtuple('result', ['count'])

first_time = None
seconds = None
events = []
did_yield = False
verified = 0


def get_time(x):
    global first_time, seconds, events, did_yield
    if first_time is None:
        first_time = x[0].scheduled
    now = x[0].scheduled
    seconds = (now - first_time).seconds
    if did_yield:
        events.clear()
        did_yield = False
    events.append(result(seconds))
    print(events)
    print('elapsed {0}'.format(seconds))
    if seconds > 20:
        exit()


def yield_times():
    global events, did_yield
    while True:
        print (events)
        did_yield = True
        yield events


def verify(x):
    global verified
    assert verified == x[0].count
    verified = verified + 1


if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()
    op = graph.features

    period = timedelta(seconds=1)
    timer = op.timer(period)

    data_in = op.live_batch(yield_times(), timedelta(seconds=5))
    res = op.tuple_msg(data_in, "result", (("count", extr.Int64),))
    graph.callback(timer, get_time)
    graph.callback(res, verify)

    graph.stream_ctx().run_live()
