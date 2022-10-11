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
 * @file coinbase.py
 * @author Ivan Gonzalez
 * @date 28 Mar 2022
 * @brief Python test for the coinbase generated book
 */
"""
from datetime import datetime, timedelta
import extractor as extr
from yamal import ytp
import os
import shutil
import argparse

src_dir = os.path.dirname(os.path.realpath(__file__))
bulldozer_dir = os.path.join(src_dir, '..', '..', 'bulldozer')
ytp_file = os.path.join(bulldozer_dir, 'tests', 'data', 'coinbase_bulldozer.base.ytp')
book_base_file = os.path.join(src_dir, 'data', 'coinbase_bulldozer.base.csv')
book_test_file = os.path.join(src_dir, 'data', 'coinbase_bulldozer.test.csv')


book_entries = 5


def n_entries(x):
    global book_entries
    book_entries -= 1
    if book_entries < 1:
        exit()


if __name__ == "__main__":
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()
    op = graph.features

    if os.path.exists(book_test_file):
        os.remove(book_test_file)

    seq = ytp.sequence(ytp_file, readonly=True)
    peer = seq.peer('mypeer')
    ch = peer.channel(1000, "mychannel/imnts/coinbase/BTC-USD")
    op.ytp_sequence(seq, timedelta(milliseconds=1))
    bookupd = op.ore_ytp_decode(ch)
    decoded = op.decode_data(bookupd)
    decoded_time = op.decode_receive(bookupd)
    levels = op.book_build(decoded, 10)
    op.csv_record(levels, book_test_file)

    graph.callback(levels, n_entries)

    graph.stream_ctx().run_live()

    extr.flush()
    extr.assert_numdiff(book_base_file, book_test_file)

    if os.path.exists(book_test_file):
        os.remove(book_test_file)
