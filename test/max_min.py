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
@package max_min.py
@author Maxim Trokhimtchouk
@date 22 Apr 2018
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
import pytz
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def epoch_delta(date):
    return date - pytz.timezone("UTC").localize(datetime(1970, 1, 1))


def New_York_time(year, mon, day, h=0, m=0, s=0):
    return epoch_delta(pytz.timezone("America/New_York").
                       localize(datetime(year, mon, day, h, m, s)))


def compute_bar(nbbo, ctrdt):
    resolution = timedelta(seconds=1)
    bar_period = timedelta(minutes=5)
    timer = op.timer(bar_period)
    nbbo_sampled = op.sample(nbbo, timer)
    nbbo_sampled_lagged = op.tick_lag(nbbo_sampled, 1)
    ctrdt_sampled = op.sample(ctrdt, timer)
    ctrdt_sampled_lagged = op.tick_lag(ctrdt_sampled, 1)
    ctrdt_diff = op.diff(ctrdt_sampled, ctrdt_sampled_lagged)
    notional = op.field(ctrdt_diff, "notional")
    shares = op.convert(op.field(ctrdt_diff, "shares"), extr.Float64)
    vwap = op.divide(notional, shares)
    max_ask = op.max(op.field(nbbo, "askprice"), timer)
    min_ask = op.min(op.field(nbbo, "askprice"), timer)
    max_bid = op.max(op.field(nbbo, "bidprice"), timer)
    min_bid = op.min(op.field(nbbo, "bidprice"), timer)
    max_ask_sampled = op.sample(op.tick_lag(max_ask, 1), timer)
    min_ask_sampled = op.sample(op.tick_lag(min_ask, 1), timer)
    max_bid_sampled = op.sample(op.tick_lag(max_bid, 1), timer)
    min_bid_sampled = op.sample(op.tick_lag(min_bid, 1), timer)

    return op.combine(
        nbbo_sampled_lagged,
        (("receive", "start_receive"),
         ("bidprice", "start_bidprice"),
         ("askprice", "start_askprice"),
         ("bidqty", "start_bidqty"),
         ("askqty", "start_askqty")),
        nbbo_sampled,
        (("receive", "end_receive"),
         ("bidprice", "end_bidprice"),
         ("askprice", "end_askprice"),
         ("bidqty", "end_bidqty"),
         ("askqty", "end_askqty")),
        timer, (("actual", "end_time"),),
        ctrdt_diff, tuple(),
        max_ask_sampled, (("askprice", "max_askprice"),),
        min_ask_sampled, (("askprice", "min_askprice"),),
        max_bid_sampled, (("bidprice", "max_bidprice"),),
        min_bid_sampled, (("bidprice", "min_bidprice"),),
        vwap, ("vwap",))


if __name__ == "__main__":
    graph = extr.system.comp_graph()

    bbo_file = os.path.join(src_dir, "data/sip_quotes_20171018.mp")
    trade_file = os.path.join(src_dir, "data/sip_trades_20171018.mp")
    bar_file = os.path.join(src_dir, "data/max_min_bar_20171018_vwap.test.csv")

    markets = ["NYSEMKT", "NASDAQOMX", "NYSEArca"]
    tickers = [
        {"NYSEMKT": "A", "NASDAQOMX": "A", "NYSEArca": "A"},
        {"NYSEMKT": "AA", "NASDAQOMX": "AA", "NYSEArca": "AA"},
        {"NYSEMKT": "BA", "NASDAQOMX": "BA", "NYSEArca": "BA"}
    ]

    op = graph.features
    bbos_in = op.mp_play(
        bbo_file,
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("bidprice", extr.Decimal64, ""),
         ("askprice", extr.Decimal64, ""),
         ("bidqty", extr.Int32, ""),
         ("askqty", extr.Int32, "")))

    bbos_in = op.combine(bbos_in.receive, tuple(),
                                   bbos_in.ticker, tuple(),
                                   bbos_in.market, tuple(),
                                   op.convert(bbos_in.bidprice, extr.Decimal128), tuple(),
                                   op.convert(bbos_in.askprice, extr.Decimal128), tuple(),
                                   op.convert(bbos_in.bidqty, extr.Decimal128), tuple(),
                                   op.convert(bbos_in.askqty, extr.Decimal128), tuple());

    bbo_split = op.split(bbos_in, "market", tuple(markets))

    trades_in = op.mp_play(
        trade_file,
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("price", extr.Decimal64, ""),
         ("qty", extr.Int32, ""),
         ("side", extr.Int32, "")))

    trades_in = op.combine(trades_in.receive, tuple(),
                                   trades_in.ticker, tuple(),
                                   trades_in.market, tuple(),
                                   op.convert(trades_in.price, extr.Decimal128), tuple(),
                                   op.convert(trades_in.qty, extr.Decimal128), tuple(),
                                   op.convert(trades_in.side, extr.Decimal128), tuple());

    trade_split = op.split(trades_in, "market", tuple(markets))

    bbos = []
    ctrds = []
    mkt_idx = 0
    for mkt in markets:
        mkt_tickers = [x[mkt] for x in tickers]
        mkt_bbo_split = op.split(bbo_split[mkt_idx], "ticker", tuple(mkt_tickers))
        mkt_trade_split = op.split(trade_split[mkt_idx], "ticker", tuple(mkt_tickers))
        mkt_bbos = []
        mkt_ctrds = []
        ticker_idx = 0
        for _ in tickers:
            bbo = op.identity(mkt_bbo_split[ticker_idx])
            trade = op.identity(mkt_trade_split[ticker_idx])
            cum_trade = op.cum_trade(trade)
            mkt_bbos.append(bbo)
            mkt_ctrds.append(cum_trade)
            ticker_idx = ticker_idx + 1
        bbos.append(mkt_bbos)
        ctrds.append(mkt_ctrds)
        mkt_idx = mkt_idx + 1

    nbbos = [op.bbo_aggr(*x) for x in zip(*bbos)]
    ctrdts = [op.cum_trade_total(*x) for x in zip(*ctrds)]

    bars = [compute_bar(nbbo, ctrdt) for nbbo, ctrdt in zip(nbbos, ctrdts)]
    out_stream = op.join(*bars, "ticker", extr.Array(extr.Char, 16),
                         tuple([x["NASDAQOMX"] for x in tickers]))

    op.csv_record(out_stream, bar_file)

    graph.stream_ctx().run_to(New_York_time(2017, 10, 18, 16))

    extr.flush()

    extr.assert_numdiff(os.path.join(src_dir, 'data/max_min_bar_20171018_vwap.base.csv'),
                        os.path.join(src_dir, 'data/max_min_bar_20171018_vwap.test.csv'))
