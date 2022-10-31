import msgpack as mp
import argparse
from ytp_base import peer

from time import monotonic_ns as current_ns
from time import monotonic as current_sec


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Replay ORE file into ytp')
    parser.add_argument("--ore", help="ORE file name", required=True)
    parser.add_argument("--transact", help="ytp transact file", required=True)
    args = parser.parse_args()

    ore_file = open(args.ore, "rb")
    ytp_peer = peer(args.transact)

    unpacker = mp.Unpacker(ore_file, raw=False)

    version = unpacker.unpack()
    symbology = unpacker.unpack()

    if version[0] != version[0] or version[1] > version[1]:
        raise RuntimeError(
            "ore_reader supports Ore 1.1.0, Input file uses", version)

    symbols = {}
    for i, symb in enumerate(symbology):
        symbols[symb['symbol']] = {
            'bookid': i,
            'symbol': symb['symbol'],
            'price_tick': symb['price_tick']
        }

    ytp_peer.write(0x100, mp.packb([0, int(current_sec())]))
    for symb, info in symbols.items():
        ytp_peer.write(0x100, mp.packb([15, 0, 0, 0, 0, info['bookid'], symb, info['price_tick']]))

    data_sec = 0
    data_ns = 0
    offset_sec = 0
    offset_ns = 0

    while True:    # process the first message which should be time
        msg = unpacker.unpack()

        if msg[0] == 0:
            data_sec = msg[1]
            data_ns = data_sec * 1000000000
        else:
            data_ns = data_sec * 1000000000 + msg[1]

        if offset_sec == 0:
            offset_sec = int(current_sec()) - data_sec + 2
            offset_ns = offset_sec * 1000000000

        # wait loop
        while current_ns() < data_ns + offset_ns:
            pass

        if msg[0] == 0:
            msg[1] = data_sec + offset_sec

        ytp_peer.write(0x100, mp.packb(msg))
