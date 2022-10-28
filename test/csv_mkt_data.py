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
@package csv_mkt_data.py
@author Maxim Trokhimtchouk
@date 8 Dec 2017
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import filecmp
import sys
import extractor as extr
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()

    data_in = graph.features.csv_play(
        os.path.join(src_dir, "data/sip_small_20171018.csv"),
        (("receive", extr.Time64, ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("type", extr.Char, ""),
         ("bidprice", extr.Decimal64, ""),
         ("askprice", extr.Decimal64, ""),
         ("bidqty", extr.Int32, ""),
         ("askqty", extr.Int32, "")), name="csv_play_0")

    split = graph.features.split(data_in, "market", ("NYSEArca", "NASDAQOMX"), name="split_0")

    graph.features.csv_record(split[0],
                              os.path.join(src_dir, "data/sip_small_NYSEArca_20171018.test.csv"),
                              ("receive", "ticker", "market", "type",
                               "bidprice", "askprice", "bidqty", "askqty"),
                              name="csv_record_0")

    graph.features.csv_record(split[1],
                              os.path.join(src_dir, "data/sip_small_NASDAQOMX_20171018.test.csv"),
                              ("receive", "ticker", "market", "type",
                               "bidprice", "askprice", "bidqty", "askqty"),
                              name="csv_record_1")

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
        name="csv_play_1")

    [id_RNN, id_JGH, id_IL, id_BSMX] = graph.features.split(imb_data_in, "ticker",
                                                            ("RNN", "JGH", "IL", "BSMX"), name="split_1")

    graph.features.csv_record(id_RNN,
                              os.path.join(src_dir, "data/nasdaq_imbalance_open.RNN.20171018.test.csv"),
                              ("ticker", "imbalanceSide", "type", "matchPrice",
                               "matchQty", "imbalanceQuantity", "receive", "Exchange"),
                              name="csv_record_RNN")

    graph.features.csv_record(id_JGH,
                              os.path.join(src_dir, "data/nasdaq_imbalance_open.JGH.20171018.test.csv"),
                              ("ticker", "imbalanceSide", "type", "matchPrice",
                               "matchQty", "imbalanceQuantity", "receive", "Exchange"),
                              name="csv_record_JGH")

    graph.features.csv_record(id_IL,
                              os.path.join(src_dir, "data/nasdaq_imbalance_open.IL.20171018.test.csv"),
                              ("ticker", "imbalanceSide", "type", "matchPrice",
                               "matchQty", "imbalanceQuantity", "receive", "Exchange"),
                              name="csv_record_IL")

    graph.features.csv_record(id_BSMX,
                              os.path.join(src_dir, "data/nasdaq_imbalance_open.BSMX.20171018.test.csv"),
                              ("ticker", "imbalanceSide", "type", "matchPrice",
                               "matchQty", "imbalanceQuantity", "receive", "Exchange"),
                              name="csv_record_BSMX")

    graph.stream_ctx().run()

    extr.flush()

    extr.assert_numdiff(os.path.join(src_dir, 'data/sip_small_NYSEArca_20171018.base.csv'),
                        os.path.join(src_dir, 'data/sip_small_NYSEArca_20171018.test.csv'))

    extr.assert_numdiff(os.path.join(src_dir, 'data/sip_small_NASDAQOMX_20171018.base.csv'),
                        os.path.join(src_dir, 'data/sip_small_NASDAQOMX_20171018.test.csv'))

    extr.assert_numdiff(os.path.join(src_dir, 'data/nasdaq_imbalance_open.RNN.20171018.base.csv'),
                        os.path.join(src_dir, 'data/nasdaq_imbalance_open.RNN.20171018.test.csv'))

    extr.assert_numdiff(os.path.join(src_dir, 'data/nasdaq_imbalance_open.JGH.20171018.base.csv'),
                        os.path.join(src_dir, 'data/nasdaq_imbalance_open.JGH.20171018.test.csv'))

    extr.assert_numdiff(os.path.join(src_dir, 'data/nasdaq_imbalance_open.IL.20171018.base.csv'),
                        os.path.join(src_dir, 'data/nasdaq_imbalance_open.IL.20171018.test.csv'))

    extr.assert_numdiff(os.path.join(src_dir, 'data/nasdaq_imbalance_open.BSMX.20171018.base.csv'),
                        os.path.join(src_dir, 'data/nasdaq_imbalance_open.BSMX.20171018.test.csv'))
