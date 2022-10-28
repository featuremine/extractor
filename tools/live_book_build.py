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
from datetime import datetime, timedelta
import extractor as extr
import pytz
import gzip
import json
import sys
import tempfile
import itertools
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--yamal", help="Yamal file", required=True)
    parser.add_argument("--imnts", help="Comma separated instrument list", required=True)
    parser.add_argument("--levels", help="Number of levels to display", type=int, required=False, default=5)
    args = parser.parse_args()

    imnts = tuple(args.imnts.split(','))

    graph = extr.system.comp_graph()
    op = graph.features
    upds = op.ore_live_split(args.yamal, imnts)
    levels = [op.book_build(upd, args.levels) for upd in upds]
    joint_stream = op.join(*levels, "ticker", extr.Array(extr.Char, 16), imnts)
    trigger = op.trigger(joint_stream)
    out_stream = op.combine(trigger, tuple(), joint_stream, tuple())
    graph.callback(out_stream, lambda frame: print(frame))

    graph.stream_ctx().run_live()
