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
@package tuple_split.py
@author Maxim Trokhimtchouk
@date 21 Feb 2020
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


def generate_data(r, full):
    Point = namedtuple('Point', ['tm', 'type', 'x', 'y'])
    types = ['one', 'two', 'three', 'four']
    count = 0
    time = pd.to_datetime('2017/01/01', format='%Y/%m/%d')
    td = pd.to_timedelta(1, unit='s')
    xval = 0
    yval = 0
    for i in range(r):
        m = count // 4
        if full:
            if m % 5:
                xval = m
            else:
                yval = m
        else:
            xval = None
            yval = None
        yield Point(tm=time, type=types[count % 4], x=m if m % 5 else xval, y=yval if m % 5 else m)
        count = count + 1
        time = time + td


if __name__ == "__main__":

    graph = extr.system.comp_graph()
    op = graph.features

    data_in = op.sim_poll(batch_results(generate_data(100, False), 6), 'tm')

    splits = op.tuple_split(data_in, "type", ('one', 'two', 'three', 'four'))

    points_in = [op.tuple_msg(split, "Point",
                              (("tm", extr.Time64),
                               ("type", extr.Array(extr.Char, 5)),
                               ("x", extr.Int64),
                               ("y", extr.Int64))) for split in splits]

    acc = [op.accumulate(p) for p in points_in]
    graph.stream_ctx().run()

    dt1 = [result_as_pandas(a) for a in acc]
    dt2 = pd.DataFrame(data=list(generate_data(100, True)))
    dt2['Timestamp'] = dt2.tm
    dt2 = dt2[['Timestamp', 'tm', 'type', 'x', 'y']]
    dt2s = [dt2[i:100:4] for i in range(0, 4)]
    for a in dt2s:
        a.set_index(pd.Index(range(0, 25)), inplace=True)
    for a, b in zip(dt1, dt2s):
        assert_frame_equal(a, b)
