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
@package book_montage.py
@author Andres Rangel
@date 25 Mar 2020
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


class ValidationBook:
    def __init__(self):
        self.bids = {}
        self.asks = {}
        self.oldbidpx = {}
        self.oldaskpx = {}

    def proc_bbo(self, bbo, idx):
        bidqty = bbo[0].bidqty
        askqty = bbo[0].askqty
        bidprice = float(str(bbo[0].bidprice))
        askprice = float(str(bbo[0].askprice))

        if (idx in self.oldbidpx) and (bidprice != self.oldbidpx[idx][0]):
            if self.oldbidpx[idx][1] != 0:
                oldpx = self.oldbidpx[idx][0]
                del self.bids[oldpx][idx]
                if len(self.bids[oldpx]) == 0:
                    del self.bids[oldpx]

        if (idx in self.oldaskpx) and (askprice != self.oldaskpx[idx][0]):
            if self.oldaskpx[idx][1] != 0:
                oldpx = self.oldaskpx[idx][0]
                del self.asks[oldpx][idx]
                if len(self.asks[oldpx]) == 0:
                    del self.asks[oldpx]

        if askqty != 0:
            if askprice in self.asks:
                self.asks[askprice][idx] = askqty
            else:
                self.asks[askprice] = {idx: askqty}

        if bidqty != 0:
            if bidprice in self.bids:
                self.bids[bidprice][idx] = bidqty
            else:
                self.bids[bidprice] = {idx: bidqty}

        self.oldbidpx[idx] = [bidprice, bidqty]
        self.oldaskpx[idx] = [askprice, askqty]

    def validate_side(self, validationside, bookside):
        if len(bookside) != len(validationside):
            return False

        for price, level in bookside:
            if price not in validationside:
                return False
            validationlevel = validationside[price]

            if len(validationlevel) != len(level):
                return False

            for order in level:
                if order.id not in validationlevel:
                    return False
                if validationlevel[order.id] != order.qty:
                    return False

        return True

    def validate_book(self, book):
        return self.validate_side(self.bids, book[extr.trade_side.BID()]) and \
            self.validate_side(self.asks, book[extr.trade_side.ASK()])


if __name__ == "__main__":
    graph = extr.system.comp_graph()

    bbo_file = os.path.join(src_dir, "data/sip_quotes_20171018.mp")
    trade_file = os.path.join(src_dir, "data/sip_trades_20171018.mp")

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
                                   bbos_in.bidqty, tuple(),
                                   bbos_in.askqty, tuple());

    bbo_split = op.split(bbos_in, "market", tuple(markets))

    bbos = []
    decimal_bbos = []
    ctrds = []
    mkt_idx = 0
    for mkt in markets:
        mkt_tickers = [x[mkt] for x in tickers]
        mkt_bbo_split = op.split(bbo_split[mkt_idx], "ticker", tuple(mkt_tickers))
        mkt_bbos = []
        mkt_decimal_bbos = []
        ticker_idx = 0
        for _ in tickers:
            bbo = mkt_bbo_split[ticker_idx]
            mkt_bbos.append(bbo)
            decimal_bbo = op.combine(op.fields(bbo, ("receive", "bidprice", "askprice")), tuple(),
                                     op.convert(bbo.bidqty, extr.Decimal128), tuple(),
                                     op.convert(bbo.askqty, extr.Decimal128), tuple())
            mkt_decimal_bbos.append(decimal_bbo)
            ticker_idx = ticker_idx + 1
        bbos.append(mkt_bbos)
        decimal_bbos.append(mkt_decimal_bbos)
        mkt_idx = mkt_idx + 1

    books = [extr.Book() for _ in tickers]
    validation_books = [ValidationBook() for _ in tickers]
    for valbook, ticker_bbos in zip(validation_books, zip(*bbos)):
        i = 0

        def gen_clbck(b, j):
            def clbck(frame):
                b.proc_bbo(frame, j)
                pass
            return clbck
        for bbo in ticker_bbos:
            graph.callback(bbo, gen_clbck(valbook, i))
            i += 1

    book_nbbos = [op.bbo_book_aggr(*x, b) for b, x in zip(books, zip(*decimal_bbos))]
    book_nbbo_refs = [graph.get_ref(comp) for comp in book_nbbos]
    for valbook, book_nbbo, book in zip(validation_books, book_nbbos, books):
        def gen_clbck(v, b):
            def clbck(frame):
                assert v.validate_book(b)
            return clbck
        graph.callback(book_nbbo, gen_clbck(valbook, book))

    nbbos = [op.bbo_aggr(*x) for x in zip(*bbos)]
    nbbo_refs = [graph.get_ref(comp) for comp in nbbos]

    out_book_nbbos = op.join(*book_nbbos, "ticker", extr.Array(extr.Char, 16),
                             tuple([x["NASDAQOMX"] for x in tickers]))

    out_nbbos = op.join(*nbbos, "ticker", extr.Array(extr.Char, 16),
                        tuple([x["NASDAQOMX"] for x in tickers]))

    nbbos_aggr = op.accumulate(out_nbbos)
    book_nbbos_aggr = op.accumulate(out_book_nbbos)

    ctx = graph.stream_ctx()

    for book_nbbo, nbbo in zip(book_nbbo_refs, nbbo_refs):
        assert book_nbbo[0].receive == nbbo[0].receive
        assert book_nbbo[0].askprice == nbbo[0].askprice
        assert isinstance(nbbo[0].askqty, int)
        assert book_nbbo[0].askqty == extr.Decimal128(nbbo[0].askqty)
        assert book_nbbo[0].bidprice == nbbo[0].bidprice
        assert isinstance(nbbo[0].bidqty, int)
        assert book_nbbo[0].bidqty == extr.Decimal128(nbbo[0].bidqty)

    ctx.run()
    book_nbbos_aggr_df = result_as_pandas(book_nbbos_aggr)
    book_nbbos_aggr_df.bidqty = book_nbbos_aggr_df.bidqty.astype('str')
    book_nbbos_aggr_df.askqty = book_nbbos_aggr_df.askqty.astype('str')
    book_nbbos_aggr_df.bidqty = book_nbbos_aggr_df.bidqty.astype('int32')
    book_nbbos_aggr_df.askqty = book_nbbos_aggr_df.askqty.astype('int32')
    assert_frame_equal(result_as_pandas(nbbos_aggr), book_nbbos_aggr_df)
