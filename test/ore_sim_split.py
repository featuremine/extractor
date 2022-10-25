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
 * @author Federico Ravchina
 * @date 05 Apr 2021
 * @brief Python test for ore_sim_split
 */
"""

import extractor as extr
import tempfile
import msgpack as mp
import ytp_base
import numpy as np
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


def main_sim(yamal_path, symbols, with_time):
    graph = extr.system.comp_graph()
    op = graph.features

    if with_time:
        time_upds, *upds = op.ore_sim_split(yamal_path, symbols, True)
    else:
        upds = op.ore_sim_split(yamal_path, symbols)

    add_msgs = op.book_msg(upds[1], 'add')
    one = op.accumulate(add_msgs)

    if with_time:
        time_msgs = op.book_msg(time_upds, 'time')
        two = op.accumulate(time_msgs)

    graph.stream_ctx().run()

    as_pd_one = extr.result_as_pandas(one)

    if with_time:
        as_pd_two = extr.result_as_pandas(two)

    np.testing.assert_array_equal(as_pd_one['Timestamp'],
                                  np.array([
                                      '1970-01-09T08:01:49.000000000',
                                      '1970-01-09T08:01:54.000000000',
                                      '1970-01-09T08:01:56.000000000',
                                      '1970-01-09T08:01:58.000000000',
                                      '1970-01-09T08:02:01.000000000',
                                      '1970-01-09T08:02:01.000000000'],
        dtype='datetime64[ns]'))

    np.testing.assert_array_equal(as_pd_one['id'],
                                  [300, 303, 304, 305, 306, 307])

    if with_time:
        np.testing.assert_array_equal(as_pd_two['Timestamp'],
                                      np.array(['1970-01-01T00:00:00.000000000'],
                                               dtype='datetime64[ns]'))

        np.testing.assert_array_equal(as_pd_two['seconds'],
                                      np.array(['1970-01-05T04:00:54.000000000'],
                                               dtype='datetime64[ns]'))


if __name__ == "__main__":

    with tempfile.NamedTemporaryFile() as yamal_file:
        ore_to_ytp(os.path.join(src_dir, "data/synth_book.ore"), yamal_file.name)
        main_sim(yamal_file.name, ("2_YEAR", "3_YEAR"), False)
        main_sim(yamal_file.name, ("2_YEAR", "3_YEAR"), True)
