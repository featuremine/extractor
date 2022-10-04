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
@package kafka_pub.py
@author Maxim Trokhimtchouk
@date 22 Mar 2019
@brief This file replays the market data to a Kafka bus
"""

from datetime import datetime, timedelta
import pandas as pd
import numpy as np
from kafka import KafkaProducer
import msgpack
import json
import time
import random
import argparse
import sys


def stringify(x):
    return x.decode("utf-8").rstrip('\0') if isinstance(x, bytes) else x


class MPFileReader(object):
    def __init__(self, file):
        self.unpacker = msgpack.Unpacker(file, raw=False)
        self.header = self.unpacker.unpack()

    def __iter__(self):
        return self

    def __next__(self):
        try:
            item = {key: stringify(self.unpacker.unpack()) for key in self.header}
        except msgpack.exceptions.OutOfData as e:
            raise StopIteration
        return (item["receive"], json.dumps(item).encode('utf-8'))


class KafkaSender(object):
    def __init__(self, producer, topic, iter):
        self.iter = iter
        self.data = None
        self.producer = producer
        self.topic = topic

    def __iter__(self):
        return self

    def __next__(self):
        if self.data is not None:
            producer.send(self.topic, value=self.data[1])
            producer.flush()

        self.data = next(self.iter)
        return self.data[0]


class Sequencer(object):
    def __init__(self, *iters):
        self.iters = iters
        self.values = np.array([next(it) for it in self.iters])

    def __iter__(self):
        return self

    def __next__(self):
        try:
            i = np.nanargmin(self.values)
        except ValueError as e:
            raise StopIteration
        val = self.values[i]
        self.values[i] = next(self.iters[i])
        return val


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--bootstrap_servers",
        nargs='*',
        help="‘host[:port]’ string (or list of ‘host[:port]’ strings) that the producer should contact to bootstrap initial cluster metadata. This does not have to be the full node list. It just needs to have at least one broker that will respond to a Metadata API Request. Default port is 9092. If no servers are specified, will default to localhost:9092.")
    args = parser.parse_args()

    if args.bootstrap_servers:
        producer = KafkaProducer(bootstrap_servers=args.bootstrap_servers)
    else:
        producer = KafkaProducer()

    with open("../test/data/sip_quotes_20171018.mp", "rb") as file1:
        with open("../test/data/sip_trades_20171018.mp", "rb") as file2:
            sender1 = KafkaSender(producer, "quotes", MPFileReader(file1))
            sender2 = KafkaSender(producer, "trades", MPFileReader(file2))
            for line in Sequencer(sender1, sender2):
                time.sleep(0.01 * np.random.poisson(1.0))
