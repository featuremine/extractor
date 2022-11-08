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
@package bars.py
@author Maxim Trokhimtchouk
@date 22 Apr 2018
@brief File contains extractor python sample
"""

import pandas as pd
from pandas.testing import assert_frame_equal
from datetime import datetime, timedelta
import extractor as extr
from extractor import result_as_pandas
import pytz
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def epoch_delta(date):
    return date - pytz.timezone("UTC").localize(datetime(1970, 1, 1))


def New_York_time(year, mon, day, h=0, m=0, s=0):
    return epoch_delta(pytz.timezone("America/New_York").
                       localize(datetime(year, mon, day, h, m, s)))


def quote_side_float64(quote, name):
    return op.cond(op.is_zero(op.field(quote, name)),
                   op.nan(quote),
                   op.convert(quote, extr.Float64))


def quote_float64(quote):
    bid_quote = op.fields(quote, ("bidprice", "bidqty"))
    ask_quote = op.fields(quote, ("askprice", "askqty"))
    return op.combine(quote_side_float64(bid_quote, "bidqty"), tuple(),
                      quote_side_float64(ask_quote, "askqty"), tuple())


def compute_bar(nbbo, trades, ctrdt, name=None):
    bar_period = timedelta(minutes=5)
    close = op.timer(bar_period)
    close = op.perf_timer_start(close, "bars")

    quote = op.fields(nbbo, ("bidprice", "askprice", "bidqty", "askqty"))
    quote_bid = op.field(nbbo, "bidprice")
    quote_ask = op.field(nbbo, "askprice")
    open_quote = op.asof_prev(quote, close)
    close_quote = op.left_lim(quote, close)
    high_quote = op.left_lim(op.asof(quote, op.max(quote_ask, close)), close)
    low_quote = op.left_lim(op.asof(quote, op.min(quote_bid, close)), close)

    tw_quote = op.average_tw(quote_float64(quote), close)
    trade = op.fields(trades, ("price", "qty", "market"))
    trade_px = op.field(trade, "price")
    first_trade = op.first_after(trade, close)
    open_trade = op.last_asof(first_trade, close)
    close_trade = op.last_asof(trade, close)
    high_trade = op.last_asof(op.asof(trade, op.max(trade_px, first_trade)), close)
    low_trade = op.last_asof(op.asof(trade, op.min(trade_px, first_trade)), close)

    ctrdt_sampled = op.left_lim(ctrdt, close)
    ctrdt_sampled_lagged = op.tick_lag(ctrdt_sampled, 1)
    ctrdt_diff = op.diff(ctrdt_sampled, ctrdt_sampled_lagged)

    combined = op.combine(
        open_trade,
        (("price", "open_px"),
         ("qty", "open_sz"),
         ("market", "open_exch")),
        close_trade,
        (("price", "close_px"),
         ("qty", "close_sz"),
         ("market", "close_exch")),
        high_trade,
        (("price", "high_px"),
         ("qty", "high_sz"),
         ("market", "high_exch")),
        low_trade,
        (("price", "low_px"),
         ("qty", "low_sz"),
         ("market", "low_exch")),
        open_quote,
        (("bidprice", "open_bidpx"),
         ("askprice", "open_askpx"),
         ("bidqty", "open_bidsz"),
         ("askqty", "open_asksz")),
        close_quote,
        (("bidprice", "close_bidpx"),
         ("askprice", "close_askpx"),
         ("bidqty", "close_bidsz"),
         ("askqty", "close_asksz")),
        high_quote,
        (("bidprice", "high_bidpx"),
         ("askprice", "high_askpx"),
         ("bidqty", "high_bidsz"),
         ("askqty", "high_asksz")),
        low_quote,
        (("bidprice", "low_bidpx"),
         ("askprice", "low_askpx"),
         ("bidqty", "low_bidsz"),
         ("askqty", "low_asksz")),
        tw_quote,
        (("bidprice", "tw_bidpx"),
         ("askprice", "tw_askpx"),
         ("bidqty", "tw_bidsz"),
         ("askqty", "tw_asksz")),
        ctrdt_diff, tuple(),
        close, (("actual", "close_time"),), name=name)
    combined = op.perf_timer_stop(combined, "bars")
    return combined


if __name__ == "__main__":
    graph = extr.system.comp_graph()

    bbo_file = src_dir + "/data/sip_quotes_20171018.mp"
    trade_file = src_dir + "/data/sip_trades_20171018.mp"

    markets = ["NYSEMKT", "NASDAQOMX", "NYSEArca"]
    good_tickers = [
        {"NYSEMKT": "A", "NASDAQOMX": "A", "NYSEArca": "A"},
        {"NYSEMKT": "AA", "NASDAQOMX": "AA", "NYSEArca": "AA"},
        {"NYSEMKT": "BA", "NASDAQOMX": "BA", "NYSEArca": "BA"}
    ]
    fake_symbols = ["A" + str(i) for i in range(4, 100)]
    fake_tickers = [{"NYSEMKT": sym, "NASDAQOMX": sym, "NYSEArca": sym} for sym in fake_symbols]
    tickers = good_tickers + fake_tickers

    op = graph.features
    bbos_in = op.mp_play(
        bbo_file,
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("bidprice", extr.Rprice, ""),
         ("askprice", extr.Rprice, ""),
         ("bidqty", extr.Int32, ""),
         ("askqty", extr.Int32, "")))

    converted_bbos_in = op.combine(bbos_in.receive, tuple(),
                                   bbos_in.ticker, tuple(),
                                   bbos_in.market, tuple(),
                                   op.convert(bbos_in.bidprice, extr.Decimal128), tuple(),
                                   op.convert(bbos_in.askprice, extr.Decimal128), tuple(),
                                   op.convert(bbos_in.bidqty, extr.Decimal128), tuple(),
                                   op.convert(bbos_in.askqty, extr.Decimal128), tuple());

    bbo_split = op.split(converted_bbos_in, "market", tuple(markets))

    trades_in = op.mp_play(
        trade_file,
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("price", extr.Rprice, ""),
         ("qty", extr.Int32, ""),
         ("side", extr.Int32, "")))

    converted_trades_in = op.combine(trades_in.receive, tuple(),
                                   trades_in.ticker, tuple(),
                                   trades_in.market, tuple(),
                                   op.convert(trades_in.price, extr.Decimal128), tuple(),
                                   op.convert(trades_in.qty, extr.Decimal128), tuple(),
                                   trades_in.side, tuple());

    trade_split = op.split(converted_trades_in, "market", tuple(markets))

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
            bbo = mkt_bbo_split[ticker_idx]
            trade = mkt_trade_split[ticker_idx]
            cum_trade = op.cumulative(op.fields(trade, ("qty", "price")))
            mkt_bbos.append(bbo)
            mkt_ctrds.append(cum_trade)
            ticker_idx = ticker_idx + 1
        bbos.append(mkt_bbos)
        ctrds.append(mkt_ctrds)
        mkt_idx = mkt_idx + 1

    nbbos = [op.bbo_aggr(*x) for x in zip(*bbos)]
    ctrdts = [op.sum(*x) for x in zip(*ctrds)]

    trade_imnt_split = op.split(trades_in, "ticker", tuple([x["NASDAQOMX"] for x in tickers]))

    bars = [compute_bar(nbbo, trd, ctrdt) for nbbo, trd, ctrdt in zip(nbbos, trade_imnt_split, ctrdts)]

    out_stream = op.join(*bars, "ticker", extr.Array(extr.Char, 16),
                         tuple([x["NASDAQOMX"] for x in tickers]))

    out_stream = op.perf_timer_start(out_stream, "accumulate")
    val_aggr = op.accumulate(out_stream)
    val_aggr = op.perf_timer_stop(val_aggr, "accumulate")

    graph.stream_ctx().run_to(New_York_time(2017, 10, 18, 16))

    print("Average time spent in average in bars for instrument", extr.system.sample_value("bars"))
    print("Time spent accumulating", extr.system.sample_value("accumulate"))

    result_as_pandas(val_aggr).to_csv(src_dir + "/data/bar_20171018.test.csv", index=False)

    extr.flush()
    extr.assert_numdiff(src_dir + "/data/bar_20171018.base.csv", src_dir + "/data/bar_20171018.test.csv")
