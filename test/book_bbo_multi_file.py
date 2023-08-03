import os
import argparse
import shutil
import extractor
from datetime import timedelta


src_dir = os.path.dirname(os.path.realpath(__file__))

def test_printer(cnt, testname):
    bbo_trades_base_file = os.path.join(src_dir, 'data', f'{testname}.base.txt')
    bbo_trades_test_file = os.path.join(src_dir, 'data', f'{testname}.test.txt')

    if os.path.exists(bbo_trades_test_file):
        os.remove(bbo_trades_test_file)

    def fn(x):
        nonlocal cnt
        f = open(bbo_trades_test_file, "a")
        print(x, file=f)
        f.close()

        cnt -= 1
        if cnt < 1:
            extractor.assert_base(bbo_trades_base_file, bbo_trades_test_file)
            if os.path.exists(bbo_trades_test_file):
                os.remove(bbo_trades_test_file)
            exit()

    return fn


def stdout_printer():
    return lambda x: print(x)


def setup_prod_sip(universe, symbology, markets, lvl, time_ch, graph, ytpfile, printer):
    period = timedelta(weeks=52*50)
    op = graph.features
    imnts_chs = tuple()
    mkt_imnts = []
    for imnt in universe.get("all"):
        ticker = symbology.info(imnt)["ticker"]
        for mkt in markets:
            imnts_chs += ("channels/imnts/{0}/{1}".format(mkt, ticker),)  # E.G. "channels/imnts/NYSE/APLE"
            mkt_imnts += [(mkt, ticker)]

    time_upd, *upds = op.seq_ore_live_split(ytpfile, time_ch, imnts_chs)
    gothru = op.filter_if(op.logical_not(op.delayed(op.book_msg(time_upd, "time"), period)))
    headers = [op.book_header(upd) for upd in upds]

    levels = [op.book_build(upd, lvl) for upd in upds]
    lvlhdrs = [op.asof(hdr, lvl) for hdr, lvl in zip(headers, levels)]
    bbos = [op.combine(
        level, (
            ("bid_prx_0", "bidprice"),
            ("ask_prx_0", "askprice"),
            ("bid_shr_0", "bidqty"),
            ("ask_shr_0", "askqty")
        ),
        hdr, (("receive", "receive"),)) for level, hdr in zip(levels, lvlhdrs)]
    filtered_bbos = {(mkt_imnt[0], mkt_imnt[1]):
                     op.filter_if(gothru, bbo, name=f'bbo/{mkt_imnt[0]}/{mkt_imnt[1]}')
                     for bbo, mkt_imnt in zip(bbos, mkt_imnts)}

    filtered_trades = {}
    book_trades = [op.book_trades(upd) for upd in upds]
    for trade, mkt_imnt in zip(book_trades, mkt_imnts):
        const_b = op.constant(('decoration', extractor.Array(extractor.Char, 4), 'b'))
        const_a = op.constant(('decoration', extractor.Array(extractor.Char, 4), 'a'))
        const_u = op.constant(('decoration', extractor.Array(extractor.Char, 4), 'u'))
        decoration_equals_a = trade.decoration == const_a  # op.equal(trade.decoration, const_a)
        decoration_equals_b = trade.decoration == const_b  # op.equal(trade.decoration, const_a)
        const_n_value_b = op.constant(('side', extractor.Int32, 0))
        const_n_value_a = op.constant(('side', extractor.Int32, 1))
        const_n_value_u = op.constant(('side', extractor.Int32, 2))
        eq_b = op.cond(decoration_equals_b, const_n_value_b, const_n_value_u)
        side = op.cond(decoration_equals_a, const_n_value_a, eq_b)

        trade = op.combine(
            trade, (
                ("trade_price", "price"),
                ("receive", "receive"),
                ("qty", "qty")
            ),
            side, (("side", "side"),))

        name = f'trade/{mkt_imnt[0]}/{mkt_imnt[1]}'
        filtered_trades[(mkt_imnt[0], mkt_imnt[1])] = op.skip_unless(gothru, trade, name=name)

    for imnt in universe.get("all"):
        ticker = symbology.info(imnt)["ticker"]
        bbos_book = []
        trades_combined = []
        cum_trades = []
        statuses = []
        for mkt in markets:
            bbo_book_combined = filtered_bbos[(mkt, ticker)]
            trade_combined = filtered_trades[(mkt, ticker)]
            graph.callback(bbo_book_combined, printer)
            graph.callback(trade_combined, printer)
            bbos_book.append(bbo_book_combined)
            trades_combined.append(trade_combined)
            cum_trade = op.cumulative(op.combine(trade_combined.qty, (("qty", "shares"),), op.convert(trade_combined.qty, extractor.Float64) * op.convert(trade_combined.price, extractor.Float64), (("qty", "notional",),), name="cum_trade/{0}/{1}".format(mkt, imnt)))
            cum_trades.append(cum_trade)
            status_frame = op.constant(("receive", extractor.Time64, timedelta(seconds=0)),
                                       ("short_sale_indicator", extractor.Int32, 0))
            statuses.append(status_frame)
        op.bbo_aggr(*bbos_book, name="nbbo/{0}".format(imnt))
        #op.last_trade(*trades_combined, name="last_trade/{0}".format(imnt))
        op.sum(*cum_trades, name="cum_trade_total/{0}".format(imnt))
        #op.status_aggr(*statuses, name="status_aggr/{0}".format(imnt))


class Universe:
    def __init__(self, universe_keys):
        self.universe_keys = universe_keys

    def get(self, name):
        if name != "all":
            raise LookupError("unknown universe name")
        return self.universe_keys


class Symbology:
    def __init__(self, symb_table):
        self.imnts = symb_table

    def info(self, id):
        return self.imnts[id]


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--ytp", help="YTP file", required=True)
    parser.add_argument("--markets", help="Comma separated markets list", required=False)
    parser.add_argument("--imnts", help="Comma separated instrument list", required=False)
    parser.add_argument("--levels", help="Number of levels to display", type=int, required=False, default=1)
    parser.add_argument("--time", help="Time channel name", required=False, default="channels/seconds")
    parser.add_argument("--testcount", help="number of lines expected in the test output", required=False)
    parser.add_argument("--testname", help="test name", required=False)
    args = parser.parse_args()

    graph = extractor.system.comp_graph()
    op = graph.features

    # E.G.
    # symb_table = {
    #     1: {"ticker": "APLE"},
    #     2: {"ticker": "APAM"},
    #     3: {"ticker": "BMY"},
    #     4: {"ticker": "BAC"},
    # }
    symb_table = {}
    i = 1
    for imnt in args.imnts.split(','):
        symb_table[i] = {"ticker": imnt}
        i += 1
    symbology = Symbology(symb_table)
    universe = Universe(symb_table.keys())

    # E.G.
    # markets = [
    #     "NYSE",
    #     "NASDAQOMX",
    #     "BATS",
    #     "DirectEdgeX",
    #     "DirectEdgeA",
    #     "NYSEArca",
    #     "NASDAQOMXBX",
    #     "BATSY"
    # ]
    markets = tuple(args.markets.split(','))

    if args.testname is not None:
        printer = test_printer(int(args.testcount), args.testname)
    else:
        printer = stdout_printer()

    setup_prod_sip(universe, symbology, markets, args.levels, args.time, graph, args.ytp, printer)

    graph.stream_ctx().run_live()
