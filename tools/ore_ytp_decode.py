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
 * @brief Python tool to extract a ytp channel into a book
 */
"""
from datetime import datetime, timedelta
import extractor as extr
import argparse
import ytp

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--ytp", help="YTP file", required=True)
    parser.add_argument("--peer", help="YTP Peer", required=False, default="peer_writer")
    parser.add_argument("--channel", help="YTP channel to extract data", required=True)
    parser.add_argument("--levels", help="Number of levels to display", type=int, required=False, default=5)
    parser.add_argument(
        "--license",
        help="Extractor license (defaults to '../test/test.lic' if not provided)",
        required=False,
        default="../test/test.lic")
    args = parser.parse_args()

    extr.set_license(args.license)
    graph = extr.system.comp_graph()
    op = graph.features

    seq = ytp.sequence(args.ytp)
    peer = seq.peer(args.peer)
    ch = peer.channel(1000, args.channel)  # "channels/imnts/NYSE/APLE"
    op.ytp_sequence(seq, timedelta(milliseconds=1))
    bookupd = op.ore_ytp_decode(ch)
    decoded = op.decode_data(bookupd)
    decoded_time = op.decode_receive(bookupd)
    level = op.book_build(decoded, args.levels)

    graph.callback(level, lambda frame: print(frame))

    graph.callback(decoded_time, lambda frame: print(frame))

    graph.stream_ctx().run_live()
