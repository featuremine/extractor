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
 * @file ore_ytp_decode.py
 * @author Ivan Gonzalez
 * @date 18 Jan 2022
 * @brief Python test to extract a ytp channel into a book
 */
"""
from datetime import datetime, timedelta
import extractor as extr
from yamal import ytp
import os
import shutil

src_dir = os.path.dirname(os.path.realpath(__file__))

ytp_file = 'ytpseq_ore_bbo.ytp'
# copy the ytp file, because it is overwritten
ytp_file_cpy = ytp_file + '.cpy'
bbos_base_file = os.path.join(src_dir, 'data', 'ore_ytp_decode.base.txt')
bbos_file = os.path.join(src_dir, 'data', 'ore_ytp_decode.test.txt')

bbos_cnt = 40


def print_bbos(x):
    global bbos_file, bbos_base_file, bbos_cnt, ytp_file_cpy
    try:
        f = open(bbos_file, "a")
        print(x, file=f)
        f.close()
    except FileNotFoundError as e:
        print("File not found")
    except RuntimeError as e:
        print("RuntimeError:", str(e))

    bbos_cnt -= 1
    if bbos_cnt < 1:
        extr.assert_base(bbos_base_file, bbos_file)
        if os.path.exists(bbos_file):
            os.remove(bbos_file)
        if os.path.exists(ytp_file_cpy):
            os.remove(ytp_file_cpy)
        exit()


if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    if os.path.exists(bbos_file):
        os.remove(bbos_file)

    shutil.copy(ytp_file, ytp_file_cpy)
    seq = ytp.sequence(ytp_file_cpy)
    peer = seq.peer('peer_producer')
    ch = peer.channel(1000, "channels/imnts/NASDAQOMX/APLE")
    op.ytp_sequence(seq, timedelta(milliseconds=1))
    bookupd = op.ore_ytp_decode(ch)
    decoded = op.decode_data(bookupd)
    level = op.book_build(decoded, 1)

    decoded_time = op.decode_receive(bookupd)
    level_receive = op.asof(decoded_time, level)
    bbo_book_combined = op.combine(
        level, (
            ("ask_ord_0", "ask_ord_0"),
            ("ask_prx_0", "ask_prx_0"),
            ("ask_shr_0", "ask_shr_0"),
            ("bid_ord_0", "bid_ord_0"),
            ("bid_prx_0", "bid_prx_0"),
            ("bid_shr_0", "bid_shr_0")
        ),
        level_receive, (("time", "time"),)
    )

    graph.callback(bbo_book_combined, print_bbos)

    graph.stream_ctx().run_live()
