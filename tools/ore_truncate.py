import msgpack as mp
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Truncate a ORE file')
    parser.add_argument("--input", help="Input ORE file name", required=True)
    parser.add_argument("--output", help="Output ORE file name", required=True)
    parser.add_argument("--size", help="The file is truncated to that number of transactions", required=True)
    args = parser.parse_args()

    with open(args.input, "rb") as input_file:
        with open(args.output, "wb") as output_file:
            unpacker = mp.Unpacker(input_file, raw=False)

            version = unpacker.unpack()
            mp.pack(version, output_file)

            symbology = unpacker.unpack()
            mp.pack(symbology, output_file)

            transaction_count = int(args.size)
            while transaction_count > 0:
                transaction_count -= 1
                try:
                    t = unpacker.unpack()
                except OutOfData:
                    break

                mp.pack(t, output_file)
