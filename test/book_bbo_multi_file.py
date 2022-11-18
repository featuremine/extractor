import os
import argparse
import shutil
import extractor
from datetime import timedelta


src_dir = os.path.dirname(os.path.realpath(__file__))

trades_base_file = os.path.join(src_dir, 'data', 'book_bbo_trades_multi_file.base.txt')
trades_file = os.path.join(src_dir, 'data', 'book_bbo_trades_multi_file.test.txt')
bbos_base_file = os.path.join(src_dir, 'data', 'book_bbo_bbos_multi_file.base.txt')
bbos_file = os.path.join(src_dir, 'data', 'book_bbo_bbos_multi_file.test.txt')

trades_cnt = 500
bbos_cnt = 1000
test = False


def print_trades(x):
    global trades_file, trades_base_file, trades_cnt, test
    try:
        f = open(trades_file, "a")
        print(x, file=f)
        print(x)
        f.close()
    except FileNotFoundError as e:
        print("File not found")
    except RuntimeError as e:
        print("RuntimeError:", str(e))

    trades_cnt -= 1
    if test and trades_cnt < 1:
        extractor.assert_base(trades_base_file, trades_file)
        if os.path.exists(trades_file):
            os.remove(trades_file)
        exit()


def print_bbos(x):
    global bbos_file, bbos_base_file, bbos_cnt, test
    try:
        f = open(bbos_file, "a")
        print(x, file=f)
        print(x)
        f.close()
    except FileNotFoundError as e:
        print("File not found")
    except RuntimeError as e:
        print("RuntimeError:", str(e))

    bbos_cnt -= 1
    if test and bbos_cnt < 1:
        extractor.assert_base(bbos_base_file, bbos_file)
        if os.path.exists(bbos_file):
            os.remove(bbos_file)
        exit()


def setup_prod_sip(universe, symbology, markets, lvl, time_ch, graph, ytpfile):
    op = graph.features
    imnts_chs = tuple()
    mkt_imnts = []
    for imnt in universe.get("all"):
        ticker = symbology.info(imnt)["ticker"]
        for mkt in markets:
            imnts_chs += ("channels/imnts/{0}/{1}".format(mkt, ticker),)  # E.G. "channels/imnts/NYSE/APLE"
            mkt_imnts += [(mkt, ticker)]
    if time_ch:
        time_upds, *upds = op.seq_ore_live_split(ytpfile, imnts_chs, time_ch)
        graph.callback(time_upds, lambda frame: print(frame))
    else:
        upds = op.seq_ore_live_split(ytpfile, imnts_chs)
    headers = [op.book_header(upd) for upd in upds]
    filtered_headers = [op.filter_if(op.is_zero(hdr.batch), hdr) for hdr in headers]
    levels = [op.book_build(upd, lvl) for upd in upds]
    bbos = {(mkt_imnt[0], mkt_imnt[1]): op.combine(
        level, (
            ("bid_prx_0", "bidprice"),
            ("ask_prx_0", "askprice"),
            ("bid_shr_0", "bidqty"),
            ("ask_shr_0", "askqty")
        ),
        header, (("vendor", "receive"),),
        name=f'bbo/{mkt_imnt[0]}/{mkt_imnt[1]}')
        for level, header, mkt_imnt in zip(levels, filtered_headers, mkt_imnts)}

    trades = {}
    book_trades = [op.book_trades(upd) for upd in upds]
    for trade, mkt_imnt in zip(book_trades, mkt_imnts):
        bid, ask, unk = op.split(trade.decoration, 'decoration', ('b', 'a', 'u'))
        bid_val = op.constant(bid, ('side', extractor.Int32, 0))
        ask_val = op.constant(ask, ('side', extractor.Int32, 1))
        unk_val = op.constant(unk, ('side', extractor.Int32, 2))
        side = op.join(
            bid_val, ask_val, unk_val, 'decoration', extractor.Array(
                extractor.Char, 1), ('b', 'a', 'u')).side

        trades[(mkt_imnt[0], mkt_imnt[1])] = op.combine(
            trade, (
                ("trade_price", "price"),
                ("vendor", "receive"),
                ("qty", "qty")
            ),
            side, (("side", "side"),),
            name=f'trade/{mkt_imnt[0]}/{mkt_imnt[1]}')

    for imnt in universe.get("all"):
        ticker = symbology.info(imnt)["ticker"]
        bbos_book = []
        trades_combined = []
        cum_trades = []
        statuses = []
        for mkt in markets:
            bbo_book_combined = bbos[(mkt, ticker)]
            trade_combined = trades[(mkt, ticker)]
            graph.callback(bbo_book_combined, print_bbos)
            graph.callback(trade_combined, print_trades)
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
    parser.add_argument("--time", help="Time channel name", required=False, default="")
    parser.add_argument("--test", help="boolean to assert against base book", required=False, default="false")
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

    if os.path.exists(trades_file):
        os.remove(trades_file)
    if os.path.exists(bbos_file):
        os.remove(bbos_file)

    if args.test.lower() in ('yes', 'true', 't', 'y', '1'):
        test = True
    elif args.test.lower() in ('no', 'false', 'f', 'n', '0'):
        test = False

    setup_prod_sip(universe, symbology, markets, args.levels, args.time, graph, args.ytp)

    graph.stream_ctx().run_live()
