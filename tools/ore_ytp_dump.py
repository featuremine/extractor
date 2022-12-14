import msgpack as mp
import argparse
from yamal import ytp
import json

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='dump ytp ORE channel')
    parser.add_argument("--ytp", help="ytp input file", required=True)
    parser.add_argument("--channel", help="The channel or channel prefix", required=True)
    args = parser.parse_args()

    seq = ytp.sequence(args.ytp, readonly=True)

    def seq_clbck(peer, channel, time, data):
        print(json.dumps(mp.unpackb(data)))

    seq.data_callback(args.channel, seq_clbck)

    try:
        while seq.poll():
            pass
    except BrokenPipeError:
        pass
