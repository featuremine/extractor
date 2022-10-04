import msgpack as mp
import argparse
from ytp_base import peer
from time import sleep


def run(dump, out):
    if out:
        mp.pack([1, 1, 1], out)
        mp.pack([], out)
    while True:
        msg = ytp_peer.read()
        if msg is None:
            sleep(0.0001)
            continue
        if dump:
            print(msg[0], mp.unpackb(msg[1]))
        if out:
            out.write(msg[1])


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Replay ORE file into ytp')
    parser.add_argument("--ore", help="ORE file name", required=False)
    parser.add_argument("--dump", help="ORE file name", action='store_true', required=False)
    parser.add_argument("--transact", help="ytp transact file", required=True)
    args = parser.parse_args()

    dump = args.dump is None or args.dump
    ytp_peer = peer(args.transact)

    version = [1, 1, 1]
    if args.ore:
        with open(args.ore, "wb") as out:
            run(dump, out)
    else:
        run(dump, out=None)
