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
from ytp_base import peer

if __name__ == "__main__":

    # Simulation

    with open("../test/data/synthetic_announcements.ore", "wb") as out:
        mp.pack([1, 1, 2], out)
        mp.pack([
            {'symbol': 'BTC-USD', 'price_tick': 200, 'qty_tick': 100},
            {'symbol': 'ETH-USD', 'price_tick': 100}
        ], out)
        mp.pack([0, 10], out)
        mp.pack([15, 10, 0, 0, 0, 0, "BTC-USD", 100], out)
        mp.pack([15, 11, 0, 0, 0, 1, "ETH-USD", 50, 100], out)
        mp.pack([1, 11, 0, 0, 0, 1, 0, 125, 125, True], out)
        mp.pack([0, 11], out)
        mp.pack([1, 10, 0, 0, 0, 0, 0, 125, 125, True], out)
        mp.pack([0, 12], out)
        mp.pack([1, 11, 0, 0, 0, 1, 0, 200, 200, False], out)
        mp.pack([1, 10, 0, 0, 0, 0, 0, 200, 200, False], out)

    # Live

    with open("../test/data/synthetic_announcements_live.ore", "wb") as out:
        ytp_peer = peer(out.name)
        ytp_peer.write(0x100, mp.packb([0, 10]))
        ytp_peer.write(0x100, mp.packb([15, 10, 0, 0, 0, 0, "BTC-USD", 100]))
        ytp_peer.write(0x100, mp.packb([15, 11, 0, 0, 0, 1, "ETH-USD", 50, 100]))
        ytp_peer.write(0x100, mp.packb([1, 11, 0, 0, 0, 1, 0, 125, 125, True]))
        ytp_peer.write(0x100, mp.packb([0, 11]))
        ytp_peer.write(0x100, mp.packb([1, 10, 0, 0, 0, 0, 0, 125, 125, True]))
        ytp_peer.write(0x100, mp.packb([0, 12]))
        ytp_peer.write(0x100, mp.packb([1, 11, 0, 0, 0, 1, 0, 200, 200, False]))
        ytp_peer.write(0x100, mp.packb([1, 10, 0, 0, 0, 0, 0, 200, 200, False]))
