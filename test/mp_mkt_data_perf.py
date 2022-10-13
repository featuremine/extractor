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
import time
import extractor as extr
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()

    beg_ts = time.time()
    data_in = graph.features.mp_play(
        os.path.join(src_dir, "data/sip_small_20171018.base.mp"),
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("type", extr.Char, ""),
         ("bidprice", extr.Decimal64, ""),
         ("askprice", extr.Decimal64, ""),
         ("bidqty", extr.Int32, ""),
         ("askqty", extr.Int32, "")), name="mp_play_0")

    graph.stream_ctx().run()
    end_ts = time.time()

    print("md mkt data took: %f" % (end_ts - beg_ts))
