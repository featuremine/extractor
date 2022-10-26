"""
        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and pr
        rietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

        """

"""
@package fields.py
@author Andres Rangel
@date 26 Sep 2018
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import pandas as pd
import filecmp
import sys
import extractor as extr
from extractor import result_as_pandas
import pytz
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def epoch_delta(date):
    return date - pytz.timezone("UTC").localize(datetime(1970, 1, 1))


def New_York_time(year, mon, day, h=0, m=0, s=0):
    return epoch_delta(pytz.timezone("America/New_York").
                       localize(datetime(year, mon, day, h, m, s)))


if __name__ == "__main__":
    graph = extr.system.comp_graph()

    data_in = graph.features.csv_play(
        os.path.join(src_dir, "data/sip_trades_20171018.csv"),
        (("receive", extr.Time64, ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("price", extr.Float64, ""),
         ("qty", extr.Float64, "")))

    ticker_split = graph.features.split(data_in, "ticker", tuple(["A", "AA", "BA"]))

    fields = graph.features.fields(ticker_split[1], ("price", "qty"))

    bar_period = timedelta(minutes=5)
    timer = graph.features.timer(bar_period)

    avg_tw = graph.features.average_tw(fields, timer)
    aggr_avg = graph.features.accumulate(avg_tw)
    graph.stream_ctx().run_to(New_York_time(2017, 10, 18, 16))

    as_pd = result_as_pandas(aggr_avg)
    as_pd.to_csv(os.path.join(src_dir, "data/average_tw.test.csv"), index=False)

    extr.flush()

    extr.assert_numdiff(os.path.join(src_dir, 'data/average_tw.base.csv'),
                        os.path.join(src_dir, 'data/average_tw.test.csv'))
