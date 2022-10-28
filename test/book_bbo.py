import os
import argparse
import shutil
import extractor
from yamal import ytp
from datetime import timedelta


src_dir = os.path.dirname(os.path.realpath(__file__))

# copy the ytp file, because it is overwritten
ytp_file_cpy = ""

trades_base_file = os.path.join(src_dir, 'data', 'book_bbo_trades.base.txt')
trades_file = os.path.join(src_dir, 'data', 'book_bbo_trades.test.txt')
bbos_base_file = os.path.join(src_dir, 'data', 'book_bbo_bbos.base.txt')
bbos_file = os.path.join(src_dir, 'data', 'book_bbo_bbos.test.txt')

trades_cnt = 500
bbos_cnt = 1000


def print_trades(x):
    global trades_file, trades_base_file, trades_cnt, ytp_file_cpy
    try:
        f = open(trades_file, "a")
        print(x, file=f)
        f.close()
    except FileNotFoundError as e:
        print("File not found")
    except RuntimeError as e:
        print("RuntimeError:", str(e))

    trades_cnt -= 1
    if trades_cnt < 1:
        extractor.assert_base(trades_base_file, trades_file)
        if os.path.exists(trades_file):
            os.remove(trades_file)
        if os.path.exists(ytp_file_cpy):
            os.remove(ytp_file_cpy)
        exit()


def print_bbos(x):
    global bbos_file, bbos_base_file, bbos_cnt, ytp_file_cpy
    try:
        f = open(bbos_file, "a")
        print(x, file=f)
        f.close()
    except FileNotFoundError as e:
        print("File not found")
    except RuntimeError as e:
        print("RuntimeError:", str(e))

    bbos_cnt -= 1
    if bbos_cnt < 1:
        extractor.assert_base(bbos_base_file, bbos_file)
        if os.path.exists(bbos_file):
            os.remove(bbos_file)
        if os.path.exists(ytp_file_cpy):
            os.remove(ytp_file_cpy)
        exit()


def setup_prod_sip(universe, symbology, graph, ytpfile):
    markets = [
        "NYSE",
        "NASDAQOMX",
        "BATS",
        "DirectEdgeX",
        "DirectEdgeA",
        "NYSEArca",
        "NASDAQOMXBX",
        "BATSY"
    ]
    op = graph.features
    seq = ytp.sequence(ytpfile)
    peer = seq.peer("peer_reader")
    op.ytp_sequence(seq, timedelta(milliseconds=1))
    for imnt in universe.get("all"):
        ticker = symbology.info(imnt)["ticker"]
        bbos_book = []
        trades = []
        cum_trades = []
        statuses = []
        for mkt in markets:
            ch = peer.channel(1000, "channels/imnts/{0}/{1}".format(mkt, ticker))  # E.G. "channels/imnts/NYSE/APLE"
            bookupd = op.ore_ytp_decode(ch)
            decoded = op.decode_data(bookupd)
            bbo_book = op.book_build(decoded, 1)  # level = 1

            book_receive = op.decode_receive(bookupd)
            bbo_receive = op.asof(book_receive, bbo_book)

            bidqty_int32 = op.convert(op.round(bbo_book.bid_shr_0), extractor.Int32)
            askqty_int32 = op.convert(op.round(bbo_book.ask_shr_0), extractor.Int32)
            bbo_book_combined = op.combine(
                bbo_book, (
                    ("bid_prx_0", "bidprice"),
                    ("ask_prx_0", "askprice"),
                    ("bid_shr_0", "bidqty"),
                    ("ask_shr_0", "askqty")
                ),
                bbo_receive, (("time", "receive"),)
            )
            graph.callback(bbo_book_combined, print_bbos)

            trade = op.book_trades(decoded)  # change
            trade_receive = op.asof(book_receive, trade)

            bid, ask, unk = op.split(trade.decoration, 'decoration', ('b', 'a', 'u'))
            bid_val = op.constant(bid, ('side', extractor.Int32, 0))
            ask_val = op.constant(ask, ('side', extractor.Int32, 1))
            unk_val = op.constant(unk, ('side', extractor.Int32, 2))
            side = op.join(
                bid_val, ask_val, unk_val, 'decoration', extractor.Array(
                    extractor.Char, 1), ('b', 'a', 'u')).side

            trade_combined = op.combine(
                trade, (("trade_price", "price"),),
                trade.qty, (("qty", "qty"),),
                trade_receive, (("time", "receive"),),
                side, (("side", "side"),)
            )
            graph.callback(trade_combined, print_trades)

            cum_trade = op.cum_trade(trade_combined,
                                     name="cum_trade/{0}/{1}".format(mkt, imnt))
            bbos_book.append(bbo_book_combined)
            trades.append(trade)
            cum_trades.append(cum_trade)
        op.bbo_aggr(*bbos_book, name="nbbo/{0}".format(imnt))
        op.cum_trade_total(*cum_trades,
                           name="cum_trade_total/{0}".format(imnt))


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
    args = parser.parse_args()

    graph = extractor.system.comp_graph()
    op = graph.features

    symb_table = {
        1: {"ticker": "APLE"},
        2: {"ticker": "APAM"},
        3: {"ticker": "BMY"},
        4: {"ticker": "BAC"},
    }
    symbology = Symbology(symb_table)
    universe = Universe(symb_table.keys())

    if os.path.exists(trades_file):
        os.remove(trades_file)
    if os.path.exists(bbos_file):
        os.remove(bbos_file)

    ytp_file_cpy = args.ytp + '.cpy'
    shutil.copy(args.ytp, ytp_file_cpy)
    setup_prod_sip(universe, symbology, graph, ytp_file_cpy)

    graph.stream_ctx().run_live()
