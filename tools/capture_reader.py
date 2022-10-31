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
 * @file log_reader.py
 * @author Andres Rangel
 * @date 16 Aug 2018
 * @brief Log reader python 3 script for messagepack encoded files
 */
"""

import msgpack
import sys


def main():

    names = {}
    comp_names = {}
    comp_params = {}
    if len(sys.argv) != 2 or sys.argv[1] == "--help":
        print("Log reader.")
        print("Usage: python3", sys.argv[0], "logfile")
        return

    try:
        f = open(sys.argv[1], "rb")

        g_size = int(f.readline().decode())

        print("Graph size:", g_size)

        i = 0

        while i < g_size:

            names[i] = f.readline().decode().strip()
            print("\nName", names[i], "found for feature", g_size - i - 1)
            comp_names[i] = f.readline().decode().strip()
            print("Comp name", comp_names[i], "found for feature", g_size - i - 1)

            inpc = int(f.readline().decode().strip())

            j = 0
            while j < inpc:
                inp = f.readline().decode().strip()
                print("Input %d: %s" % (j, inp))
                j += 1

            comp_params[i] = f.readline().decode().strip()

            if comp_params[i] and comp_params[i] != "tuple()":

                args_count = 1

                for j in comp_params[i]:
                    if j == ',':
                        args_count += 1

                print("Args: ", comp_params[i])

                pass_count = 0

                while pass_count < args_count:
                    print("Reading arg:", f.readline().decode().strip())
                    pass_count += 1

            print("Feature has callbacks:", f.readline().decode().strip())

            print("Feature data is required:", f.readline().decode().strip())

            i += 1

        print()

        unpacker = msgpack.Unpacker(f, raw=False)

        for unpacked in unpacker:
            print(unpacked)

        f.close()
    except FileNotFoundError as e:
        print("File not found")


if __name__ == "__main__":
    main()
