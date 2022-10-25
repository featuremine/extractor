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
@author Leandro Rabindranath Leon
@date 9 Jul 2018
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

    quotes_in = graph.features.mp_play(
        "cat " + os.path.join(src_dir, "data/sip_quotes_20171018.mp |"),
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("bidprice", extr.Decimal64, ""),
         ("askprice", extr.Decimal64, ""),
         ("bidqty", extr.Int32, ""),
         ("askqty", extr.Int32, "")), name="mp_play_0")

    graph.features.csv_record(quotes_in,
                              os.path.join(src_dir, "data/sip_quotes_20171018_play_mp_pipe.test.csv"),
                              ("receive", "ticker", "market", "bidprice",
                               "askprice", "bidqty", "askqty"),
                              name="csv_record_0")

    graph.stream_ctx().run()

    extr.flush()

    extr.assert_numdiff(os.path.join(src_dir, 'data/sip_quotes_20171018_play.base.csv'),
                        os.path.join(src_dir, 'data/sip_quotes_20171018_play_mp_pipe.test.csv'))
