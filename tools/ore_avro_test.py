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
 * @file ore_avro_test.py
 * @author Andres Rangel
 * @date 15 Jan 2019
 * @brief Python script to validate Avro spec for Ore
 */
"""

from avro import schema, datafile, io

OUTFILE_NAME = 'ore.avro'
INPUT_SCHEMA_NAME = 'ore.avsc'
SCHEMA_STR = None


def main():
    with open(INPUT_SCHEMA_NAME, "r+") as f:
        SCHEMA_STR = f.read()

    if SCHEMA_STR is None:
        return

    SCHEMA = schema.parse(SCHEMA_STR)

    # Create a 'record' (datum) writer
    rec_writer = io.DatumWriter(SCHEMA)

    # Create a 'data file' (avro file) writer
    df_writer = datafile.DataFileWriter(
        open(OUTFILE_NAME, 'wb'),
        rec_writer,
        writers_schema=SCHEMA
    )

    # header
    df_writer.append({"version": [1, 1, 0],
                      "symbology": [
        {'symbol': '5_YEAR', 'price_tick': 256},
        {'symbol': '7_YEAR', 'price_tick': 256},
        {'symbol': '2_YEAR', 'price_tick': 256}
    ]
    })

    # time
    df_writer.append({"receive": 355351531})

    # add
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "price": 512,
                      "qty": 200,
                      "is_bid": True})

    # insert
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "priority": 3,
                      "price": 512,
                      "qty": 200,
                      "is_bid": True})

    # position
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "position": 55,
                      "price": 512,
                      "qty": 200,
                      "is_bid": True})

    # cancel
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "qty": 200})

    # delete
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "deleted_order_id": 365})

    # modify
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "new_order_id": 366,
                      "price": 512,
                      "qty": 200})

    # execute
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365})

    # execute_px
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "trade_price": 512})

    # execute_partial
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "qty": 200})

    # execute_partial_px
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "trade_price": 512,
                      "qty": 200})

    # trade, no decorator
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "trade_price": 512,
                      "qty": 200})

    # trade, with \0 filled decorator
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "trade_price": 512,
                      "qty": 200,
                      "decorator": "\0\0\0\0\0\0\0\0"})

    # trade, with test decorator
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "trade_price": 512,
                      "qty": 200,
                      "decorator": "test"})

    # status
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "passive_order_id": 300,
                      "price": 512,
                      "state": 1,
                      "passive_is_bid": False})

    # control
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "uncross": 32,
                      "command": ' '})

    # set
    df_writer.append({"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "price": 512,
                      "qty": 300,
                      "is_bid": False})

    # file
    df_writer.append({"header": {"version": [1, 1, 0],
                                 "symbology": [
        {'symbol': '5_YEAR', 'price_tick': 256},
        {'symbol': '7_YEAR', 'price_tick': 256},
        {'symbol': '2_YEAR', 'price_tick': 256}
    ]
    }, "time": {"receive": 355351531},
        "messages": [{"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "price": 512,
                      "qty": 200,
                      "is_bid": True},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "priority": 3,
                      "price": 512,
                      "qty": 200,
                      "is_bid": True},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "position": 55,
                      "price": 512,
                      "qty": 200,
                      "is_bid": True},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "qty": 200},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "deleted_order_id": 365},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "new_order_id": 366,
                      "price": 512,
                      "qty": 200},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "trade_price": 512},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "qty": 200},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "order_id": 365,
                      "trade_price": 512,
                      "qty": 200},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "trade_price": 512,
                      "qty": 200},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "trade_price": 512,
                      "qty": 200,
                      "decorator": "\0\0\0\0\0\0\0\0"},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "trade_price": 512,
                      "qty": 200,
                      "decorator": "test"},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "passive_order_id": 300,
                      "price": 512,
                      "state": 1,
                      "passive_is_bid": False},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "uncross": 32,
                      "command": ' '},
                     {"receive": 355351531,
                      "vendor_offset": 0,
                      "vendor_seqno": 0,
                      "batch": 0,
                      "imnt_id": 1,
                      "price": 512,
                      "qty": 300,
                      "is_bid": False}
                     ]
    })

    df_writer.close()


if __name__ == "__main__":

    main()
