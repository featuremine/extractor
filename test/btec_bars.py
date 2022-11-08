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
@date 13 Dec 2018
@brief File contains extractor python example of generating bars
"""

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
    float_quote = op.convert(quote, extr.Float64)
    return op.cond(op.is_zero(op.field(float_quote, name)),
                   op.nan(float_quote), float_quote)


def quote_float64(quote):
    bid_quote = op.fields(quote, ("bidprice", "bidqty"))
    ask_quote = op.fields(quote, ("askprice", "askqty"))
    return op.combine(quote_side_float64(bid_quote, "bidqty"), tuple(),
                      quote_side_float64(ask_quote, "askqty"), tuple())


def quote_count(nbbo, close):
    counter = op.count(op.unique(nbbo))
    return op.delta(op.left_lim(counter, close), close)


def compute_bar(nbbo, trades, ctrdt):
    # timer event
    bar_period = timedelta(seconds=15)
    close = op.timer(bar_period)

    # quote event
    quote = quote_float64(op.fields(nbbo, ("bidprice", "askprice", "bidqty", "askqty")))
    quote_bid = op.field(quote, "bidprice")
    quote_ask = op.field(quote, "askprice")
    open_quote = op.asof_prev(quote, close)
    close_quote = op.left_lim(quote, close)
    high_quote = op.left_lim(op.asof(quote, op.max(quote_ask, close)), close)
    low_quote = op.left_lim(op.asof(quote, op.min(quote_bid, close)), close)
    tw_quote = op.average_tw(quote, close)

    # quote_counter
    bid_count = quote_count(op.fields(nbbo, ("bidprice", "bidqty")), close)
    ask_count = quote_count(op.fields(nbbo, ("askprice", "askqty")), close)

    spread = op.diff(quote_ask, quote_bid)
    open_spread = op.asof_prev(spread, close)
    close_spread = op.left_lim(spread, close)
    max_spread = op.left_lim(op.max(spread, close), close)
    min_spread = op.left_lim(op.min(spread, close), close)
    tw_spread = op.average_tw(spread, close)

    trade = op.fields(trades, ("price", "qty"))
    trade_px = op.field(trade, "price")
    first_trade = op.first_after(trade, close)
    open_trade = op.last_asof(first_trade, close)
    close_trade = op.last_asof(trade, close)
    high_trade = op.last_asof(op.asof(trade, op.max(trade_px, first_trade)), close)
    low_trade = op.last_asof(op.asof(trade, op.min(trade_px, first_trade)), close)

    ctrdt_sampled = op.left_lim(ctrdt, close)
    ctrdt_sampled_lagged = op.tick_lag(ctrdt_sampled, 1)
    ctrdt_diff = op.diff(ctrdt_sampled, ctrdt_sampled_lagged)

    return op.combine(
        open_trade,
        (("price", "open_px"),
         ("qty", "open_sz")),
        close_trade,
        (("price", "close_px"),
         ("qty", "close_sz")),
        high_trade,
        (("price", "high_px"),
         ("qty", "high_sz")),
        low_trade,
        (("price", "low_px"),
         ("qty", "low_sz")),
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
        open_spread,
        (('askprice', 'open_spread'),),
        close_spread,
        (('askprice', 'close_spread'),),
        max_spread,
        (('askprice', 'max_spread'),),
        min_spread,
        (('askprice', 'min_spread'),),
        tw_spread,
        (('askprice', 'tw_spread'),),
        ctrdt_diff,
        (("notional", "trded_notional"),
         ("shares", "lots")),
        bid_count,
        (("count", "bid_update_count"),),
        ask_count,
        (("count", "ask_update_count"),),
        close,
        (("actual", "close_time"),)
    )


if __name__ == "__main__":
    graph = extr.system.comp_graph()

    bbo_file = str(src_dir + "/data/book.itch.20150901.csv")
    trade_file = str(src_dir + "/data/trades.itch.20150901.csv")

    tickers = [str(x) + '_YEAR' for x in [2, 3, 5, 7, 10, 30]]

    op = graph.features

    bbos_in = op.csv_play(bbo_file,
                          (("time", extr.Time64, ""),
                           ("ticker", extr.Array(extr.Char, 16), ""),
                              ("bid_prx_0", extr.Rprice, ""),
                              ("ask_prx_0", extr.Rprice, ""),
                              ("bid_shr_0", extr.Int32, ""),
                              ("ask_shr_0", extr.Int32, "")))

    bbos_in = op.combine(
        bbos_in,
        (('time', 'receive'),
         ('ticker', 'ticker'),
         ('bid_prx_0', 'bidprice'),
         ('ask_prx_0', 'askprice'),
         ('bid_shr_0', 'bidqty'),
         ('ask_shr_0', 'askqty')
         )
    )

    bbos = op.split(bbos_in, "ticker", tuple(tickers))

    trades_in = op.csv_play(trade_file,
                            (("time", extr.Time64, ""),
                             ("ticker", extr.Array(extr.Char, 16), ""),
                                ("price", extr.Rprice, ""),
                                ("size", extr.Int32, "")))

    trades_in = op.combine(
        trades_in,
        (('time', 'receive'),
         ('ticker', 'ticker')),
        op.convert(trades_in.size, extr.Decimal128),
        (('size', 'qty'),),
        op.convert(trades_in.price, extr.Decimal128),
        tuple()
    )

    trade_split = op.split(trades_in, "ticker", tuple(tickers))

    ctrds = []
    ticker_idx = 0
    for _ in tickers:
        trade_per_imnt = trade_split[ticker_idx]
        cum_trade = op.cum_trade(trade_per_imnt)
        ctrds.append(cum_trade)
        ticker_idx += 1

    bars = [compute_bar(bbo, trd, ctrdt) for bbo, trd, ctrdt in zip(bbos, trade_split, ctrds)]

    out_stream = op.join(*bars, "ticker", extr.Array(extr.Char, 16), tuple(tickers))

    val_aggr = op.accumulate(out_stream)

    graph.stream_ctx().run_to(New_York_time(2015, 9, 1, 17, 30, 5))

    result_as_pandas(val_aggr).to_csv(src_dir + "/brokertech_bar.test.csv", index=False)

    extr.flush()
    extr.assert_numdiff(src_dir + "/data/brokertech_bar.base.csv", src_dir + "/brokertech_bar.test.csv")
