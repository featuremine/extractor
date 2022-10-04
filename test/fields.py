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
@package fields.py
@author Andres Rangel
@date 26 Sep 2018
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

    fields = graph.features.fields(data_in, ("receive", "market", "ticker", "bidprice", "bidqty"))

    graph.features.csv_record(fields,
                              os.path.join(src_dir, "data/fields.test.csv"),
                              ("receive", "market", "ticker", "bidprice",
                               "bidqty"),
                              name="csv_record_0")

    graph.stream_ctx().run()

    extr.flush()

    extr.assert_numdiff(os.path.join(src_dir, 'data/fields.base.csv'), os.path.join(src_dir, 'data/fields.test.csv'))
