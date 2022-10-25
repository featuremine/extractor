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
 * @file accumulate.py
 * @author Andrus Suvalov
 * @date 14 May 2020
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

    upd1 = op.clock_timer(timedelta(seconds=1), timedelta(seconds=100), timedelta(seconds=2))
    upd2 = op.clock_timer(timedelta(seconds=2), timedelta(seconds=100), timedelta(seconds=2))
    reset = op.clock_timer(timedelta(seconds=10), timedelta(seconds=100), timedelta(seconds=10))
    acc1 = op.accumulate(upd1, reset)
    acc2 = op.accumulate(upd2, reset)

    join = op.join(acc1, acc2, "upd", extr.Array(extr.Char, 25), ("acc1", "acc2"))

    count1 = timedelta(seconds=1)
    count2 = timedelta(seconds=2)

    def updates_validation(frame):
        global count1
        global count2
        assert len(frame) == 5
        for i in range(len(frame)):
            if frame[0].upd == "acc1":
                assert frame[i].Timestamp == count1
                assert frame[i].actual == count1
                assert frame[i].scheduled == count1
                count1 += timedelta(seconds=2)
            elif frame[0].upd == "acc2":
                assert frame[i].Timestamp == count2
                assert frame[i].actual == count2
                assert frame[i].scheduled == count2
                count2 += timedelta(seconds=2)

    graph.callback(join, updates_validation)

    graph.stream_ctx().run()
