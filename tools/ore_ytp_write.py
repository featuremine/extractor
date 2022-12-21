import msgpack as mp
import argparse
from yamal import ytp
import json
import sys
import lzma

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='write ytp ORE channel')
    parser.add_argument("--ytp", help="ytp input file", required=True)
    parser.add_argument("--channel", help="The channel or channel prefix", required=True)
    parser.add_argument("--peer", help="The peer name", required=True)
    parser.add_argument("--ore", help="The ore file", required=True)
    parser.add_argument("--decompress", default=False, help="Decompress input using LZMA", action='store_true')
    args = parser.parse_args()

    seq = ytp.sequence(args.ytp)
    peer = seq.peer(args.peer)
    ch = peer.channel(1000, args.channel)
    stream = peer.stream(ch)

    for line in open(args.ore, "r") if args.decompress is False else lzma.open(args.ore, "r"):
        stream.write(0, mp.packb(json.loads(line)))
