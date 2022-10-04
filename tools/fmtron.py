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
 @file /tools/extractor/fmtron.py
 @author Andrus Suvalov
 @date 24 Apr 2020
"""

import sys
import argparse
import extractor as extr
import numpy as np

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--host',
        default='10.185.130.129',
        help="Ip address and port (defaults to '10.185.130.129' if not provided)")
    parser.add_argument(
        '--user',
        default='qsemble',
        help="User name for credentials (defaults to 'qsemble' if not provided)")
    parser.add_argument(
        '--srvname',
        default='ELEKTRON_EDGE',
        help="Service name (defaults to 'ELEKTRON_EDGE' if not provided)")
    parser.add_argument(
        '--imnts',
        default=['1COV.DE', 'A2.MI', 'AA4.MI'],
        nargs='*',
        help="Instruments markets specified in format instrument:market (default to '['1COV.DE','A2.MI','AA4.MI']' if not provided)")
    parser.add_argument(
        '--license',
        default="../test/test.lic",
        help="Extractor license (defaults to '../test/test.lic' if not provided)")
    args = parser.parse_args()

    extr.set_license(args.license)
    graph = extr.system.comp_graph()
    op = graph.features

    for imnt in args.imnts:
        imnt_mkt = imnt.split('.')
        fmtrn_bbo = op.fmtron(args.host, args.user, args.srvname, imnt,
                              (("date", 16, extr.Time64),
                               ("time", 1025, extr.Time64),
                               ("bidprice", 22, extr.Float64),
                               ("askprice", 25, extr.Float64),
                               ("bidqty", 30, extr.Float64),
                               ("askqty", 31, extr.Float64)))

        bbo_date = op.field(fmtrn_bbo, "date")
        bbo_time = op.field(fmtrn_bbo, "time")
        bbo_receive = op.add(bbo_date, bbo_time)
        bbo_in = op.combine(fmtrn_bbo, (("bidprice", "bidprice"),
                                        ("askprice", "askprice"),
                                        ("bidqty", "bidqty"),
                                        ("askqty", "askqty"),),
                            bbo_receive, (("receive",)),
                            name="bbo/{0}/{1}".format(imnt_mkt[1], imnt_mkt[0]))

        fmtrn_trade = op.fmtron(args.host, args.user, args.srvname, imnt,
                                (("date", 16, extr.Time64),
                                 ("time", 18, extr.Time64),
                                 ("price", 6, extr.Float64),
                                 ("qty", 178, extr.Float64)))
        trade_date = op.field(fmtrn_trade, "date")
        trade_time = op.field(fmtrn_trade, "time")
        trade_receive = op.add(trade_date, trade_time)
        side = op.constant(("side", extr.Int32, extr.Int32(0)))
        trade_in = op.combine(fmtrn_trade, (("price", "price"),
                                            ("qty", "qty"),),
                              side, tuple(),
                              trade_receive, (("receive",)),
                              name="trade/{0}/{1}".format(imnt_mkt[1], imnt_mkt[0]))

        def gen_frame_clbck(imnt, mkt):
            def frame_clbk(frame):
                print("{0}.{1}".format(imnt, mkt))
                print(frame)
            return frame_clbk

        graph.callback(bbo_in, gen_frame_clbck(imnt_mkt[0], imnt_mkt[1]))
        graph.callback(trade_in, gen_frame_clbck(imnt_mkt[0], imnt_mkt[1]))

    ctx = graph.stream_ctx()
    ctx.run_live()
