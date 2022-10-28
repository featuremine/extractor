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
 * @file top_book_update.py
 * @author Andres Rangel
 * @date 31 Mar 2020
 * @brief Python test for book header operator
 */
"""
import extractor as extr
from extractor import result_as_pandas
from pandas.testing import assert_frame_equal
import pandas as pd
from numpy.testing import assert_array_equal, assert_array_almost_equal
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    upd = op.book_play_split(os.path.join(src_dir, "data/book.base.ore"), ("3_YEAR",))

    top_book = op.book_build(upd[0], 1)

    same_top = op.equal(top_book, op.tick_lag(top_book, 1))

    changed_top = op.logical_not(same_top)

    changed_top_res = changed_top.ask_ord_0 | changed_top.ask_prx_0 | changed_top.ask_shr_0 | changed_top.bid_ord_0 | changed_top.bid_prx_0 | changed_top.bid_shr_0

    top_book_update = op.filter_if(changed_top_res, top_book)

    header = op.book_header(upd[0])

    updates = op.combine(op.asof(header, top_book_update), tuple(), top_book_update, tuple())

    op.csv_record(updates, os.path.join(src_dir, "data/top_book_update.test.csv"))

    graph.stream_ctx().run()

    extr.flush()

    extr.assert_numdiff(os.path.join(src_dir, 'data/top_book_update.base.csv'),
                        os.path.join(src_dir, 'data/top_book_update.test.csv'))
