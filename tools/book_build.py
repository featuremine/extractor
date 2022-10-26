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
 * @file book_build.py
 * @author Andres Rangel
 * @date 7 Jul 2020
 * @brief Python tool to read Ore files and generate book
 */
"""

import msgpack as mp
import itertools
import extractor as extr
import sys
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


class BookBuilder:

    def __init__(self, file, imnts, dump_file=None):
        self.version = [1, 1, 0]
        self.imnts = imnts
        self.symbols = {}
        self.orders = {}
        self.book = {}
        self.time_obj = None
        self.dump = True if dump_file is not None else False
        self.lines = 0
        self.file = file

        self.callbacks = [
            self.update_tme,
            self.update_add,
            self.update_ins,
            self.update_pos,
            self.update_cxl,
            self.update_del,
            self.update_mod,
            self.update_exe,
            self.update_epx,
            self.update_fld,
            self.update_fpx,
            None,
            None,
            None
        ]

        self.ore_file = open(self.file, "rb")

        self.unpacker = mp.Unpacker(self.ore_file, raw=False)

        [file_version, symbology] = self.parse_header(self.unpacker)

        if file_version[0] != self.version[0] or file_version[1] > self.version[1]:
            raise RuntimeError("ore_reader supports Ore 1.1.0, Input file uses", file_version)

        for i, symb in enumerate(symbology):
            self.symbols[symb['symbol']] = {
                'bookid': i,
                'symbol': symb['symbol'],
                'price_tick': symb['price_tick'],
                'qty_tick': symb['qty_tick'] if 'qty_tick' in symb else 1.0
            }

        for imnt in imnts:
            bookid = self.symbols[imnt]['bookid']
            self.orders[bookid] = {}
            self.book[bookid] = {'S': {}, 'B': {}, imnt: {}}

        self.update(self.unpacker.unpack())

        if (not self.time_obj):
            raise RuntimeError("File corrupted, time entry not found in file header")

        if self.dump:
            self.py_out = open(dump_file, 'w')
            print('time', 'ticker',
                  ','.join(["bid_prx_{0},bid_shr_{0},bid_ord_{0}".format(idx) for idx in range(0, 5)] +
                           ["ask_prx_{0},ask_shr_{0},ask_ord_{0}".format(idx) for idx in range(0, 5)]),
                  sep=',', file=self.py_out)

    def get_book(self, idx):
        return self.book[idx]

    def get_orders(self, idx):
        return self.orders[idx]

    def get_symbols(self, idx):
        return self.symbols[idx]

    def lvls_list(self, lvls, key):
        res = ["{0},{1},{2}".format(lvl[0] / 256.0, lvl[1][0], lvl[1][1])
               for lvl in sorted(lvls.items(), key=key)]
        if len(res) >= 5:
            return res[:5]
        else:
            return res + list(itertools.repeat("0,0,0", 5 - len(res)))

    def book_dump(self, book, curr_time):
        if (self.dump):
            time = self.time_obj * 1000000000 + curr_time
            ticker = None
            for imnt in self.imnts:
                if imnt in book:
                    ticker = imnt
            print(time, ticker,
                  ','.join(self.lvls_list(book['B'], lambda x: -1 * x[0]) +
                           self.lvls_list(book['S'], lambda x: x[0])),
                  sep=',', file=self.py_out)

    def dump_flush(self):
        if (self.dump):
            self.py_out.flush()

    def parse_header(self, unpacker):
        version = unpacker.unpack()
        symbology = unpacker.unpack()
        return version, symbology

    def update_tme(self, message):
        self.time_obj = message[1]

    def update_add(self, message):
        bookid = message[5]
        orderid = message[6]
        px = message[7]
        qty = message[8]
        side = "B" if message[9] else "S"

        ords = self.orders[bookid]
        levels = self.book[bookid]
        lvls = levels[side]

        ords[orderid] = {
            'price': px,
            'qty': qty,
            'side': side
        }

        if (px not in lvls):
            lvls[px] = [0, 0]

        lvl = lvls[px]
        lvl[0] += qty
        lvl[1] += 1

        self.book_dump(levels, message[1])

    def update_ins(self, message):
        bookid = message[5]
        px = message[8]
        qty = message[9]
        side = "B" if message[9] else "S"

        levels = self.book[bookid]
        lvls = self.levels[side]

        if (px not in lvls):
            lvls[px] = [0, 0]

        lvl = lvls[px]
        lvl[0] += qty
        lvl[1] += 1

        self.book_dump(levels, message[1])

    def update_pos(self, message):
        bookid = message[5]
        orderid = message[6]
        px = message[8]
        qty = message[9]
        side = "B" if message[10] else "S"

        ords = self.orders[bookid]
        levels = self.book[bookid]
        lvls = levels[side]

        ords[orderid] = {
            'price': px,
            'qty': qty,
            'side': side
        }

        if (px not in lvls):
            lvls[px] = [0, 0]

        lvl = lvls[px]
        lvl[0] += qty
        lvl[1] += 1

        self.book_dump(levels, message[1])

    def update_cxl(self, message):
        bookid = message[5]
        orderid = message[6]
        qty = message[7]

        levels = self.book[bookid]
        ords = self.orders[bookid]
        ord = ords[orderid]
        lvls = levels[ord['side']]
        lvl = lvls[ord['price']]
        left = ord['qty'] - qty

        if (left <= 0):
            lvl[0] -= ord['qty']
            lvl[1] -= 1
            del orders[orderid]
        else:
            ord['qty'] = left
            lvl[0] -= qty
        if lvl[0] <= 0:
            del lvls[ord['price']]

        self.book_dump(levels, message[1])

    def update_del(self, message):
        bookid = message[5]
        orderid = message[6]

        levels = self.book[bookid]
        ords = self.orders[bookid]
        ord = ords[orderid]
        px = ord['price']
        side = ord['side']
        lvls = levels[side]
        lvl = lvls[px]

        lvl[0] -= ord['qty']
        lvl[1] -= 1
        del ords[orderid]
        if lvl[0] <= 0:
            del lvls[px]

        self.book_dump(levels, message[1])

    def update_mod(self, message):
        bookid = message[5]
        orderid = message[6]
        new_orderid = message[7]
        new_px = message[8]
        new_qty = message[9]

        levels = self.book[bookid]
        ords = self.orders[bookid]
        ord = ords[orderid]
        px = ord['price']
        side = ord['side']
        lvls = levels[side]
        lvl = lvls[px]

        lvl[0] -= ord['qty']
        lvl[1] -= 1
        del ords[orderid]
        if lvl[0] <= 0:
            del lvls[px]

        ords[new_orderid] = {
            'price': new_px,
            'qty': new_qty,
            'side': side
        }

        if (new_px not in lvls):
            lvls[new_px] = [0, 0]
        lvl = lvls[new_px]
        lvl[0] += new_qty
        lvl[1] += 1

        self.book_dump(levels, message[1])

    def update_exe(self, message):
        bookid = message[5]
        orderid = message[6]

        levels = self.book[bookid]
        ords = self.orders[bookid]
        ord = ords[orderid]
        px = ord['price']
        side = ord['side']
        lvls = levels[side]
        lvl = lvls[px]

        lvl[0] -= ord['qty']
        lvl[1] -= 1
        del ords[orderid]
        if lvl[0] <= 0:
            del lvls[px]

        self.book_dump(levels, message[1])

    def update_epx(self, message):
        bookid = message[5]
        orderid = message[6]

        levels = self.book[bookid]
        ords = self.orders[bookid]
        ord = ords[orderid]
        px = ord['price']
        side = ord['side']
        lvls = levels[side]
        lvl = lvls[px]

        lvl[0] -= ord['qty']
        lvl[1] -= 1
        del ords[orderid]
        if lvl[0] <= 0:
            del lvls[px]

        self.book_dump(levels, message[1])

    def update_fld(self, message):
        bookid = message[5]
        orderid = message[6]
        exec_qty = message[7]

        levels = self.book[bookid]
        ords = self.orders[bookid]
        ord = ords[orderid]
        px = ord['price']
        side = ord['side']
        lvls = levels[side]
        lvl = lvls[px]
        left = ord['qty'] - exec_qty

        if (left <= 0):
            lvl[0] -= ord['qty']
            lvl[1] -= 1
            del ords[orderid]
        else:
            ord['qty'] = left
            lvl[0] -= exec_qty
        if lvl[0] <= 0:
            del lvls[px]

        self.book_dump(levels, message[1])

    def update_fpx(self, message):
        bookid = message[5]
        orderid = message[6]
        exec_qty = message[8]

        levels = self.book[bookid]
        ords = self.orders[bookid]
        ord = ords[orderid]
        px = ord['price']
        side = ord['side']
        lvls = levels[side]
        lvl = lvls[px]
        left = ord['qty'] - exec_qty

        if (left <= 0):
            lvl[0] -= ord['qty']
            lvl[1] -= 1
            del ords[orderid]
        else:
            ord['qty'] = left
            lvl[0] -= exec_qty
        if lvl[0] <= 0:
            del lvls[px]

        self.book_dump(levels, message[1])

    def update(self, message):
        if not isinstance(message, list):
            raise RuntimeError("File corrupted, book entry must be an array")

        type = message[0]

        if type not in range(0, 14) or (type != 0 and message[5] not in self.book):
            return

        callback = self.callbacks[type]
        if (callback):
            callback(message)

    def build_book(self):
        while True:
            try:
                self.update(self.unpacker.unpack())
            except mp.exceptions.OutOfData as e:
                # Finished reading file
                break


def validate_books(extr_book, py_book, py_orders, price_tick, qty_tick):
    for side, levels in extr_book:
        if side == extr.trade_side.BID():
            py_side = "B"
        else:
            py_side = "S"

        py_levels = py_book[py_side]
        assert len(levels) == len(py_levels), f"The number of levels in the extractor book ({len(levels)}) does not match the number of levels in the python book ({len(py_levels)}) for side {py_side}"
        for px, lvl in levels:
            py_px = int(round(px * price_tick))
            assert py_px in py_levels, f"Price {px} not in python book levels"
            py_lvl = py_levels[py_px]
            lvl_accum = 0
            for order in lvl:
                assert order.id in py_orders, f"Unable to find order {order.id} in python orders map"
                py_order = py_orders[order.id]
                assert py_order['price'] == py_px, f"Python order price {float(py_order['price']) / float(price_tick)} for order id {order.id} does not match with extractor order price, {px}"
                assert py_order['side'] == py_side, f"Python order side {py_order['side']} for order id {order.id} does not match with extractor order side, {py_side}"
                assert py_order['qty'] == int(round(order.qty * qty_tick)), f"Python order qty {py_order['qty']} for order id {order.id} does not match with extractor order qty, {order.qty}"
                lvl_accum += int(round(order.qty * qty_tick))
            assert py_lvl[0] == lvl_accum, f"Shares in extractor levels ({lvl_accum}) do not match shares in python levels ({py_lvl[0]}) in level for price {px}"
            assert py_lvl[1] == len(lvl), f"Orders in extractor levels ({len(lvl)}) do not match orders in python levels ({py_lvl[1]}) in level for price {px}"


def book_cross_validation(file, imnts, *args):
    print("cross validating", imnts, args)
    builder = BookBuilder(file, imnts, *args)
    builder.build_book()

    graph = extr.system.comp_graph()
    op = graph.features
    upds = op.book_play_split(file, tuple(imnts))
    extr_books = [extr.Book() for i in upds]
    top_of_books = [op.book_build(upd, 1, extr_book) for upd, extr_book in zip(upds, extr_books)]

    graph.stream_ctx().run()

    for imnt_idx, imnt in enumerate(imnts):
        symbols = builder.get_symbols(imnt)
        idx = symbols["bookid"]
        validate_books(
            extr_books[imnt_idx],
            builder.get_book(idx),
            builder.get_orders(idx),
            symbols["price_tick"],
            symbols["qty_tick"])
