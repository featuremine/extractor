"""
        COPYRIGHT (c) 2019-2023 by Featuremine Corporation.
        
        This Source Code Form is subject to the terms of the Mozilla Public
        License, v. 2.0. If a copy of the MPL was not distributed with this
        file, You can obtain one at https://mozilla.org/MPL/2.0/.
"""

import argparse
import extractor
from datetime import timedelta, datetime, timezone

class TradePlotter:
    def __init__(self, file, chan, count, callback):
        self.count = count
        self.bid = None
        self.ask = None
        self.chan = chan
        self.file = file
        self.callback = callback
    def times(self):
        return self.tms
    def trade(self, tm, px, qt, isbid):
        if self.bid is None or self.ask is None:
            return
        tm = datetime(1970, 1, 1, tzinfo=timezone.utc) + tm
        print(f"{self.chan},{tm},{self.bid},{self.ask},{px},{qt},{isbid}", file=self.file)
        self.count[0] -= 1
        if (self.count[0] == 0):
            self.callback(self)
    def quote(self, bid, ask):
        self.bid = bid
        self.ask = ask

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("--ytp-file", help="ytp file name", required=True)
    parser.add_argument("--base", help="test base file name", required=True)
    args = parser.parse_args()

    base_file = args.base.format("base")
    test_file = args.base.format("test")

    graph = extractor.system.comp_graph()
    op = graph.features

    securities = ["btcusdt", "ethusdt", "busdusdt", "xrpusdt"]
    channels = tuple([f"ore/binance/{sec}" for sec in securities])
    upds = op.seq_ore_live_split(args.ytp_file, channels)
    headers = [op.book_header(upd) for upd in upds]

    levels = [op.book_build(upd, 1) for upd in upds]
    lvlhdrs = [op.asof(hdr, lvl) for hdr, lvl in zip(headers, levels)]
    bbos = [op.combine(
        level, (
            ("bid_prx_0", "bidprice"),
            ("ask_prx_0", "askprice"),
            ("bid_shr_0", "bidqty"),
            ("ask_shr_0", "askqty")
        ),
        hdr, (("receive", "receive"),)) for level, hdr in zip(levels, lvlhdrs)]

    const_b = op.constant(('decoration', extractor.Array(extractor.Char, 4), 'b'))
    const_a = op.constant(('decoration', extractor.Array(extractor.Char, 4), 'a'))
    const_u = op.constant(('decoration', extractor.Array(extractor.Char, 4), 'u'))
    def normalize_trade(trd):
        decoration_equals_a = trd.decoration == const_a
        decoration_equals_b = trd.decoration == const_b
        const_n_value_b = op.constant(('side', extractor.Int32, 0))
        const_n_value_a = op.constant(('side', extractor.Int32, 1))
        const_n_value_u = op.constant(('side', extractor.Int32, 2))
        eq_b = op.cond(decoration_equals_b, const_n_value_b, const_n_value_u)
        side = op.cond(decoration_equals_a, const_n_value_a, eq_b)

        return op.combine(
            trd, (
                ("trade_price", "price"),
                ("receive", "receive"),
                ("qty", "qty")
            ),
            side, (("side", "side"),))

    trades = [normalize_trade(op.book_trades(upd)) for upd in upds]

    count = [5770]
    file = open(test_file, "w")
    print("channel,time,bidpx,askpx,trdpx,trdqt,isbid", file=file)

    def draw_plot(data):
        file.close()
        extractor.assert_base(base_file, test_file)
        exit()
    def add_callback(sec, trd, mkt):
        plotter = TradePlotter(file, sec, count, draw_plot)
        graph.callback(trd, lambda ev: plotter.trade(ev[0].receive, ev[0].price, ev[0].qty, ev[0].side))
        graph.callback(mkt, lambda ev: plotter.quote(ev[0].bidprice, ev[0].askprice))
    # Then subscribe to message callbacks.
    for security, trade, bbo in zip(securities, trades, bbos):
        add_callback(security, trade, bbo)

    graph.stream_ctx().run_live()

