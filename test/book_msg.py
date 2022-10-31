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
 * @file book_msg.py
 * @author Andres Rangel
 * @date 10 Jan 2019
 * @brief Puthon test for book_msg and book_trades features
 */
"""
import extractor as extr
from extractor import result_as_pandas
from pandas.testing import assert_frame_equal
import pandas as pd
from numpy.testing import assert_array_equal, assert_array_almost_equal
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    upd = op.book_play_split(os.path.join(src_dir, "data/book.base.ore"), ("3_YEAR",))

    trade_msgs = op.fields(op.book_msg(upd[0], "trade"), ("batch", "qty", "seqn", "trade_price", "vendor"))
    exec_msgs = op.fields(op.book_msg(upd[0], "execute"), ("batch", "qty", "seqn", "trade_price", "vendor"))
    all_msgs = op.fields(op.book_trades(upd[0]), ("batch", "qty", "seqn", "trade_price", "vendor"))

    dirty_msgs = op.join(trade_msgs, exec_msgs, "type", extr.Array(extr.Char, 16), ("trade_msgs", "exec_msgs"))
    merged_msgs = op.fields(dirty_msgs, ("batch", "qty", "seqn", "trade_price", "vendor"))

    merged_aggr = op.accumulate(merged_msgs)
    all_aggr = op.accumulate(all_msgs)

    synth = op.book_play_split(os.path.join(src_dir, "data/synth_book.base.ore"), ("3_YEAR",))
    add_msgs = op.book_msg(synth[0], "add")
    add_aggr = op.accumulate(add_msgs)
    insrt_msgs = op.book_msg(synth[0], "insert")
    insrt_aggr = op.accumulate(insrt_msgs)
    pos_msgs = op.book_msg(synth[0], "position")
    pos_aggr = op.accumulate(pos_msgs)
    cancel_msgs = op.book_msg(synth[0], "cancel")
    cancel_aggr = op.accumulate(cancel_msgs)
    execute_msgs = op.book_msg(synth[0], "execute")
    execute_aggr = op.accumulate(execute_msgs)
    trade_msgs = op.book_msg(synth[0], "trade")
    trade_aggr = op.accumulate(trade_msgs)
    state_msgs = op.book_msg(synth[0], "state")
    state_aggr = op.accumulate(state_msgs)
    control_msgs = op.book_msg(synth[0], "control")
    control_aggr = op.accumulate(control_msgs)
    book = extr.Book()

    set_msgs = op.book_msg(synth[0], "set")

    def validate_non_empty_book(frame):
        assert len(book[extr.trade_side.BID()]) != 0
        assert len(book[extr.trade_side.ASK()]) != 0

    graph.callback(set_msgs, validate_non_empty_book)
    set_aggr = op.accumulate(set_msgs)

    op.book_build(synth[0], book)

    graph.stream_ctx().run()

    merged_pd = result_as_pandas(merged_aggr)
    all_pd = result_as_pandas(all_aggr)
    assert_frame_equal(merged_pd, all_pd)

    add_pd = result_as_pandas(add_aggr)
    assert_array_equal(add_pd["Timestamp"].values,
                       pd.to_datetime(['1970-01-09T08:01:49.000000000', '1970-01-09T08:01:54.000000000',
                                       '1970-01-09T08:01:56.000000000', '1970-01-09T08:01:58.000000000',
                                       '1970-01-09T08:02:01.000000000', '1970-01-09T08:02:01.000000000']))
    # The modify msg is within a batch of a cancel(batch=1) and add(batch=0)
    assert_array_equal(add_pd["batch"].values, [0, 0, 0, 0, 0, 0])
    assert_array_equal(add_pd["id"].values, [300, 303, 304, 305, 306, 307])
    assert_array_equal(add_pd["is_bid"].values, [1, 1, 1, 1, 1, 0])
    assert_array_equal(add_pd["price"].values, [extr.Decimal128(val) for val in ["0.2109375", "0.2109375", "0.2109375", "0.2109375", "0.2109375", "0.2109375"]])
    assert_array_equal(add_pd["qty"].values, [extr.Decimal128(val) for val in [200, 250, 200, 200, 200, 200]])
    assert_array_equal(add_pd["seqn"].values, [0, 0, 0, 0, 0, 0])
    assert_array_equal(add_pd["vendor"].values,
                       pd.to_datetime(['1970-01-09T08:01:49.000000000', '1970-01-09T08:01:54.000000000',
                                       '1970-01-09T08:01:56.000000000', '1970-01-09T08:01:58.000000000',
                                       '1970-01-09T08:02:01.000000000', '1970-01-09T08:02:01.000000000']))

    insrt_pd = result_as_pandas(insrt_aggr)
    assert_array_equal(insrt_pd["Timestamp"].values,
                       pd.to_datetime(['1970-01-09T08:01:50.000000000']))
    assert_array_equal(insrt_pd["batch"].values, [0])
    assert_array_equal(insrt_pd["id"].values, [301])
    assert_array_equal(insrt_pd["is_bid"].values, [1])
    assert_array_equal(insrt_pd["price"].values, [extr.Decimal128("0.2109375")])
    assert_array_equal(insrt_pd["prio"].values, [5])
    assert_array_equal(insrt_pd["qty"].values, [extr.Decimal128(200)])
    assert_array_equal(insrt_pd["seqn"].values, [0])
    assert_array_equal(insrt_pd["vendor"].values,
                       pd.to_datetime(['1970-01-09T08:01:50.000000000']))

    pos_pd = result_as_pandas(pos_aggr)
    assert_array_equal(pos_pd["Timestamp"].values,
                       pd.to_datetime(['1970-01-09T08:01:51.000000000']))
    assert_array_equal(pos_pd["batch"].values, [0])
    assert_array_equal(pos_pd["id"].values, [302])
    assert_array_equal(pos_pd["is_bid"].values, [1])
    assert_array_equal(pos_pd["pos"].values, [0])
    assert_array_equal(pos_pd["price"].values, [extr.Decimal128("0.2109375")])
    assert_array_equal(pos_pd["qty"].values, [extr.Decimal128(200)])
    assert_array_equal(pos_pd["seqn"].values, [0])
    assert_array_equal(pos_pd["vendor"].values,
                       pd.to_datetime(['1970-01-09T08:01:51.000000000']))

    cancel_pd = result_as_pandas(cancel_aggr)
    assert_array_equal(cancel_pd["Timestamp"].values,
                       pd.to_datetime(['1970-01-09T08:01:52.000000000', '1970-01-09T08:01:53.000000000',
                                       '1970-01-09T08:01:54.000000000']))
    # The modify msg is within a batch of a cancel(batch=1) and add(batch=0)
    assert_array_equal(cancel_pd["batch"].values, [0, 0, 1])
    assert_array_equal(cancel_pd["id"].values, [300, 301, 302])
    assert_array_equal(cancel_pd["is_bid"].values, [1, 1, 1])
    assert_array_equal(cancel_pd["price"].values, [extr.Decimal128(val) for val in ["0.2109375", "0.2109375", "0.2109375"]])
    assert_array_equal(cancel_pd["qty"].values, [extr.Decimal128(val) for val in [200, 200, 200]])
    assert_array_equal(cancel_pd["seqn"].values, [0, 0, 0])
    assert_array_equal(cancel_pd["vendor"].values,
                       pd.to_datetime(['1970-01-09T08:01:52.000000000', '1970-01-09T08:01:53.000000000',
                                       '1970-01-09T08:01:54.000000000']))

    execute_pd = result_as_pandas(execute_aggr)
    assert_array_equal(execute_pd["Timestamp"].values,
                       pd.to_datetime(['1970-01-09T08:01:55.000000000', '1970-01-09T08:01:57.000000000',
                                       '1970-01-09T08:01:58.000000000', '1970-01-09T08:01:58.000000000']))
    assert_array_equal(execute_pd["batch"].values, [0, 0, 0, 0])
    assert_array_equal(execute_pd["id"].values, [303, 304, 305, 305])
    assert_array_equal(execute_pd["is_bid"].values, [1, 1, 1, 1])
    assert_array_equal(execute_pd["price"].values, [extr.Decimal128(val) for val in ["0.2109375", "0.2109375", "0.2109375", "0.2109375"]])
    assert_array_equal(execute_pd["qty"].values, [extr.Decimal128(val) for val in [250, 200, 100, 50]])
    assert_array_equal(execute_pd["seqn"].values, [0, 0, 0, 0])
    assert_array_equal(execute_pd["vendor"].values,
                       pd.to_datetime(['1970-01-09T08:01:55.000000000', '1970-01-09T08:01:57.000000000',
                                       '1970-01-09T08:01:57.000000000', '1970-01-09T08:01:58.000000000']))

    trade_pd = result_as_pandas(trade_aggr)
    assert_array_equal(trade_pd["Timestamp"].values,
                       pd.to_datetime(['1970-01-09T08:01:59.000000000', '1970-01-09T08:02:00.000000000']))
    assert_array_equal(trade_pd["batch"].values, [0, 0])
    assert_array_equal(trade_pd["trade_price"].values, [extr.Decimal128(val) for val in ["0.2109375", "0.2109375"]])
    assert_array_equal(trade_pd["qty"].values, [extr.Decimal128(val) for val in [25, 25]])
    assert_array_equal(trade_pd["seqn"].values, [0, 0])
    assert_array_equal(trade_pd["vendor"].values,
                       pd.to_datetime(['1970-01-09T08:01:59.000000000', '1970-01-09T08:02:00.000000000']))

    state_pd = result_as_pandas(state_aggr)
    assert_array_equal(state_pd["Timestamp"].values,
                       pd.to_datetime(['1970-01-09T08:02:02.000000000']))
    assert_array_equal(state_pd["batch"].values, [0])
    assert_array_equal(state_pd["id"].values, [306])
    assert_array_equal(state_pd["is_bid"].values, [1])
    assert_array_equal(state_pd["price"].values, [extr.Decimal128("0.2109375")])
    assert_array_equal(state_pd["seqn"].values, [0])
    assert_array_equal(state_pd["state"].values, [3])
    assert_array_equal(state_pd["vendor"].values,
                       pd.to_datetime(['1970-01-09T08:02:02.000000000']))

    control_pd = result_as_pandas(control_aggr)
    assert_array_equal(control_pd["Timestamp"].values,
                       pd.to_datetime(['1970-01-09T08:02:03.000000000',
                                       '1970-01-09T08:02:05.000000000']))
    assert_array_equal(control_pd["batch"].values, [0, 0])
    assert_array_equal(control_pd["command"].values, ['\x00', 'C'])
    assert_array_equal(control_pd["seqn"].values, [0, 0])
    assert_array_equal(control_pd["uncross"].values, [4, 4])
    assert_array_equal(control_pd["vendor"].values,
                       pd.to_datetime(['1970-01-09T08:02:03.000000000',
                                       '1970-01-09T08:02:05.000000000']))

    set_pd = result_as_pandas(set_aggr)
    assert_array_equal(set_pd["Timestamp"].values,
                       pd.to_datetime(['1970-01-09T08:02:04.000000000']))
    assert_array_equal(set_pd["batch"].values, [0])
    assert_array_equal(set_pd["is_bid"].values, [1])
    assert_array_equal(set_pd["price"].values, [extr.Decimal128("0.2109375")])
    assert_array_equal(set_pd["qty"].values, [extr.Decimal128(100)])
    assert_array_equal(set_pd["seqn"].values, [0])
    assert_array_equal(set_pd["vendor"].values,
                       pd.to_datetime(['1970-01-09T08:02:04.000000000']))

    assert len(book[extr.trade_side.BID()]) == 0
    assert len(book[extr.trade_side.ASK()]) == 0
