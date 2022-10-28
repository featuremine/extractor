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
 * @file json2ore.py
 * @author Maxim Trokhimtchouk
 * @date 26 Oct 2018
 * @brief Python tool for converting itch to ore
 */
"""
import pandas as pd
from pandas.testing import assert_frame_equal
from datetime import datetime, timedelta
import msgpack as mp
import gzip
import json
import sys
import tempfile
import itertools
import extractor as extr
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

global time_obj
time_obj = None

version = [1, 1, 0]

# Ore file is composed of the version numbers, header and messages sequence.

# The version numbers is written as MessagePack array consisting of
# major, minor and sub version number.

# The file header is a MessagePack array of instrument properties dictionary.
# Currently instrument properties include symbols and price denumerator field.

# Each book update message is
# a MessagePack array, it is composed of the common message header elements
# and other elements depending on the message type.

# Message Header
# Fields:
#   message type (
#       0 - time
#       1 - order add
#       2 - order insert
#       3 - order position
#       4 - order cancelled
#       5 - order delete
#       6 - order modify
#       7 - complete execution
#       8 - complete execution at a price
#       9 - partial fill
#       10 - partial fill at a  price
#       11 - off book trade
#       12 - status message
#       13 - book control message
#       14 - level set message
#   )
# The rest of the fields apply for messages other than time message:
#   receive time epoch (nanoseconds only)
#   vendor offset (nanoseconds difference between receive and vendor time)
#   vendor sequence number (zero if unavailable)
#   batch flag (0 - no batch, 1 - batch begin, 2 - batch end)
#   instrument index (mp integer matching the order of the symbol in the file header)

# Order Cancel Message
#   id (mp integer)
#   cancelled quantity (mp integer)

# Time Message
#   receive time epoch (seconds only)


def parse_tme(out, idx_map, obj):
    global time_obj
    time_obj = obj
    mp.pack([0, obj['second']], out)

# Order Add Message
#   id (mp integer)
#   price (mp integer numerator part only)
#   quantity (mp integer)
#   is_bid (mp bool)


def parse_add(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([1, obj['nanoseconds'], 0, 0, 0, idx,
             obj['orderId'], obj['price'],
             obj['quantity'], obj['side'] == 'B'], out)

# Order Insert Message
#   id (mp integer)
#   priority (mp integer)
#   price (mp integer numerator part only)
#   quantity (mp integer)
#   is_bid (mp bool)


def parse_ins(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([2, obj['nanoseconds'], 0, 0, 0, idx,
             obj['orderId'], obj['priority'], obj['price'],
             obj['quantity'], obj['side'] == 'B'], out)

# Order Position Message
# Note the difference between inserting and positioning the orders is as follows:
# Positioning the order places the order at the prescribed position in the order queue.
# Inserting orders is based on comparing a priority property of the order which does
# not change with the live-time of the order, while position of the order will change.
#   id (mp integer)
#   position starting from 0 (mp integer)
#   price (mp integer numerator part only)
#   quantity (mp integer)
#   is_bid (mp bool)


def parse_pos(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([3, obj['nanoseconds'], 0, 0, 0, idx,
             obj['orderId'], obj['orderBookPosition'] - 1, obj['price'],
             obj['quantity'], obj['side'] == 'B'], out)

# Order Cancel Message
#   id (mp integer)
#   quantity (mp integer)


def parse_cxl(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([4, obj['nanoseconds'], 0, 0, 0, idx, obj['orderId'],
             obj['quantity']], out)

# Order Delete Message
#   id (mp integer)


def parse_del(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([5, obj['nanoseconds'], 0, 0, 0, idx, obj['orderId']], out)

# Order Modify Message
#   id (mp integer)
#   newId (mp integer)
#   newPrice (mp integer numerator part only)
#   newQuantity (mp integer)


def parse_mod(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([6, obj['nanoseconds'], 0, 0, 0, idx, obj['orderId'],
             obj['newOrderId'], obj['price'], obj['quantity']], out)

# Order Executed Whole Message
#   id (mp integer)


def parse_exe(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([7, obj['nanoseconds'], 0, 0, 0, idx,
             obj['orderId']], out)

# Order Executed Whole at a Price Message
#   id (mp integer)
#   tradePrice (mp integer numerator part only)


def parse_epx(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([8, obj['nanoseconds'], 0, 0, 0, idx,
             obj['orderId'], obj['tradePrice']], out)

# Order Executed Partially Message
#   id (mp integer)
#   quantity (mp integer)


def parse_fld(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([9, obj['nanoseconds'], 0, 0, 0, idx,
             obj['orderId'], obj['executedQuantity']], out)

# Order Executed Partially at a Price Message
#   id (mp integer)
#   tradePrice (mp integer numerator part only)
#   quantity (mp integer)


def parse_fpx(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([10, obj['nanoseconds'], 0, 0, 0, idx,
             obj['orderId'], obj['tradePrice'],
             obj['executedQuantity']], out)

# Off book trade message
#   tradePrice (mp integer numerator part only)
#   quantity (mp integer)


def parse_trd(out, idx_map, obj):
    idx = idx_map[obj['orderBookId']]
    mp.pack([11, obj['nanoseconds'], 0, 0, 0, idx,
             obj['tradePrice'], obj['quantity'], "\0\0\0\0\0\0\0\0"], out)

# Status message
#   order id (mp integer, could be ignored for some states)
#   price (mp integer numerator part only, could be ignored for some states)
#   state id (mp integer, user determined stated)
#   is_bid (mp bool, could be ignored for some states)


def parse_wrk(out, idx_map, obj):
    idx = idx_map[obj['orderBookId']]
    mp.pack([12, obj['nanoseconds'], 0, 0, 0, idx, obj['passiveOrderId'],
             obj['price'], obj['state'], obj['passiveSide'] == 'B'], out)

# Book control message
#   uncross (mp bool)


def parse_ctl(out, idx_map, obj):
    idx = idx_map[obj['orderBookId']]
    mp.pack([13, obj['nanoseconds'], 0, 0, 0, idx, True], out)

# Level set message
#   price (mp integer numerator part only)
#   quantity (mp integer)
#   is_bid (mp bool)


def parse_set(out, idx_map, obj):
    bookid = obj['orderBookId']
    idx = idx_map[bookid]
    mp.pack([1, obj['nanoseconds'], 0, 0, 0, idx,
             obj['price'], obj['quantity'], obj['side'] == 'B'], out)


callbacks = {
    'R': None,
    'L': None,
    'O': None,
    'Z': None,
    'S': None,
    'T': parse_tme,
    'A': parse_pos,
    'X': parse_cxl,
    'D': parse_del,
    'U': parse_mod,
    # not sure what W means here
    'W': parse_wrk,
    'E': parse_fld,
    'C': parse_epx,
    # not sure what would be filled in part
    'P': parse_trd,
}

# Questions / Discussion
# It is better to follow itch closer to simplify conversion and compress data better.
# What would amount to a partial fill message? Does not seem clear from the sample.
# Can we have a bigger sample to parse through more message types?
# How do we mark the difference between cross and non-cross trades?
# Time message has to be the first message, since other messages before the first Time
# will not have a well defined receive time.


def parse(out, idx_map, obj):
    try:
        callback = callbacks[obj['messageType']]
        if callback:
            callback(out, idx_map, obj)
    except BaseException:
        print("error for the following line:")
        print(line)
        raise


def write_header(out, symbology):
    mp.pack(version, out)
    mp.pack(symbology, out)


if __name__ == "__main__":
    fname = os.path.join(src_dir, "../test/data/itch.20150901.partial.json.gz")
    symbology = []
    idx_map = {}
    with open(os.path.join(src_dir, "../test/data/json_book.test.ore"), "wb") as out:
        with (gzip.GzipFile(fname) if fname.endswith('.gz') else open(fname)) as file:
            # First we need to find all of the symbol definitions
            fileiter = file.readlines()

            for line in fileiter:
                obj = json.loads(line)
                type = obj['messageType']
                if type not in ['R', 'T', 'L', 'O', 'Z']:
                    break
                if type == 'R':
                    symb = {'symbol': obj['longName'], 'price_tick': obj['numPriceDecimals']}
                    idx_map[obj['orderBookId']] = len(symbology)
                    symbology.append(symb)
                elif type == 'T':
                    time_obj = obj

            write_header(out, symbology)
            if (time_obj is None):
                raise "time message has not been found in the begining of the file"

            parse_tme(out, idx_map, time_obj)

            # Then we parse the rest of the file
            for line in fileiter:
                obj = json.loads(line)
                parse(out, idx_map, obj)

    extr.assert_base(os.path.join(src_dir, "../test/data/json_book.test.ore"),
                     os.path.join(src_dir, "../test/data/book.base.ore"))
