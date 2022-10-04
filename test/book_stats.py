#!/usr/bin/env python3
"""
/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/
"""

"""
 * @file book_stats.py
 * @author Andres Rangel
 * @date 10 Feb 2020
 * @brief Python tool for order cancelation calculations
 */
"""

from datetime import timedelta
import extractor as extr
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def count_lambda(year_label, side):
    def l(frame):
        print("{} Canceled orders count for {} is: {}".format(side, year_label, frame[0].count))
    return l


def shares_lambda(year_label, side):
    def l(frame):
        print("{} Canceled shares for {}: {}".format(side, year_label, frame[0].qty))
    return l


def avg_px_lambda(year_label, side):
    def l(frame):
        print("{} Average cancelation price for {} is: {}".format(side, year_label, frame[0].price))
    return l


def cancel_print_lambda(year_label, side):
    def l(frame):
        print("{} Cancel for {} is: {}".format(side, year_label, frame))
    return l

# If it is desired to debug the data produced by the nodes of interest,
# please enable the callbacks uncommenting the lines: 79, 93, 104, 116, 130 and 141.


if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()
    op = graph.features

    year_labels = ("2_YEAR", "3_YEAR", "10_YEAR")
    upds = op.book_play_split(os.path.join(src_dir, "data/book.base.ore"), year_labels)

    for upd, year in zip(upds, year_labels):

        # If desired, the number of levels can be changes as desired
        level = op.book_build(upd, 2)

        cancel = op.book_msg(upd, "cancel")

        is_bid = op.convert(cancel.is_bid, extr.Bool)

        # bid

        bid_cancel = op.filter_if(is_bid & op.greater_equal(cancel.price, level.bid_prx_0), cancel)

        bid_cancel_count = op.count(bid_cancel)
        bid_lagged_cancel_count = op.time_lag(bid_cancel_count, timedelta(seconds=10), timedelta(milliseconds=100))

        # Canceled orders for buy side from the best book level
        bid_diff = op.diff(bid_cancel_count, bid_lagged_cancel_count)

        #graph.callback(bid_diff, count_lambda(year, "BID"))

        bid_px = bid_cancel.price
        bid_qty = bid_cancel.qty
        float_bid_px = op.convert(bid_px, extr.Float64)
        float_bid_qty = op.convert(bid_qty, extr.Float64)

        bid_shares = op.cumulative(float_bid_qty)

        delayed_bid_shares = op.time_lag(bid_shares, timedelta(seconds=10), timedelta(milliseconds=100))

        # Canceled shares for buy side from the best book level
        bid_shares_diff = op.diff(bid_shares, delayed_bid_shares)

        #graph.callback(bid_shares_diff, shares_lambda(year, "BID"))

        bid_factors = op.cumulative(float_bid_px * float_bid_qty)
        delayed_bid_factors = op.time_lag(bid_factors, timedelta(seconds=10), timedelta(milliseconds=100))
        bid_factors_diff = bid_factors - delayed_bid_factors

        # Average price for buy side cancellations
        bid_canceled_px = op.cond(op.is_zero(bid_shares_diff),
                                  op.constant(("price", extr.Float64, 0.0)),
                                  bid_factors_diff / bid_shares_diff)

        #graph.callback(bid_canceled_px, avg_px_lambda(year, "BID"))

        # ask

        ask_cancel = op.filter_if(op.logical_not(is_bid) & op.less_equal(cancel.price, level.ask_prx_0), cancel)

        ask_cancel_count = op.count(ask_cancel)
        ask_lagged_cancel_count = op.time_lag(ask_cancel_count, timedelta(seconds=10), timedelta(milliseconds=100))

        # Canceled orders for sell side from the best book level
        ask_diff = op.diff(ask_cancel_count, ask_lagged_cancel_count)

        #graph.callback(ask_diff, count_lambda(year, "ASK"))

        ask_px = ask_cancel.price
        ask_qty = ask_cancel.qty
        float_ask_px = op.convert(ask_px, extr.Float64)
        float_ask_qty = op.convert(ask_qty, extr.Float64)

        ask_shares = op.cumulative(float_ask_qty)

        delayed_ask_shares = op.time_lag(ask_shares, timedelta(seconds=10), timedelta(milliseconds=100))

        # Canceled shares for sell side from the best book level
        ask_shares_diff = op.diff(ask_shares, delayed_ask_shares)

        #graph.callback(ask_shares_diff, shares_lambda(year, "ASK"))

        ask_factors = op.cumulative(float_ask_px * float_ask_qty)
        delayed_ask_factors = op.time_lag(ask_factors, timedelta(seconds=10), timedelta(milliseconds=100))
        ask_factors_diff = ask_factors - delayed_ask_factors

        # Average price for sell side cancellations
        ask_canceled_px = op.cond(op.is_zero(ask_shares_diff),
                                  op.constant(("price", extr.Float64, 0.0)),
                                  ask_factors_diff / ask_shares_diff)

        #graph.callback(ask_canceled_px, avg_px_lambda(year, "ASK"))

    graph.stream_ctx().run()
