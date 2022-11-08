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
@package vwap_reverse.py
@author Andres Rangel
@date 28 Jun 2018
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
        vwap, ("vwap",))


if __name__ == "__main__":
    graph = extr.system.comp_graph()

    bbo_file = os.path.join(src_dir, "data/sip_quotes_20171018.mp")
    trade_file = os.path.join(src_dir, "data/sip_trades_20171018.mp")
    bar_file = os.path.join(src_dir, "data/bar_20171018_vwap_reverse.test.csv")

    markets = ["NYSEMKT", "NASDAQOMX", "NYSEArca"]
    tickers = [
        {"NYSEMKT": "A", "NASDAQOMX": "A", "NYSEArca": "A"},
        {"NYSEMKT": "AA", "NASDAQOMX": "AA", "NYSEArca": "AA"},
        {"NYSEMKT": "BA", "NASDAQOMX": "BA", "NYSEArca": "BA"}
    ]
    imnts = [x["NASDAQOMX"] for x in tickers]

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

    bbo_split = op.split(bbos_in, "ticker", tuple(imnts))

    trades_in = op.mp_play(
        trade_file,
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("price", extr.Rprice, ""),
         ("qty", extr.Int32, ""),
         ("side", extr.Int32, "")))

    trade_split = op.split(trades_in, "ticker", tuple(imnts))

    bbos = {k: [] for k in markets}
    ctrds = {k: [] for k in markets}
    imnt_idx = 0
    for _ in imnts:
        mkt_bbo_split = op.split(bbo_split[imnt_idx], "market", tuple(markets))
        mkt_trade_split = op.split(trade_split[imnt_idx], "market", tuple(markets))
        mkt_idx = 0
        for mkt in markets:
            bbo = mkt_bbo_split[mkt_idx]
            trade = mkt_trade_split[mkt_idx]
            cum_trade = op.cumulative(op.combine(trade.qty, (("qty", "shares"),), op.convert(trade.qty, extr.Float64) * op.convert(trade.price, extr.Float64), (("qty", "notional",),)))
            bbos[mkt].append(bbo)
            ctrds[mkt].append(cum_trade)
            mkt_idx = mkt_idx + 1
        imnt_idx = imnt_idx + 1

    nbbos = [op.bbo_aggr(*x) for x in zip(*bbos.values())]
    ctrdts = [op.sum(*x) for x in zip(*ctrds.values())]

    bars = [compute_bar(nbbo, ctrdt) for nbbo, ctrdt in zip(nbbos, ctrdts)]
    out_stream = op.join(*bars, "ticker", extr.Array(extr.Char, 16),
                         tuple([x["NASDAQOMX"] for x in tickers]))

    op.csv_record(out_stream, bar_file)

    graph.stream_ctx().run_to(New_York_time(2017, 10, 18, 16))

    extr.flush()

    extr.assert_numdiff(os.path.join(src_dir, 'data/bar_20171018_vwap.base.csv'),
                        os.path.join(src_dir, 'data/bar_20171018_vwap_reverse.test.csv'))
