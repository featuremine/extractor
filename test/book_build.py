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
 * @file book_build.py
 * @author Maxim Trokhimtchouk
 * @date 26 Oct 2018
 * @brief Puthon tool for converting itch to ore
 */
"""
import tempfile
import extractor as extr
import msgpack as mp
import ytp_base
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def ore_to_ytp(ore_path, yamal_path):
    with open(ore_path, "rb") as ore_file:
        peer = ytp_base.peer(yamal_path)

        unpacker = mp.Unpacker(ore_file, raw=False)

        version = unpacker.unpack()
        symbology = unpacker.unpack()

        symbols = {}
        for i, symb in enumerate(symbology):
            symbols[symb['symbol']] = {
                'bookid': i,
                'symbol': symb['symbol'],
                'price_tick': symb['price_tick']
            }

        for symb, info in symbols.items():
            peer.write(1, mp.packb([15, 0, 0, 0, 0, info['bookid'], symb, info['price_tick']]))

        while True:
            try:
                msg = unpacker.unpack()
            except mp.OutOfData:
                break

            peer.write(1, mp.packb(msg))


if __name__ == "__main__":

    def book_play_split(op, in_file):
        return op.book_play_split(in_file, ("2_YEAR", "3_YEAR", "10_YEAR"))

    def book_sim_split(op, in_file):
        return op.ore_sim_split(in_file, ("2_YEAR", "3_YEAR", "10_YEAR"))

    def run(in_file, book_split):
        graph = extr.system.comp_graph()
        op = graph.features
        upds = book_split(op, in_file)
        levels = [op.book_build(upd, 5) for upd in upds]
        joint_stream = op.last(*levels, "ticker", extr.Array(extr.Char, 16),
                               ("2_YEAR", "3_YEAR", "10_YEAR"))
        trigger = op.trigger(joint_stream)
        out_stream = op.combine(trigger, tuple(), joint_stream, tuple())
        graph.features.csv_record(out_stream, os.path.join(src_dir, "data/book_levels.extr.py_book_build.test.csv"),
                                  ("time", "ticker",
                                   "bid_prx_0", "bid_shr_0", "bid_ord_0",
                                   "bid_prx_1", "bid_shr_1", "bid_ord_1",
                                   "bid_prx_2", "bid_shr_2", "bid_ord_2",
                                   "bid_prx_3", "bid_shr_3", "bid_ord_3",
                                   "bid_prx_4", "bid_shr_4", "bid_ord_4",
                                   "ask_prx_0", "ask_shr_0", "ask_ord_0",
                                   "ask_prx_1", "ask_shr_1", "ask_ord_1",
                                   "ask_prx_2", "ask_shr_2", "ask_ord_2",
                                   "ask_prx_3", "ask_shr_3", "ask_ord_3",
                                   "ask_prx_4", "ask_shr_4", "ask_ord_4",
                                   ))
        graph.stream_ctx().run()

        extr.flush()
        extr.assert_numdiff(os.path.join(src_dir, "data/book_levels.extr.py_book_build.test.csv"),
                            os.path.join(src_dir, "data/book_levels.extr.base.csv"))

    run(os.path.join(src_dir, "data/book.base.ore"), book_play_split)
    run(os.path.join(src_dir, "data/book.1.0.0.ore"), book_play_split)

    with tempfile.NamedTemporaryFile() as yamal_file:
        ore_to_ytp(os.path.join(src_dir, "data/book.base.ore"), yamal_file.name)
        run(yamal_file.name, book_sim_split)

    with tempfile.NamedTemporaryFile() as yamal_file:
        ore_to_ytp(os.path.join(src_dir, "data/book.base.ore"), yamal_file.name)
        run(yamal_file.name, book_sim_split)
