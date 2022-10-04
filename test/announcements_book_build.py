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
 * @date 29 Oct 2020
 * @brief Puthon tool for converting itch to ore
 */
"""

import extractor as extr
import tempfile
import numpy as np
from datetime import timedelta
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def main_live(ore_path, symbols, out_path, with_time):
    graph = extr.system.comp_graph()
    op = graph.features

    if with_time:
        time_upds, *upds = op.ore_live_split(ore_path, symbols, True)
    else:
        upds = op.ore_live_split(ore_path, symbols)
    levels = [op.book_build(upd, 1) for upd in upds]

    all_levels = op.join(*levels, "ticker", extr.Array(extr.Char, 7), symbols)

    op.csv_record(all_levels, out_path)

    t = op.timer(timedelta(seconds=1))

    if with_time:
        time_msgs = op.book_msg(time_upds, 'time')
        time_acc = op.accumulate(time_msgs)

    def stop_context(frame):
        if with_time:
            as_pd_time = extr.result_as_pandas(time_acc)

            np.testing.assert_array_equal(as_pd_time['seconds'],
                                          np.array(['1970-01-01T00:00:10.000000000',
                                                    '1970-01-01T00:00:11.000000000',
                                                    '1970-01-01T00:00:12.000000000'],
                                                   dtype='datetime64[ns]'))
        raise RuntimeError('stopping context intentionally from test')

    graph.callback(t, stop_context)

    graph.stream_ctx().run_live()


def main(ore_path, symbols, out_path):
    graph = extr.system.comp_graph()
    op = graph.features

    upds = op.book_play_split(ore_path, symbols)
    levels = [op.book_build(upd, 1) for upd in upds]

    all_levels = op.join(*levels, "ticker", extr.Array(extr.Char, 7), symbols)

    op.csv_record(all_levels, out_path)

    graph.stream_ctx().run()


if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))
    with tempfile.NamedTemporaryFile() as out_file:
        main(os.path.join(src_dir, "data/synthetic_announcements.ore"), ("BTC-USD", "ETH-USD"), out_file.name)
        extr.flush()
        extr.assert_numdiff(out_file.name, os.path.join(src_dir, "data/synth_top_of_book.base.csv"))

    with tempfile.NamedTemporaryFile() as out_file:
        main(os.path.join(src_dir, "data/synthetic_announcements.ore"), ("ETH-USD", "BTC-USD"), out_file.name)
        extr.flush()
        extr.assert_numdiff(out_file.name, os.path.join(src_dir, "data/synth_top_of_book.base.csv"))

    with tempfile.NamedTemporaryFile() as out_file:
        try:
            main_live(os.path.join(src_dir, "data/synthetic_announcements_live.ore"),
                      ("ETH-USD", "BTC-USD"), out_file.name, False)
        except RuntimeError as e:
            assert len(e.args) == 1
            assert e.args[0] == 'stopping context intentionally from test'
        else:
            raise RuntimeError("Expected to fail to stop context")
        extr.flush()
        extr.assert_numdiff(out_file.name, os.path.join(src_dir, "data/synth_top_of_book.base.csv"))

    with tempfile.NamedTemporaryFile() as out_file:
        try:
            main_live(os.path.join(src_dir, "data/synthetic_announcements_live.ore"),
                      ("ETH-USD", "BTC-USD"), out_file.name, True)
        except RuntimeError as e:
            assert len(e.args) == 1
            assert e.args[0] == 'stopping context intentionally from test'
        else:
            raise RuntimeError("Expected to fail to stop context")
        extr.flush()
        extr.assert_numdiff(out_file.name, os.path.join(src_dir, "data/synth_top_of_book.base.csv"))

    with tempfile.NamedTemporaryFile() as out_file:
        try:
            main_live(os.path.join(src_dir, "data/synthetic_announcements_live.ore"),
                      ("BTC-USD", "ETH-USD"), out_file.name, False)
        except RuntimeError as e:
            assert len(e.args) == 1
            assert e.args[0] == 'stopping context intentionally from test'
        else:
            raise RuntimeError("Expected to fail to stop context")
        extr.flush()
        extr.assert_numdiff(out_file.name, os.path.join(src_dir, "data/synth_top_of_book.base.csv"))
