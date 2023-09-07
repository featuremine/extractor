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
 * @file custom_book_play_split.py
 * @date 23 Aug 2021
 * @brief Book building python sample
 */
"""

import extractor as extr
from argparse import ArgumentParser


def print_frame(frame):
    print(frame)


if __name__ == "__main__":
    parser = ArgumentParser()
    parser.add_argument("--input", help="Input file path")
    parser.add_argument('--dump', action='store_true')
    parser.set_defaults(dump=False)

    args = parser.parse_args()

    extr.system.load_ext("book_play_split")

    graph = extr.system.comp_graph()
    op = graph.features

    upds = op.book_play_split_custom(args.input, ("BTC-USD",))
    for upd in upds:
        top_of_book = op.book_build(upd, 1)

        if args.dump:
            # Print updates for the top of book
            graph.callback(top_of_book, print_frame)

    graph.stream_ctx().run()
