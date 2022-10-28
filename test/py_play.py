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
@package py_play.py
@author Andres Rangel
@date 13 Mar 2019
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
import pytz
import pandas as pd
import numpy as np
import time


def data_feed():
    count = 0
    prev = datetime.now()
    delay = timedelta(seconds=1)
    while(count < 10):
        now = datetime.now()
        if (now - prev > delay):
            count = count + 1
            prev = now
            yield pd.DataFrame({"col1": [1, 2, 3],
                                "other_name": [b"89", b"cs9", b"8adada"],
                                "third_thing": [9.33, 8.994, 7.21543]},
                               index=pd.to_datetime([65811153, 3513518183, 6111135331], unit='s', utc=True))
        else:
            yield None


if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    data_in_one = op.immediate_play(
        data_feed(),
        (("other_name", extr.Array(extr.Char, 16)),
         ("third_thing", extr.Decimal64),
         ("col1", extr.Uint64)),
        timedelta(milliseconds=10))

    op.csv_record(data_in_one, '/dev/stdout')

    graph.stream_ctx().run()

    extr.flush()
