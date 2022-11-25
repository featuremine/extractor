"""
        COPYRIGHT (c) 2020 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.
"""

import unittest
import extractor as extr
from pandas.testing import assert_frame_equal
import pandas as pd

def run_test(comp, out, dtype, extrtype, *inps, conv=None):

    graph = extr.system.comp_graph()
    op = graph.features

    if conv is not None:
        inps = [[conv(i) for i in inp] for inp in inps]
        out = [conv(o) for o in out]

    pinps = []

    ts = pd.to_datetime(list(range(len(out))), unit='s')

    for inp in inps:
        s = pd.Series(inp, dtype=dtype)
        df = pd.DataFrame(data={"val": s,
                                "receive": ts}).set_index("receive")

        pinp = op.pandas_play(df,
                             (("val", extrtype),))

        pinps.append(pinp)

    outop = getattr(op, comp)(*pinps)

    res = op.accumulate(outop)

    graph.stream_ctx().run()

    outs = pd.Series(out, dtype=dtype)
    outdf = pd.DataFrame(data={"Timestamp": ts, "val": outs})

    pd.testing.assert_frame_equal(extr.result_as_pandas(res), outdf)

def run_tests(comps, dtype, extrtype, *inps, conv=None):
    for comp, out in comps:
        run_test(comp, out, dtype, extrtype, *inps, conv=conv)

class TestExtractorArithmetic(unittest.TestCase):

    def test_basic_arithmetic(self):
        comps = [
            ("add",[3,10,7]),
            ("diff",[-1,-2,5]),
            ("mult",[2,24,6]),
            ("sum",[3,10,7]),
        ]
        inps = [[1, 4, 6],[2,6,1]]
        run_tests(comps, "int32", extr.Int32, *inps)
        run_tests(comps, "int64", extr.Int64, *inps)

        comps = [
            ("add",[4,13,7]),
            ("diff",[0,1,5]),
            ("mult",[4,42,6]),
            ("sum",[4,13,7]),
        ]
        inps = [[2, 7, 6],[2,6,1]]
        run_tests(comps, "uint32", extr.Uint32, *inps)
        run_tests(comps, "uint64", extr.Uint64, *inps)

        comps = [
            ("add",[3.0,17.0,7.0]),
            ("diff",[-1.0,7.0,5.0]),
            ("mult",[2.0,60.0,6.0]),
            ("sum",[3.0,17.0,7.0]),
            ("divide",[0.5,2.4,6.0]),
        ]
        inps = [[1.0, 12.0, 6.0],[2.0,5.0,1.0]]
        run_tests(comps, "float32", extr.Float32, *inps)
        run_tests(comps, "float64", extr.Float64, *inps)
        # run_tests(comps, "float64", extr.Rprice, *inps)
        run_tests(comps, "object", extr.Decimal128, *inps, conv = lambda x: extr.Decimal128(str(x)))

if __name__ == '__main__':
    unittest.main()
