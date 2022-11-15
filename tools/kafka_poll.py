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
@package kafka_play.py
@author Maxim Trokhimtchouk
@date 27 Mar 2019
@brief File contains extractor python sample
"""

from kafka import KafkaConsumer
from datetime import datetime, timedelta
from collections import namedtuple
import pandas as pd
import time
import json
import sys
import argparse
import extractor as extr
import pytz

Quote = namedtuple('Quote', ["receive", "ticker", "market", "bidprice", "askprice", "bidqty", "askqty"])
Trade = namedtuple('Trade', ["receive", "ticker", "market", "price", "qty", "side"])


def prep_quotes(df):
    df['askprice'] = df['askprice'] / 10000000.0
    df['bidprice'] = df['bidprice'] / 10000000.0
    df['receive'] = pd.to_datetime(df['receive'], unit='ns')
    return Quote(**df)


def prep_trades(df):
    df['price'] = df['price'] / 10000000.0
    df['receive'] = pd.to_datetime(df['receive'], unit='ns')
    return Trade(**df)


def consume(topic, servers=None, prepare=None):
    if servers:
        consumer = KafkaConsumer(topic, auto_offset_reset='earliest',
                                 bootstrap_servers=args.bootstrap_servers,
                                 value_deserializer=lambda m: json.loads(m.decode('utf-8')))
    else:
        consumer = KafkaConsumer(topic, auto_offset_reset='earliest',
                                 value_deserializer=lambda m: json.loads(m.decode('utf-8')))

    while True:
        data = consumer.poll()
        if data:
            assert (len(data) == 1)
            rows = next(iter(data.values()))
            yield [prepare(row.value) for row in rows]
        else:
            yield None


def poller(topics, servers):
    consumers = [(consume(key, servers, prepare=value)) for key, value in topics.items()]
    while True:
        done = False
        for it in consumers:
            data = next(it)
            if data:
                yield data
                done = True
        if not done:
            yield None


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--bootstrap_servers",
        nargs='*',
        help="'host[:port]' string (or list of 'host[:port]' strings) that the producer should contact to bootstrap initial cluster metadata. This does not have to be the full node list. It just needs to have at least one broker that will respond to a Metadata API Request. Default port is 9092. If no servers are specified, will default to localhost:9092.")
    args = parser.parse_args()

    poller = poller({"quotes": prep_quotes, "trades": prep_trades}, servers=args.bootstrap_servers)

    markets = ["NYSEMKT", "NASDAQOMX", "NYSEArca"]
    tickers = ["A", "AA", "BA"]

    graph = extr.system.comp_graph()
    op = graph.features

    data_in = op.live_poll(poller, timedelta(milliseconds=1))

    bbos_in = op.tuple_msg(data_in, "Quote",
                           (("receive", extr.Time64),
                            ("ticker", extr.Array(extr.Char, 16)),
                               ("market", extr.Array(extr.Char, 32)),
                               ("bidprice", extr.Rprice),
                               ("askprice", extr.Rprice),
                               ("bidqty", extr.Int32),
                               ("askqty", extr.Int32)))

    trades_in = op.tuple_msg(data_in, "Trade",
                             (("receive", extr.Time64),
                              ("ticker", extr.Array(extr.Char, 16)),
                                 ("market", extr.Array(extr.Char, 32)),
                                 ("price", extr.Rprice),
                                 ("qty", extr.Int32),
                                 ("side", extr.Int32)))

    op.csv_record(bbos_in, '/dev/stdout')
    op.csv_record(trades_in, '/dev/stdout')

    graph.stream_ctx().run_live()
