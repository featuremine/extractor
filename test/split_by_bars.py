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
@package split_by_bars.py
@author Federico Ravchina
@date 22 Nov 2021
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


def module_compute_bar():
    bar_period = timedelta(minutes=5)

    [m, inputs] = extr.system.module(1, "test_module")
    m_op = m.features

    close = m_op.timer(bar_period)

    trades = inputs[0]
    trade = m_op.fields(trades, ("price", "qty", "market"))
    trade_px = m_op.field(trade, "price")
    first_trade = m_op.first_after(trade, close)
    open_trade = m_op.last_asof(first_trade, close)
    close_trade = m_op.last_asof(trade, close)
    high_trade = m_op.last_asof(m_op.asof(trade, m_op.max(trade_px, first_trade)), close)
    low_trade = m_op.last_asof(m_op.asof(trade, m_op.min(trade_px, first_trade)), close)

    combined = m_op.combine(
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
        close, (("actual", "close_time"),))
    m.declare_outputs(combined)
    return m


def module_compute_bar_ex(graph, trades_in, m, name=None):
    return graph.extend(m, trades_in)


good_tickers = [
    {"NYSEMKT": "BA", "NASDAQOMX": "BA", "NYSEArca": "BA"},
    {"NYSEMKT": "AA", "NASDAQOMX": "AA", "NYSEArca": "AA"},
    {"NYSEMKT": "A", "NASDAQOMX": "A", "NYSEArca": "A"},
]

tickers = good_tickers

trade_file = src_dir + "/data/sip_trades_20171018.mp"


def split_by_test(graph, op, m, trades_in):
    out_stream_split_by = op.split_by(trades_in, m, "ticker")
    return op.accumulate(out_stream_split_by)


def split_test(graph, op, m, trades_in):
    trade_imnt_split = op.split(trades_in, "ticker", tuple([x["NASDAQOMX"] for x in tickers]))
    bars = [module_compute_bar_ex(graph, trd, m)[0] for trd in trade_imnt_split]

    triggers = {
        "A": op.constant(("time", extr.Time64, pd.to_datetime('2017-10-18T13:35:00.000000000'))),
        "AA": op.constant(("time", extr.Time64, pd.to_datetime('2017-10-18T10:40:00.000000000'))),
        "BA": op.constant(("time", extr.Time64, pd.to_datetime('2017-10-18T10:00:00.000000000')))
    }

    usable_tickers = tuple([x["NASDAQOMX"] for x in tickers])

    filtered_bars = [op.filter_if(op.trigger(bar) >= triggers[ticker], bar) for ticker, bar in zip(usable_tickers, bars)]

    out_stream_filtered = op.join(*filtered_bars, "ticker", extr.Array(extr.Char, 16),
                                  tuple([x["NASDAQOMX"] for x in tickers]))

    return op.accumulate(out_stream_filtered)


def run(test_case, post, m):
    graph = extr.system.comp_graph()
    op = graph.features

    trades_in = op.mp_play(
        trade_file,
        (("receive", extr.Time64, ""),
         ("ticker", extr.Array(extr.Char, 16), ""),
         ("market", extr.Array(extr.Char, 32), ""),
         ("price", extr.Rprice, ""),
         ("qty", extr.Int32, ""),
         ("side", extr.Int32, "")))

    ret = test_case(graph, op, m, trades_in)

    graph.stream_ctx().run_to(New_York_time(2017, 10, 18, 16))

    ret = post(ret)

    extr.flush()

    return ret


if __name__ == "__main__":

    m = module_compute_bar()

    def single_test_post(ret):
        return result_as_pandas(ret)

    def multi_test(graph, op, m, trades_in):
        return (
            split_by_test(graph, op, m, trades_in),
            split_test(graph, op, m, trades_in),
        )

    def multi_test_post(ret):
        return (
            result_as_pandas(ret[0]),
            result_as_pandas(ret[1]),
        )

    psbar = run(split_by_test, single_test_post, m)
    pbar = run(split_test, single_test_post, m)

    assert_frame_equal(pbar, psbar)

    ret = run(multi_test, multi_test_post, m)

    assert_frame_equal(ret[0], ret[1])
