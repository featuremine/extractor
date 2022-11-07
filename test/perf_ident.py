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
@package perf_ident.py
@author Andres Rangel
@date 22 Apr 2018
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
import pytz
import os

# obtain full path where the file is located
# then obtain the path to the dir from that full path
src_dir = os.path.dirname(os.path.realpath(__file__))


def epoch_delta(date):
    return date - pytz.timezone("UTC").localize(datetime(1970, 1, 1))


def New_York_time(year, mon, day, h=0, m=0, s=0):
    return epoch_delta(pytz.timezone("America/New_York").
                       localize(datetime(year, mon, day, h, m, s)))


if __name__ == "__main__":
    graph = extr.system.comp_graph()

    op = graph.features

    trade_file = src_dir + "/data/sip_trades_20171018.mp"

    trades_in = op.mp_play(
        trade_file,
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("price", extr.Rprice, ""),
         ("qty", extr.Int32, ""),
         ("side", extr.Int32, "")))

    out_stream = op.perf_timer_start(trades_in, "ident_batch")

    for i in range(0, 1000):
        out_stream = op.identity(out_stream)

    out_stream = op.perf_timer_stop(out_stream, "ident_batch")

    graph.stream_ctx().run_to(New_York_time(2017, 10, 18, 16))

    print("Time spent on identity operators", extr.system.sample_value("ident_batch"))
