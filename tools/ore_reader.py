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
 * @file ore_reader.py
 * @author Andres Rangel
 * @date 9 Jan 2019
 * @brief Python tool to read Ore files and generate book
 */
"""

from book_build import BookBuilder
import extractor as extr
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def build_book():
    book_builder = BookBuilder(os.path.join(src_dir,
                                            "../test/data/book.base.ore"),
                               ['2_YEAR',
                                '3_YEAR',
                                '10_YEAR'],
                               os.path.join(src_dir,
                                            "../test/data/book_levels.ore_reader.test.csv"))
    book_builder.build_book()
    book_builder.dump_flush()


if __name__ == "__main__":
    build_book()
    extr.assert_numdiff(os.path.join(src_dir, "../test/data/book_levels.ore_reader.test.csv"),
                        os.path.join(src_dir, "../test/data/book_levels.python.base.csv"))
