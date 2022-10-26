#!/usr/bin/python3
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
@package is_nan.py
@author Andres Rangel
@date 24 Oct 2018
@brief File contains extractor python sample
"""
import extractor as extr
from extractor import result_as_pandas
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    numerical_data = graph.features.constant(("A", extr.Float64, 35.2), ("B", extr.Float64, 3.14))
    inf_data = graph.features.constant(("A", extr.Float64, float('NaN')), ("B", extr.Float64, float('NaN')))

    is_nan = graph.features.is_nan(inf_data)
    is_not_nan = graph.features.is_nan(numerical_data)

    ctx = graph.stream_ctx()
    ctx.run()

    as_pd = result_as_pandas(is_nan)
    print(as_pd)
    if not as_pd.all(axis=None).any():
        print("Not all entries are true")
        exit(1)
    as_pd = result_as_pandas(is_not_nan)
    print(as_pd)
    if as_pd.any(axis=None).any():
        print("Not all entries are false")
        exit(1)
