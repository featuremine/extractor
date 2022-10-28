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
from time import sleep

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    synth = op.book_play_split("/home/ndy/coinbase_BTC-USD.ore", ("BTC-USD",))
    book = extr.Book()
    bb = op.unique(op.book_build(synth[0], book, 1))

    add_msgs = graph.get_ref(op.book_msg(synth[0], "add"))
    insrt_msgs = graph.get_ref(op.book_msg(synth[0], "insert"))
    pos_msgs = graph.get_ref(op.book_msg(synth[0], "position"))
    cancel_msgs = graph.get_ref(op.book_msg(synth[0], "cancel"))
    execute_msgs = graph.get_ref(op.book_msg(synth[0], "execute"))
    trade_msgs = graph.get_ref(op.book_msg(synth[0], "trade"))
    state_msgs = graph.get_ref(op.book_msg(synth[0], "state"))
    control_msgs = graph.get_ref(op.book_msg(synth[0], "control"))

    graph.callback(bb, lambda frame: print(f"====================================================================\n{add_msgs}\n{insrt_msgs}\n{pos_msgs}\n{cancel_msgs}\n{execute_msgs}\n{trade_msgs}\n{state_msgs}\n{control_msgs}\n"))

    graph.stream_ctx().run()
