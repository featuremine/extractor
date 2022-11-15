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
@package mp_mkt_data_play_pipe_fail.py
@author Andres Rangel
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
        "cat " + os.path.join(src_dir, "data/sip_quotes_20171018.mp") + "&& exit 1 |",
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("bidprice", extr.Rprice, ""),
         ("askprice", extr.Rprice, ""),
         ("bidqty", extr.Int32, ""),
         ("askqty", extr.Int32, "")), name="mp_play_0")

    try:
        graph.stream_ctx().run()
    except RuntimeError:
        exit(0)
    print("A SystemError was not produced by the context. The test will fail.")
    exit(1)
