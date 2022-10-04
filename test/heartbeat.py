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
@package heartbeat.py
@author Federico Ravchina
@date 22 Nov 2021
@brief File contains extractor python sample
"""

from run_live import run_live_test
from datetime import timedelta

if __name__ == "__main__":
    def subject(op, input):
        return op.heartbeat(input, timedelta(milliseconds=200))
    run_live_test(
        in_data_array=[
            {"Timestamp": 0, "val1": 1, "val2": 1},
            {"Timestamp": 390, "val1": 2, "val2": 1},
            {"Timestamp": 430, "val1": 3, "val2": 1},
            {"Timestamp": 450, "val1": 4, "val2": 1},
            {"Timestamp": 550, "val1": 5, "val2": 1},
            {"Timestamp": 980, "val1": 6, "val2": 1},
        ],
        expected_output=[
            {"Timestamp": 0.200},
            {"Timestamp": 0.750},
            {"Timestamp": 0.950},
            {"Timestamp": 1.180},
            {"Timestamp": 1.380},
        ],
        total_time=1500,
        subject=subject,
    )
