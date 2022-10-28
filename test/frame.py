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
@package frame.py
@author Andres Rangel
@date 23 Jan 2019
@brief File contains frame tests
"""

from datetime import datetime, timedelta
import extractor as extr
import pytz
import pandas as pd
import numpy as np
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    ts = pd.to_datetime([31513, 854684, 1536868], unit='s')
    us = pd.Series(["™вгджзклмн", "Mayb♥True", "Maybeзклмн"], dtype='unicode')

    df = pd.DataFrame(data={"Timestamp": ts,
                            # Unicode string
                            "unicode_col": us})

    ddf = df.set_index("Timestamp")

    data = op.pandas_play(ddf, (("unicode_col", extr.Array(extr.Char, 30)),))

    def validate_gen():
        count = 0

        def validate(frame):
            nonlocal count
            np.testing.assert_equal(dir(frame[0]), ['unicode_col'])
            np.testing.assert_equal(frame.as_pandas()['unicode_col'][0], ddf['unicode_col'][count])
            np.testing.assert_equal(frame[0].unicode_col, ddf['unicode_col'][count])
            count = count + 1
        return validate

    graph.callback(data, validate_gen())

    accum = op.accumulate(data)

    graph.stream_ctx().run()

    accum_ref = graph.get_ref(accum)
    data_ref = graph.get_ref(data)

    np.testing.assert_equal(dir(accum_ref[0]), ['Timestamp', 'unicode_col'])
    np.testing.assert_equal(dir(data_ref[0]), ['unicode_col'])
    pd.testing.assert_frame_equal(accum_ref.as_pandas(), df)
