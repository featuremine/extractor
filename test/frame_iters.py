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
 * @file frame_iters.py
 * @author Andrus Suvalov
 * @date 15 May 2020
 * @brief Python test for accumulate operator with resetting argument
 */
"""

import extractor as extr
from datetime import timedelta
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    upd = op.clock_timer(timedelta(seconds=1), timedelta(seconds=100), timedelta(seconds=1))
    reset = op.clock_timer(timedelta(seconds=10), timedelta(seconds=100), timedelta(seconds=10))
    updates = op.accumulate(upd, reset)
    frame_ref = graph.get_ref(updates)

    try:
        for subframe in frame_ref:
            assert False
    except RuntimeError:
        pass

    count = timedelta(seconds=1)

    def updates_validation(frame):
        global count
        for subframe in frame:
            for k, v in subframe:
                assert k == "Timestamp" or k == "actual" or k == "scheduled"
                assert v == count
            count += timedelta(seconds=1)
        count -= timedelta(seconds=10)
        for subframe in frame:
            for k, v in subframe:
                assert k == "Timestamp" or k == "actual" or k == "scheduled"
                assert v == count
            count += timedelta(seconds=1)

    graph.callback(updates, updates_validation)
    graph.stream_ctx().run()
