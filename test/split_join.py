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
@package split_join.py
@author Maxim Trokhimtchouk
@date 22 May 2018
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr

if __name__ == "__main__":
    extr.set_license("../test/test.lic")
    graph = extr.system.comp_graph()
    op = graph.features

    trades_in = graph.features.mp_play(
        "trade_20161003.mp",
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("price", extr.Decimal64, ""),
         ("qty", extr.Int32, ""),
         ("side", extr.Int32, "")), name="mp_play_1")

    TQ_split = op.split(trades_in, "market", ("T", "Q"))

    T_split = op.combine(TQ_split,
                         (("receive", "receive"),
                          ("ticker", "ticker"),
                             ("price", "price"),
                             ("qty", "qty"),
                             ("side", "side")))

    Q_split = op.combine(TQ_split,
                         (("receive", "receive"),
                          ("ticker", "ticker"),
                             ("price", "price"),
                             ("qty", "qty"),
                             ("side", "side")))

    trade = op.join(Q_split, T_split, "market",
                    extr.Array(extr.Char, 16),
                    ("NASDAQ", "NASDAQ"))

    graph.features.csv_record(trade,
                              "trade_20161003.test.csv",
                              ("receive", "ticker", "market", "price",
                               "qty", "side"),
                              name="csv_record_1")

    graph.stream_ctx().run()
