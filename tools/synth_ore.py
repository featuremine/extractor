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
 * @file synth_ore.py
 * @author Andres Rangel
 * @date 11 Jan 2018
 * @brief Synthetic ore file generator
 */
"""

import msgpack as mp

if __name__ == "__main__":
    with open("../test/data/synth_book.test.ore", "wb") as out:
        mp.pack([1, 1, 0], out)
        mp.pack([
            {'symbol': '2_YEAR', 'price_tick': 256},
            {'symbol': '3_YEAR', 'price_tick': 256}
        ], out)
        mp.pack([0, 360054], out)
        mp.pack([1, 360055000000000, 0, 0, 0, 1, 300, 54, 200, True], out)
        mp.pack([2, 360056000000000, 0, 0, 0, 1, 301, 5, 54, 200, True], out)
        mp.pack([3, 360057000000000, 0, 0, 0, 1, 302, 0, 54, 200, True], out)
        mp.pack([4, 360058000000000, 0, 0, 0, 1, 300, 200, True], out)
        mp.pack([5, 360059000000000, 0, 0, 0, 1, 301], out)
        mp.pack([6, 360060000000000, 0, 0, 0, 1, 302, 303, 54, 250], out)  # remove and add
        mp.pack([7, 360061000000000, 0, 0, 0, 1, 303], out)
        mp.pack([1, 360062000000000, 0, 0, 0, 1, 304, 54, 200, True], out)
        mp.pack([8, 360063000000000, 0, 0, 0, 1, 304, 54], out)
        mp.pack([1, 360064000000000, 0, 0, 0, 1, 305, 54, 200, True], out)
        mp.pack([9, 360063000000000, 0, 0, 0, 1, 305, 100], out)
        mp.pack([10, 360064000000000, 0, 0, 0, 1, 305, 54, 50], out)
        mp.pack([11, 360065000000000, 0, 0, 0, 1, 54, 25], out)
        mp.pack([11, 360066000000000, 0, 0, 0, 1, 54, 25, "\0\0\0\0\0\0\0\0"], out)
        mp.pack([1, 360067000000000, 0, 0, 0, 1, 306, 54, 200, True], out)
        mp.pack([1, 360067000000000, 0, 0, 0, 1, 307, 54, 200, False], out)
        mp.pack([12, 360068000000000, 0, 0, 0, 1, 306, 54, 3, True], out)
        mp.pack([13, 360069000000000, 0, 0, 0, 1, 4], out)
        mp.pack([14, 360070000000000, 0, 0, 0, 1, 54, 100, True], out)
        mp.pack([13, 360071000000000, 0, 0, 0, 1, 4, ord('C')], out)
