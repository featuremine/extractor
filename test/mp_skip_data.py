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
@package mp_skip_data.py
@author Andres Rangel
@date 5 Nov 2018
@brief File contains skip test for mp_play
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

    trades_in = graph.features.mp_play(
        os.path.join(src_dir, "data/sip_trades_20171018.mp"),
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("price", extr.Decimal64, ""),
         ("qty", extr.Int32, "")), name="mp_play_1")

    graph.features.csv_record(trades_in,
                              os.path.join(src_dir, 'data/mp_skip_data.test.csv'), name="csv_record_1")

    graph.stream_ctx().run()

    extr.flush()

    extr.assert_base(os.path.join(src_dir, 'data/mp_skip_data.base.csv'),
                     os.path.join(src_dir, 'data/mp_skip_data.test.csv'))
