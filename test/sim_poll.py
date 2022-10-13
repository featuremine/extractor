"""
        COPYRIGHT (c) 2019 by Featuremine Corporation.
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
@package sim_play.py
@author Maxim Trokhimtchouk
@date 23 Apr 2019
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
from collections import namedtuple
import pandas as pd
from pandas.testing import assert_frame_equal
import time
import extractor as extr
from extractor import result_as_pandas
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def next_n(iter, count):
    for n in range(count):
        try:
            yield next(iter)
        except StopIteration as e:
            return


def batch_results(iter, size):
    while True:
        items = list(next_n(iter, size))
        if not items:
            break
        yield items


def generate_data(r):
    Point = namedtuple('Point', ['tm', 'x', 'y', 'z_ns'])
    count = 0
    time = pd.to_datetime('2017/01/01', format='%Y/%m/%d')
    td = pd.to_timedelta(1, unit='s')
    for i in range(r):
        yield Point(tm=time, x=count, y=count, z_ns=time)
        count = count + 1
        time = time + td


def generate_nanos_data(r):
    Point = namedtuple('Point', ['tm', 'x', 'y', 'z_ns'])
    count = 0
    time = pd.to_datetime('2017/01/01', format='%Y/%m/%d').to_datetime64().astype(int).item()
    td = 1000000000
    for i in range(r):
        assert isinstance(time, int)
        yield Point(tm=time, x=count, y=count, z_ns=time)
        count = count + 1
        time = time + td


if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()
    op = graph.features

    data_in = op.sim_poll(batch_results(generate_data(99), 6), 'tm')
    data_nanos_in = op.sim_poll(batch_results(generate_nanos_data(99), 6), 'tm')

    points_in = op.tuple_msg(data_in, "Point",
                             (("tm", extr.Time64),
                              ("x", extr.Int64),
                                 ("y", extr.Int64),
                                 ("z_ns", extr.Time64)))

    acc = op.accumulate(points_in)

    points_nanos_in = op.tuple_msg(data_nanos_in, "Point",
                                   (("tm", extr.Time64),
                                    ("x", extr.Int64),
                                       ("y", extr.Int64),
                                       ("z_ns", extr.Time64)))

    acc_nanos = op.accumulate(points_nanos_in)

    graph.stream_ctx().run()

    dt1 = result_as_pandas(acc)
    dt1_nanos = result_as_pandas(acc_nanos)
    dt2 = pd.DataFrame(data=list(generate_data(99)))
    dt2['Timestamp'] = dt2.tm
    dt2 = dt2[['Timestamp', 'tm', 'x', 'y', 'z_ns']]
    assert_frame_equal(dt1, dt2)
    assert_frame_equal(dt1_nanos, dt2)
