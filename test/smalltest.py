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
@package vwap.py
@author Maxim Trokhimtchouk
@date 22 Apr 2018
@brief File contains extractor python sample
"""

from datetime import datetime, timedelta
import extractor as extr
import pytz


def epoch_delta(date):
    return date - pytz.timezone("UTC").localize(datetime(1970, 1, 1))


def New_York_time(year, mon, day, h=0, m=0, s=0):
    return epoch_delta(pytz.timezone("America/New_York").
                       localize(datetime(year, mon, day, h, m, s)))


def compute_diff(nbbo):
    resolution = timedelta(seconds=1)
    bar_period = timedelta(minutes=5)
    prices = op.field(nbbo, "bidprice")
    prices_lagged = op.time_lag(prices, bar_period, resolution)
    return op.diff(prices_lagged, prices)


if __name__ == "__main__":
    graph = extr.system.comp_graph()

    bbo_file = "test/extractor/sip_quotes_20171018.base.mp"
    bar_file = "test/extractor/bar_20171018.test.csv"

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
         ("bidprice", extr.Rprice, ""),
         ("askprice", extr.Rprice, ""),
         ("bidqty", extr.Int32, ""),
         ("askqty", extr.Int32, "")))

    converted_bbos_in = op.combine(bbos_in.receive, tuple(),
                                   bbos_in.ticker, tuple(),
                                   bbos_in.market, tuple(),
                                   op.convert(bbos_in.bidprice, extr.Decimal128), tuple(),
                                   op.convert(bbos_in.askprice, extr.Decimal128), tuple(),
                                   bbos_in.bidqty, tuple(),
                                   bbos_in.askqty, tuple());

    bbo_split = op.split(converted_bbos_in, "market", tuple(markets))

    bbos = []
    for mkt in markets:
        mkt_tickers = [x[mkt] for x in tickers]
        mkt_bbo_split = op.split(bbo_split, "ticker", tuple(mkt_tickers))
        bbos.append([op.identity(mkt_bbo_split) for _ in mkt_tickers])

    nbbos = [op.bbo_aggr(*x) for x in zip(*bbos)]

    bars = [compute_diff(nbbo) for nbbo in nbbos]
    out_stream = op.join(*bars, "ticker", extr.Array(extr.Char, 16),
                         tuple([x["NASDAQOMX"] for x in tickers]))

    op.csv_record(out_stream, bar_file)

    graph.stream_ctx().run_to(New_York_time(2017, 10, 18, 16))
