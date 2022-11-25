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
import pandas as pd
import math

def run_test(comp, out, dtype, extrtype, *inps, conv=None, outts=None, outdtype=None):

    graph = extr.system.comp_graph()
    op = graph.features

    if conv is not None:
        inps = [[conv(i) for i in inp] for inp in inps]
        if dtype == outdtype:
            out = [conv(o) for o in out]

    pinps = []

    for inp in inps:
        s = pd.Series(inp, dtype=dtype)
        ts = pd.to_datetime(list(range(len(inp))), unit='s')

        df = pd.DataFrame(data={"val": s,
                                "receive": ts}).set_index("receive")

        pinp = op.pandas_play(df,
                             (("val", extrtype),))

        pinps.append(pinp)

    outop = getattr(op, comp)(*pinps)

    res = op.accumulate(outop)

    graph.stream_ctx().run()

    outs = pd.Series(out, dtype=outdtype if outdtype is not None else dtype)
    ts = outts if outts is not None else pd.to_datetime(list(range(len(out))), unit='s')
    outdf = pd.DataFrame(data={"Timestamp": ts, "val": outs})

    pd.testing.assert_frame_equal(extr.result_as_pandas(res), outdf)

def run_tests(comps, dtype, extrtype, *inps, conv=None, outts=None, outdtype=None):
    for comp, out in comps:
        run_test(comp, out, dtype, extrtype, *inps, conv=conv, outts=outts, outdtype=outdtype)

class TestExtractorArithmetic(unittest.TestCase):

    def test_basic_arithmetic(self):
        comps = [
            ("add",[4,13,7]),
            ("diff",[0,1,5]),
            ("mult",[4,42,6]),
            ("sum",[4,13,7]),
        ]
        inps = [[2, 7, 6],[2,6,1]]
        run_tests(comps, "uint32", extr.Uint32, *inps)
        run_tests(comps, "uint64", extr.Uint64, *inps)
        run_tests(comps, "int32", extr.Int32, *inps)
        run_tests(comps, "int64", extr.Int64, *inps)
        run_tests(comps, "float64", extr.Rprice, *inps)
        run_tests(comps, "float32", extr.Float32, *inps)
        run_tests(comps, "float64", extr.Float64, *inps)
        run_tests(comps, "object", extr.Decimal128, *inps, conv = lambda x: extr.Decimal128(str(x)), outdtype="object")

        comps = [
            ("divide",[0.5,2.4,6.0]),
        ]
        inps = [[1.0, 12.0, 6.0],[2.0,5.0,1.0]]
        run_tests(comps, "float32", extr.Float32, *inps)
        run_tests(comps, "float64", extr.Float64, *inps)
        run_tests(comps, "object", extr.Decimal128, *inps, conv = lambda x: extr.Decimal128(str(x)), outdtype="object")

        comps = [
            ("cumulative",[1.0,13.0,19.0,25.0,27.0]),
        ]
        inps = [[1.0, 12.0, 6.0, 6.0, 2.0],]
        run_tests(comps, "int32", extr.Int32, *inps)
        run_tests(comps, "int64", extr.Int64, *inps)
        run_tests(comps, "uint32", extr.Uint32, *inps)
        run_tests(comps, "uint64", extr.Uint64, *inps)
        run_tests(comps, "float32", extr.Float32, *inps)
        run_tests(comps, "float64", extr.Float64, *inps)
        run_tests(comps, "float64", extr.Rprice, *inps)
        run_tests(comps, "object", extr.Decimal128, *inps, conv = lambda x: extr.Decimal128(str(x)), outdtype="object")

    def test_logic_operations(self):

        comps = [
            ("unique",[1.0,12.0,6.0,2.0]),
        ]
        inps = [[1.0, 12.0, 6.0, 6.0, 2.0],]
        outts = pd.to_datetime([0, 1, 2, 4], unit='s')
        run_tests(comps, "int32", extr.Int32, *inps, outts=outts)
        run_tests(comps, "int64", extr.Int64, *inps, outts=outts)
        run_tests(comps, "uint32", extr.Uint32, *inps, outts=outts)
        run_tests(comps, "uint64", extr.Uint64, *inps, outts=outts)
        run_tests(comps, "float32", extr.Float32, *inps, outts=outts)
        run_tests(comps, "float64", extr.Float64, *inps, outts=outts)
        run_tests(comps, "float64", extr.Rprice, *inps, outts=outts)
        run_tests(comps, "object", extr.Decimal128, *inps, conv = lambda x: extr.Decimal128(str(x)), outts=outts, outdtype="object")

        comps = [
            ("is_zero",[False, False, True, False, False, False]),
            ("is_inf",[False, True, False, False, True, False]),
            ("is_nan",[False, False, False, True, False, False]),
        ]
        inps = [[1.0,math.inf,0,math.nan,-math.inf,25.33],]
        run_tests(comps, "float32", extr.Float32, *inps, outdtype='bool')
        run_tests(comps, "float64", extr.Float64, *inps, outdtype='bool')
        run_tests(comps, "object", extr.Decimal128, *inps, conv = lambda x: extr.Decimal128(str(x)), outdtype='bool')

        comps = [
            ("is_zero",[False, True, False]),
        ]
        inps = [[1.0, 0, 25.33],]
        run_tests(comps, "float64", extr.Rprice, *inps, outdtype='bool')

        comps = [
            ("greater",[False,False,True]),
            ("greater_equal",[False,True,True]),
            ("less_equal",[True,True,False]),
            ("less",[True,False,False]),
            ("equal",[False,True,False]),
            ("not_equal",[True,False,True]),
        ]
        inps = [[0, 7, 6],[2,7,1]]
        run_tests(comps, "uint32", extr.Uint32, *inps, outdtype='bool')
        run_tests(comps, "uint64", extr.Uint64, *inps, outdtype='bool')
        run_tests(comps, "int32", extr.Int32, *inps, outdtype='bool')
        run_tests(comps, "int64", extr.Int64, *inps, outdtype='bool')
        run_tests(comps, "float64", extr.Rprice, *inps, outdtype='bool')
        run_tests(comps, "float32", extr.Float32, *inps, outdtype='bool')
        run_tests(comps, "float64", extr.Float64, *inps, outdtype='bool')
        run_tests(comps, "object", extr.Decimal128, *inps, conv = lambda x: extr.Decimal128(str(x)), outdtype="bool")

if __name__ == '__main__':
    unittest.main()

