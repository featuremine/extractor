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
@package error_reporting.py
@date 10 Apr 2023
@brief File contains error reporting test
"""

import os
import extractor
import msgpack as mp
from yamal import ytp


if __name__ == "__main__":
    graph = extractor.system.comp_graph()
    op = graph.features

    if os.path.exists("test.ytp.0001"):
        os.remove("test.ytp.0001")
    seq = ytp.sequence("test.ytp.0001")
    peer = seq.peer("testpeer")
    ch = peer.channel(1000, 'channels/imnts/NASDAQOMX/AAPL')
    stream = peer.stream(ch)

    stream.write(0, mp.packb([15, 1680185041040000000, 0, 0, 0, 0, "channels/imnts/NASDAQOMX/AAPL", 10000, 1]))
    stream.write(0, mp.packb([15, 1680185041040000000, 0, 0, 0, 0, "channels/imnts/NASDAQOMX/AAPL", 10000, 1]))
    stream.write(0, mp.packb([11, 1680185045000000000, 0, 1, 0, 0, 1480400, 100, "u"]))

    data = op.seq_ore_live_split("test.ytp", "channels/seconds", (('channels/imnts/NASDAQOMX/AAPL',)))
    graph.callback(data[1], lambda x: x.as_pandas())
    graph.callback(op.count(data[1]), lambda x: x.as_pandas())

    try:
        graph.stream_ctx().run_live()
    except TypeError as e:
        assert str(e) == 'Invalid data type in frame', e
    except Exception as e:
        raise e

    if os.path.exists("test.ytp.0001"):
        os.remove("test.ytp.0001")
