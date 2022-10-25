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
 * @file book_play_split.py
 * @author Ivan Gonzalez
 * @date 18 May 2022
 * @brief Operator test
 */
"""

import tempfile
import msgpack as mp
import extractor as extr
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def get_tempfile_name(some_id='ttt'):
    return os.path.join(tempfile.gettempdir(), next(tempfile._get_candidate_names()) + "_" + some_id)


def normal():
    ore_file_path = get_tempfile_name('normal')
    with open(ore_file_path, "wb") as ore_file:
        mp.pack([1, 1, 3], ore_file)
        mp.pack([
            {'symbol': '2_YEAR', 'price_tick': 256},
            {'symbol': '3_YEAR', 'price_tick': 256}
        ], ore_file)
        mp.pack([0, 360054], ore_file)
        mp.pack([1, 360055000000000, 0, 0, 0, 1, 300, 54, 200, True], ore_file)
        mp.pack([2, 360056000000000, 0, 0, 0, 1, 301, 5, 54, 200, True], ore_file)
        mp.pack([3, 360057000000000, 0, 0, 0, 1, 302, 0, 54, 200, True], ore_file)
        mp.pack([4, 360058000000000, 0, 0, 0, 1, 300, 200, True], ore_file)
        mp.pack([5, 360059000000000, 0, 0, 0, 1, 301], ore_file)
        mp.pack([6, 360060000000000, 0, 0, 0, 1, 302, 303, 54, 250], ore_file)  # remove and add
        mp.pack([7, 360061000000000, 0, 0, 0, 1, 303], ore_file)
        mp.pack([1, 360062000000000, 0, 0, 0, 1, 304, 54, 200, True], ore_file)
        mp.pack([8, 360063000000000, 0, 0, 0, 1, 304, 54], ore_file)
        mp.pack([1, 360064000000000, 0, 0, 0, 1, 305, 54, 200, True], ore_file)
        mp.pack([9, 360063000000000, 0, 0, 0, 1, 305, 100], ore_file)
        mp.pack([10, 360064000000000, 0, 0, 0, 1, 305, 54, 50], ore_file)
        mp.pack([11, 360065000000000, 0, 0, 0, 1, 54, 25], ore_file)
        mp.pack([11, 360066000000000, 0, 0, 0, 1, 54, 25, "\0\0\0\0\0\0\0\0"], ore_file)
        mp.pack([1, 360067000000000, 0, 0, 0, 1, 306, 54, 200, True], ore_file)
        mp.pack([1, 360067000000000, 0, 0, 0, 1, 307, 54, 200, False], ore_file)
        mp.pack([12, 360068000000000, 0, 0, 0, 1, 306, 54, 3, True], ore_file)
        mp.pack([13, 360069000000000, 0, 0, 0, 1, 4], ore_file)
        mp.pack([14, 360070000000000, 0, 0, 0, 1, 54, 100, True], ore_file)
        mp.pack([13, 360071000000000, 0, 0, 0, 1, 4, ord('C')], ore_file)
    graph = extr.system.comp_graph()
    op = graph.features
    synth = op.book_play_split(ore_file_path, ("2_YEAR", "3_YEAR",))
    graph.stream_ctx().run()
    os.remove(ore_file_path)


def header_error():
    ore_file_path = get_tempfile_name('header_error')
    wrong_version = ["0, 0, 0"]
    with open(ore_file_path, "wb") as ore_file:
        mp.pack(wrong_version, ore_file)
        mp.pack([
            {'symbol': '2_YEAR', 'price_tick': 256},
            {'symbol': '3_YEAR', 'price_tick': 256}
        ], ore_file)
        mp.pack([0, 360054], ore_file)
        mp.pack([1, 360055000000000, 0, 0, 0, 1, 300, 54, 200, True], ore_file)
        mp.pack([2, 360056000000000, 0, 0, 0, 1, 301, 5, 54, 200, True], ore_file)
        mp.pack([3, 360057000000000, 0, 0, 0, 1, 302, 0, 54, 200, True], ore_file)
    graph = extr.system.comp_graph()
    op = graph.features
    synth = op.book_play_split(ore_file_path, ("2_YEAR", "3_YEAR",))
    try:
        graph.stream_ctx().run()
    except RuntimeError as e:
        res = str(e).find("(book_play_split) could not read file version")
        assert res != -1
    else:
        assert False
    os.remove(ore_file_path)


def header_error_2():
    ore_file_path = get_tempfile_name('header_error_2')
    wrong_version = [0, 0, 0]
    with open(ore_file_path, "wb") as ore_file:
        mp.pack(wrong_version, ore_file)
        mp.pack([
            {'symbol': '2_YEAR', 'price_tick': 256},
            {'symbol': '3_YEAR', 'price_tick': 256}
        ], ore_file)
        mp.pack([0, 360054], ore_file)
        mp.pack([1, 360055000000000, 0, 0, 0, 1, 300, 54, 200, True], ore_file)
        mp.pack([2, 360056000000000, 0, 0, 0, 1, 301, 5, 54, 200, True], ore_file)
        mp.pack([3, 360057000000000, 0, 0, 0, 1, 302, 0, 54, 200, True], ore_file)
    graph = extr.system.comp_graph()
    op = graph.features
    synth = op.book_play_split(ore_file_path, ("2_YEAR", "3_YEAR",))
    try:
        graph.stream_ctx().run()
    except RuntimeError as e:
        res = str(e).find(
            "(book_play_split) FeatureMine Ore file version {} does not match Ore parser version ".format(
                str(wrong_version)[
                    1:-
                    1].replace(
                    ", ",
                    ".")))
        assert res != -1
    else:
        assert False
    os.remove(ore_file_path)


def symbol_error():
    ore_file_path = get_tempfile_name('symbol_error')
    with open(ore_file_path, "wb") as ore_file:
        mp.pack([1, 1, 3], ore_file)
        mp.pack([
            {'symbol_wrong': '2_YEAR', 'price_tick': 256},
            {'symbol': '3_YEAR', 'price_tick': 256}
        ], ore_file)
        mp.pack([0, 360054], ore_file)
        mp.pack([1, 360055000000000, 0, 0, 0, 1, 300, 54, 200, True], ore_file)
        mp.pack([2, 360056000000000, 0, 0, 0, 1, 301, 5, 54, 200, True], ore_file)
        mp.pack([3, 360057000000000, 0, 0, 0, 1, 302, 0, 54, 200, True], ore_file)
    graph = extr.system.comp_graph()
    op = graph.features
    synth = op.book_play_split(ore_file_path, ("2_YEAR", "3_YEAR",))
    try:
        graph.stream_ctx().run()
    except RuntimeError as e:
        res = str(e).find("(book_play_split) could not read header of the file {}".format(ore_file_path))
        assert res != -1
    else:
        assert False
    os.remove(ore_file_path)


def symbol_error_2():
    ore_file_path = get_tempfile_name('symbol_error_2')
    with open(ore_file_path, "wb") as ore_file:
        mp.pack([1, 1, 3], ore_file)
        mp.pack([
            {'symbol': '2_YEAR', 'price_tick': 256},
            {'symbol': '3_YEAR', 'price_tick': 256}
        ], ore_file)
        mp.pack([0, 360054], ore_file)
        mp.pack([1, 360055000000000, 0, 0, 0, 1, 300, 54, 200, True], ore_file)
        mp.pack([2, 360056000000000, 0, 0, 0, 1, 301, 5, 54, 200, True], ore_file)
        mp.pack([3, 360057000000000, 0, 0, 0, 1, 302, 0, 54, 200, True], ore_file)
    graph = extr.system.comp_graph()
    op = graph.features
    synth = op.book_play_split(ore_file_path, ("2_YEAR", "3_YEAR", "10_YEAR"))
    try:
        graph.stream_ctx().run()
    except RuntimeError as e:
        res = str(e).find("(book_play_split) could find symbol 10_YEAR in the header of file {}".format(ore_file_path))
        assert res != -1
    else:
        assert False
    os.remove(ore_file_path)


def parse_error():
    ore_file_path = get_tempfile_name('parse_error')
    with open(ore_file_path, "wb") as ore_file:
        mp.pack([1, 1, 3], ore_file)
        mp.pack([
            {'symbol': '2_YEAR', 'price_tick': 256},
            {'symbol': '3_YEAR', 'price_tick': 256}
        ], ore_file)
        mp.pack([0, 360054], ore_file)
        mp.pack([1, 360055000000000, 0, 0, 0, 1, 300, 54, 200, True], ore_file)
        mp.pack("jajasdjdasondasohn", ore_file)
    graph = extr.system.comp_graph()
    op = graph.features
    synth = op.book_play_split(ore_file_path, ("2_YEAR", "3_YEAR",))
    try:
        graph.stream_ctx().run()
    except RuntimeError as e:
        res = str(e).find(
            "error reading FM Ore file {}, parsing error (failed to read ORE message header, expecting array) occurred.".format(ore_file_path))
        assert res != -1
    else:
        assert False
    os.remove(ore_file_path)


def parse_error_2():
    ore_file_path = get_tempfile_name('parse_error')
    with open(ore_file_path, "wb") as ore_file:
        mp.pack([1, 1, 3], ore_file)
        mp.pack([
            {'symbol': '2_YEAR', 'price_tick': 256},
            {'symbol': '3_YEAR', 'price_tick': 256}
        ], ore_file)
        mp.pack([0, 360054], ore_file)
        mp.pack([1, 360055000000000, 0, 0, 0, 1, 300, 54, 200, True], ore_file)
        mp.pack([], ore_file)
    graph = extr.system.comp_graph()
    op = graph.features
    synth = op.book_play_split(ore_file_path, ("2_YEAR", "3_YEAR",))
    try:
        graph.stream_ctx().run()
    except RuntimeError as e:
        res = str(e).find(
            "error reading FM Ore file {}, parsing error (failed to read ORE message type) occurred.".format(ore_file_path))
        assert res != -1
    else:
        assert False
    os.remove(ore_file_path)


def parse_error_3():
    ore_file_path = get_tempfile_name('parse_error')
    with open(ore_file_path, "wb") as ore_file:
        mp.pack([1, 1, 3], ore_file)
        mp.pack([
            {'symbol': '2_YEAR', 'price_tick': 256},
            {'symbol': '3_YEAR', 'price_tick': 256}
        ], ore_file)
        mp.pack([0, 360054], ore_file)
        mp.pack([1, 360055000000000, 0, 0, 0, 1, 300, 54, 200, True], ore_file)
        mp.pack([2, 360056000000000], ore_file)
    graph = extr.system.comp_graph()
    op = graph.features
    synth = op.book_play_split(ore_file_path, ("2_YEAR", "3_YEAR",))
    try:
        graph.stream_ctx().run()
    except RuntimeError as e:
        res = str(e).find(
            "error reading FM Ore file {}, parsing error (failed to parse ORE message of type 2) occurred.".format(ore_file_path))
        assert res != -1
    else:
        assert False
    os.remove(ore_file_path)


if __name__ == "__main__":

    normal()
    header_error()
    header_error_2()
    symbol_error()
    symbol_error_2()
    parse_error()
    parse_error_2()
    parse_error_3()
