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

if __name__ == "__main__":
    def subject(op, input):
        return op.delayed(input, timedelta(milliseconds=200))
    run_live_test(
        in_data_array=[
            {"Timestamp": 0, "time": 0, "val2": 1},
            {"Timestamp": 590, "time": 100, "val2": 1},
            {"Timestamp": 700, "time": 150, "val2": 1},
            {"Timestamp": 800, "time": 700, "val2": 1},
            {"Timestamp": 850, "time": 700, "val2": 1},
            {"Timestamp": 950, "time": 920, "val2": 1},
        ],
        expected_output=[
            {"Timestamp": 0.000, "delayed": False},
            {"Timestamp": 0.590, "delayed": True},
            {"Timestamp": 0.700, "delayed": True},
            {"Timestamp": 0.800, "delayed": False},
            {"Timestamp": 0.850, "delayed": False},
            {"Timestamp": 0.950, "delayed": False},
        ],
        total_time=1500,
        subject=subject,
    )
