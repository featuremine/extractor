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
@package timeout.py
@author Federico Ravchina
@date 22 Nov 2021
@brief File contains extractor python sample
"""

from run_live import run_live_test
from datetime import timedelta
from yamal import ytp
import os
import extractor as extr
from time import time, sleep


def time_ms():
    return int(time() * 1000)


src_dir = os.path.dirname(os.path.realpath(__file__))


def timeout_test():
    extr.set_license(os.path.join(src_dir, "test.lic"))
    graph = extr.system.comp_graph()
    op = graph.features
    try:
        os.remove('ytp_sequence.ytp')
    except BaseException:
        pass
    seq = ytp.sequence('ytp_sequence.ytp')
    peer = seq.peer('peer1')
    data_channel = peer.channel(0, 'channel1')
    now = time_ms()
    try:
        data_decoded = op.frame_ytp_decode(data_channel, timedelta(seconds=1))
    except RuntimeError as e:
        assert e.args[0] == 'header ytp message not found'
    assert time_ms() - now >= 1000

    os.remove('ytp_sequence.ytp')


if __name__ == "__main__":
    def subject_data(op, input):
        try:
            os.remove('ytp_sequence.ytp')
        except BaseException:
            pass

        seq = ytp.sequence('ytp_sequence.ytp')
        peer = seq.peer('peer1')
        data_channel = peer.channel(0, 'channel1')
        data_stream = peer.stream(data_channel)

        op.frame_ytp_encode(input, data_stream)
        data_decoded = op.frame_ytp_decode(data_channel)

        op.ytp_sequence(seq, timedelta(milliseconds=1))

        return op.decode_data(data_decoded)

    def subject_receive(op, input):
        try:
            os.remove('ytp_sequence.ytp')
        except BaseException:
            pass

        seq = ytp.sequence('ytp_sequence.ytp')
        peer = seq.peer('peer1')
        data_channel = peer.channel(0, 'channel1')
        data_stream = peer.stream(data_channel)

        op.frame_ytp_encode(input, data_stream)
        data_decoded = op.frame_ytp_decode(data_channel)

        op.ytp_sequence(seq, timedelta(milliseconds=1))

        return op.decode_receive(data_decoded)

    run_live_test(
        in_data_array=[
            {"Timestamp": 0, "val1": 1, "val2": 2},
            {"Timestamp": 390, "val1": 2, "val2": 2},
            {"Timestamp": 430, "val1": 3, "val2": 2},
            {"Timestamp": 450, "val1": 4, "val2": 2},
            {"Timestamp": 550, "val1": 5, "val2": 2},
            {"Timestamp": 980, "val1": 6, "val2": 2},
        ],
        expected_output=[
            {"Timestamp": 0.000, "val1": 1, "val2": 2},
            {"Timestamp": 0.390, "val1": 2, "val2": 2},
            {"Timestamp": 0.430, "val1": 3, "val2": 2},
            {"Timestamp": 0.450, "val1": 4, "val2": 2},
            {"Timestamp": 0.550, "val1": 5, "val2": 2},
            {"Timestamp": 0.980, "val1": 6, "val2": 2},
        ],
        total_time=1500,
        subject=subject_data,
    )
    timeout_test()
