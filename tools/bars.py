"""
        COPYRIGHT (c) 2019 by Featuremine Corporation.
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
@date 23 Mar 2019
@brief File contains extractor python sample
"""

import pandas as pd
from pandas.testing import assert_frame_equal
from datetime import datetime, timedelta
import extractor as extr
from extractor import result_as_pandas
import pytz


def epoch_delta(date):
    return date - pytz.timezone("UTC").localize(datetime(1970, 1, 1))


def New_York_time(year, mon, day, h=0, m=0, s=0):
    return epoch_delta(pytz.timezone("America/New_York").
                       localize(datetime(year, mon, day, h, m, s)))


def quote_side_float64(op, quote, name):
    return op.cond(op.is_zero(op.field(quote, name)),
                   op.nan(quote),
                   op.convert(quote, extr.Float64))


def quote_float64(op, quote):
    bid_quote = op.fields(quote, ("bidprice", "bidqty"))
    ask_quote = op.fields(quote, ("askprice", "askqty"))
    return op.combine(quote_side_float64(op, bid_quote, "bidqty"), tuple(),
                      quote_side_float64(op, ask_quote, "askqty"), tuple())


def compute_bar(op, nbbo, trades, ctrdt, name=None):
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

    tw_quote = op.average_tw(quote_float64(op, quote), close)
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
    return combined


def computation(graph, bbos_in, trades_in, tickers, markets):
    op = graph.features
    bbo_split = op.split(bbos_in, "market", tuple(markets))
    trade_split = op.split(trades_in, "market", tuple(markets))

    bbos = []
    ctrds = []
    for mkt_idx, mkt in enumerate(markets):
        mkt_bbo_split = op.split(bbo_split[mkt_idx], "ticker", tuple(tickers))
        mkt_trade_split = op.split(trade_split[mkt_idx], "ticker", tuple(tickers))
        mkt_bbos = []
        mkt_ctrds = []
        for ticker_idx, _ in enumerate(tickers):
            bbo = mkt_bbo_split[ticker_idx]
            trade = mkt_trade_split[ticker_idx]
            cum_trade = op.cum_trade(trade)
            mkt_bbos.append(bbo)
            mkt_ctrds.append(cum_trade)
        bbos.append(mkt_bbos)
        ctrds.append(mkt_ctrds)

    nbbos = [op.bbo_aggr(*x) for x in zip(*bbos)]
    ctrdts = [op.cum_trade_total(*x) for x in zip(*ctrds)]
    trdimnt = op.split(trades_in, "ticker", tuple(tickers))
    return [compute_bar(op, nbbo, trd, ctrdt) for nbbo, trd, ctrdt in zip(nbbos, trdimnt, ctrdts)]
