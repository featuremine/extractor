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
@package mp_mkt_data.py
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
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()

    quotes_in = graph.features.csv_play(
        os.path.join(src_dir, "data/sip_quotes_20171018.base.csv"),
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("bidprice", extr.Decimal64, ""),
         ("askprice", extr.Decimal64, ""),
         ("bidqty", extr.Int32, ""),
         ("askqty", extr.Int32, "")), name="csv_play_0")

    graph.features.mp_record(quotes_in,
                             os.path.join(src_dir, "data/sip_quotes_20171018_record.test.mp"),
                             ("receive", "ticker", "market", "bidprice",
                              "askprice", "bidqty", "askqty"),
                             name="mp_record_0_0")

    graph.features.mp_record(quotes_in,
                             "| cat > " + os.path.join(src_dir, "data/sip_quotes_20171018_record.pipe_test.mp"),
                             ("receive", "ticker", "market", "bidprice",
                              "askprice", "bidqty", "askqty"),
                             name="mp_record_0_1")

    trade_out = graph.features.csv_play(
        os.path.join(src_dir, "data/sip_trades_20171018.csv"),
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("price", extr.Decimal64, ""),
         ("qty", extr.Int32, ""),
         ("side", extr.Int32, "")), name="csv_play_1")

    graph.features.mp_record(trade_out,
                             os.path.join(src_dir, "data/sip_trades_20171018_record.test.mp"),
                             ("receive", "ticker", "market", "price",
                              "qty", "side"),
                             name="mp_record_1_0")

    graph.features.mp_record(trade_out,
                             "| cat > " + os.path.join(src_dir, "data/sip_trades_20171018_record.pipe_test.mp"),
                             ("receive", "ticker", "market", "price",
                              "qty", "side"),
                             name="mp_record_1_1")

    graph.stream_ctx().run()

    extr.flush()

    extr.assert_base(os.path.join(src_dir, 'data/sip_quotes_20171018_record.base.mp'),
                     os.path.join(src_dir, 'data/sip_quotes_20171018_record.test.mp'))
    extr.assert_base(os.path.join(src_dir, 'data/sip_quotes_20171018_record.base.mp'),
                     os.path.join(src_dir, 'data/sip_quotes_20171018_record.pipe_test.mp'))

    extr.assert_base(os.path.join(src_dir, 'data/sip_trades_20171018_record.base.mp'),
                     os.path.join(src_dir, 'data/sip_trades_20171018_record.test.mp'))
    extr.assert_base(os.path.join(src_dir, 'data/sip_trades_20171018_record.base.mp'),
                     os.path.join(src_dir, 'data/sip_trades_20171018_record.pipe_test.mp'))
