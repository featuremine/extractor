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
 * @file book_cross_validate.py
 * @author Andres Rangel
 * @date 7 Jul 2020
 * @brief Cross validation test script
 */
"""

import argparse
from book_build import book_cross_validation

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--imnts", help="Instruments", nargs="+")
    parser.add_argument("--ore", help="Ore file path")
    args = parser.parse_args()

    book_cross_validation(args.ore, args.imnts)
