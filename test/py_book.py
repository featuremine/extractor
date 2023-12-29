"""
        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

        """

"""
@package py_book.py
@author Andres Rangel
@date 2 Mar 2020
@brief File contains python book test
"""

import extractor as extr
import os

src_dir = os.path.dirname(os.path.realpath(__file__))


def create_and_delete():
    b = extr.Book()


def access_violations(b):
    try:
        b[extr.trade_side.ASK()][99]
    except IndexError as e:
        pass
    else:
        assert False

    try:
        b[extr.trade_side.ASK()][368.66]
    except IndexError as e:
        pass
    else:
        assert False

    try:
        b[extr.trade_side.ASK()][0][99]
    except IndexError as e:
        pass
    else:
        assert False


def iterate_and_validate(book, levellimit, levelcount, ordcount, ordtotal):
    assert len(book) == 2
    i = 0
    local_levelcount = 0
    local_ordercount = 0
    local_totalcount = 0
    j = 0
    for side, levels in book:
        i += 1
        local_levelcount += len(levels)
        k = 0
        for price, level in levels:
            if k < levellimit:
                local_ordercount += len(level)
            k += 1
            local_totalcount += len(level)
            for order in level:
                j += 1
                order.priority
                order.id
                order.qty
                order.received
                order.ven
                order.seqnum
    assert i == 2
    assert j == local_totalcount
    assert local_totalcount == ordtotal
    assert local_ordercount == ordcount
    assert local_levelcount == levelcount


