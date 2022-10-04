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
@package tick_lag.py
@author Maxim Trokhimtchouk
@date 22 Apr 2018
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import filecmp
import sys
import extractor as extr
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()

    imb_data_in = graph.features.csv_play(
        os.path.join(src_dir, "data/nasdaq_imbalance_open.20171018.csv"),
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("imbalanceSide", extr.Int16, ""),
         ("type", extr.Array(extr.Char, 16), ""),
         ("matchPrice", extr.Decimal64, ""),
         ("matchQty", extr.Int32, ""),
         ("imbalanceQuantity", extr.Int32, ""),
         ("Exchange", extr.Array(extr.Char, 32), "")),
        name="csv_play_0")

    tick_lag_out = graph.features.tick_lag(imb_data_in, 10, name="tick_lag_0")

    graph.features.csv_record(tick_lag_out,
                              os.path.join(src_dir, "data/tick_lag.test.csv"),
                              ("ticker", "imbalanceSide", "type", "matchPrice",
                               "matchQty", "imbalanceQuantity", "receive", "Exchange"),
                              name="csv_record_0")

    time_lag_out = graph.features.time_lag(imb_data_in, timedelta(minutes=1),
                                           timedelta(seconds=1), name="time_lag_0")

    graph.features.csv_record(time_lag_out,
                              os.path.join(src_dir, "data/time_lag.test.csv"),
                              ("ticker", "imbalanceSide", "type", "matchPrice",
                               "matchQty", "imbalanceQuantity", "receive", "Exchange"),
                              name="csv_record_1")

    graph.stream_ctx().run()

    extr.flush()

    extr.assert_numdiff(os.path.join(src_dir, 'data/time_lag.base.csv'),
                        os.path.join(src_dir, 'data/time_lag.test.csv'))

    extr.assert_numdiff(os.path.join(src_dir, 'data/tick_lag.base.csv'),
                        os.path.join(src_dir, 'data/tick_lag.test.csv'))