if __name__ == "__main__":
    create_and_delete()
    graph = extr.system.comp_graph()
    op = graph.features
    upds = op.book_play_split(os.path.join(src_dir, "data/book.base.ore"), ("2_YEAR", "3_YEAR", "10_YEAR"))
    books = [extr.Book() for upd in upds]
    py_levels = [op.book_build(upd, 5, b) for b, upd in zip(books, upds)]
    py_joint_stream = op.join(*py_levels, "ticker", extr.Array(extr.Char, 16),
                              ("2_YEAR", "3_YEAR", "10_YEAR"))
    py_trigger = op.trigger(py_joint_stream)
    py_out_stream = op.combine(py_trigger, tuple(), py_joint_stream, tuple())
    descr = ("time", "ticker",
             "bid_prx_0", "bid_shr_0", "bid_ord_0",
             "bid_prx_1", "bid_shr_1", "bid_ord_1",
             "bid_prx_2", "bid_shr_2", "bid_ord_2",
             "bid_prx_3", "bid_shr_3", "bid_ord_3",
             "bid_prx_4", "bid_shr_4", "bid_ord_4",
             "ask_prx_0", "ask_shr_0", "ask_ord_0",
             "ask_prx_1", "ask_shr_1", "ask_ord_1",
             "ask_prx_2", "ask_shr_2", "ask_ord_2",
             "ask_prx_3", "ask_shr_3", "ask_ord_3",
             "ask_prx_4", "ask_shr_4", "ask_ord_4",
             )
    graph.features.csv_record(py_out_stream, os.path.join(src_dir, "data/py_book_levels.extr.py_book.test.csv"),
                              descr)

    levels_ref = [graph.get_ref(py_level) for py_level in py_levels]

    graph.stream_ctx().run()

    ordaccum = []

    # book against bookbuilding frames validation

    for book, level_ref in zip(books, levels_ref):
        bidside = book[extr.trade_side.BID()]
        askside = book[extr.trade_side.ASK()]

        lvl_ref = level_ref[0]

        assert askside[0].orders == lvl_ref.ask_ord_0
        assert len(askside[0]) == lvl_ref.ask_ord_0
        assert extr.FixedPoint128(str(askside[0].shares)) == lvl_ref.ask_shr_0
        assert extr.FixedPoint128(str(askside[0].px)) == lvl_ref.ask_prx_0

        assert askside[1].orders == lvl_ref.ask_ord_1
        assert len(askside[1]) == lvl_ref.ask_ord_1
        assert extr.FixedPoint128(str(askside[1].shares)) == lvl_ref.ask_shr_1
        assert extr.FixedPoint128(str(askside[1].px)) == lvl_ref.ask_prx_1

        assert askside[2].orders == lvl_ref.ask_ord_2
        assert len(askside[2]) == lvl_ref.ask_ord_2
        assert extr.FixedPoint128(str(askside[2].shares)) == lvl_ref.ask_shr_2
        assert extr.FixedPoint128(str(askside[2].px)) == lvl_ref.ask_prx_2

        assert askside[3].orders == lvl_ref.ask_ord_3
        assert len(askside[3]) == lvl_ref.ask_ord_3
        assert extr.FixedPoint128(str(askside[3].shares)) == lvl_ref.ask_shr_3
        assert extr.FixedPoint128(str(askside[3].px)) == lvl_ref.ask_prx_3

        assert askside[4].orders == lvl_ref.ask_ord_4
        assert len(askside[4]) == lvl_ref.ask_ord_4
        assert extr.FixedPoint128(str(askside[4].shares)) == lvl_ref.ask_shr_4
        assert extr.FixedPoint128(str(askside[4].px)) == lvl_ref.ask_prx_4

        assert bidside[0].orders == lvl_ref.bid_ord_0
        assert len(bidside[0]) == lvl_ref.bid_ord_0
        assert extr.FixedPoint128(str(bidside[0].shares)) == lvl_ref.bid_shr_0
        assert extr.FixedPoint128(str(bidside[0].px)) == lvl_ref.bid_prx_0

        assert bidside[1].orders == lvl_ref.bid_ord_1
        assert len(bidside[1]) == lvl_ref.bid_ord_1
        assert extr.FixedPoint128(str(bidside[1].shares)) == lvl_ref.bid_shr_1
        assert extr.FixedPoint128(str(bidside[1].px)) == lvl_ref.bid_prx_1

        assert bidside[2].orders == lvl_ref.bid_ord_2
        assert len(bidside[2]) == lvl_ref.bid_ord_2
        assert extr.FixedPoint128(str(bidside[2].shares)) == lvl_ref.bid_shr_2
        assert extr.FixedPoint128(str(bidside[2].px)) == lvl_ref.bid_prx_2

        assert bidside[3].orders == lvl_ref.bid_ord_3
        assert len(bidside[3]) == lvl_ref.bid_ord_3
        assert extr.FixedPoint128(str(bidside[3].shares)) == lvl_ref.bid_shr_3
        assert extr.FixedPoint128(str(bidside[3].px)) == lvl_ref.bid_prx_3

        assert bidside[4].orders == lvl_ref.bid_ord_4
        assert len(bidside[4]) == lvl_ref.bid_ord_4
        assert extr.FixedPoint128(str(bidside[4].shares)) == lvl_ref.bid_shr_4
        assert extr.FixedPoint128(str(bidside[4].px)) == lvl_ref.bid_prx_4

        ordaccum.append(
            lvl_ref.ask_ord_0 +
            lvl_ref.ask_ord_1 +
            lvl_ref.ask_ord_2 +
            lvl_ref.ask_ord_3 +
            lvl_ref.ask_ord_4 +
            lvl_ref.bid_ord_0 +
            lvl_ref.bid_ord_1 +
            lvl_ref.bid_ord_2 +
            lvl_ref.bid_ord_3 +
            lvl_ref.bid_ord_4)

    # indices validation

    lvl_by_price = books[-1][extr.trade_side.ASK()][98.546875]
    lvl_by_idx = books[-1][extr.trade_side.ASK()][0]

    assert lvl_by_price.px == lvl_by_idx.px

    ord_by_price = lvl_by_price[0]
    ord_by_idx = lvl_by_idx[0]

    assert ord_by_price.id == ord_by_idx.id
    assert ord_by_price.priority == ord_by_idx.priority
    assert ord_by_price.qty == ord_by_idx.qty
    assert ord_by_price.received == ord_by_idx.received
    assert ord_by_price.ven == ord_by_idx.ven
    assert ord_by_price.seqnum == ord_by_idx.seqnum

    # negative indices validation

    lvl_by_price = books[-1][extr.trade_side.ASK()][99.0625]
    lvl_by_idx = books[-1][extr.trade_side.ASK()][-1]
    lvl_by_positive_idx = books[-1][extr.trade_side.ASK()][33]
    assert lvl_by_price.px == lvl_by_idx.px
    assert lvl_by_price.px == lvl_by_positive_idx.px

    ord_by_price = lvl_by_price[-1]
    ord_by_idx = lvl_by_idx[0]

    assert ord_by_price.id == ord_by_idx.id
    assert ord_by_price.priority == ord_by_idx.priority
    assert ord_by_price.qty == ord_by_idx.qty
    assert ord_by_price.received == ord_by_idx.received
    assert ord_by_price.ven == ord_by_idx.ven
    assert ord_by_price.seqnum == ord_by_idx.seqnum

    # iteration validation

    ordtotals = [390, 328, 410]
    levels = [54, 55, 72]

    for book, level, ordcount, ordtotal in zip(books, levels, ordaccum, ordtotals):
        iterate_and_validate(book, 5, level, ordcount, ordtotal)
        access_violations(book)

    # output file validation

    extr.flush()
    extr.assert_numdiff(os.path.join(src_dir, "data/py_book_levels.extr.py_book.test.csv"),
                        os.path.join(src_dir, "data/book_levels.extr.base.csv"))
