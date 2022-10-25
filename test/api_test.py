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
@package api_test.py
@author Andres Rangel
@date 31 Mar 2020
@brief File contains extractor python sample
"""

import extractor as extr
import pytz
import os

src_dir = os.path.dirname(os.path.realpath(__file__))

if __name__ == "__main__":

    graph = extr.system.comp_graph()
    op = graph.features

    data_in = graph.features.csv_play(os.path.join(src_dir, "data/csv_play_file.csv"),
                                      (("timestamp", extr.Time64, ""),
                                       ("val1", extr.Int32, ""),
                                          ("val2", extr.Uint16, ""),
                                          ("unicode", extr.Array(extr.Char, 30), "")), name="csv_play_0")

    fields = data_in._fields
    # {'timestamp': <class 'extractor.Time64'>,
    #  'unicode': <extractor.Array object at 0x7f5cf835acb0>,
    #  'val1': <class 'extractor.Int32'>,
    #  'val2': <class 'extractor.Uint16'>}

    assert len(fields) == 4
    assert isinstance(fields, dict)

    # Equal validation
    assert fields["timestamp"] == extr.Time64
    assert fields["unicode"] == extr.Array(extr.Char, 30)
    assert fields["val1"] == extr.Int32
    assert fields["val2"] == extr.Uint16

    # Non equal validation
    assert fields["timestamp"] != extr.Int64
    assert fields["timestamp"] != extr.Array(extr.Char, 33)
    assert fields["unicode"] != extr.Array(extr.Char, 22)
    assert fields["unicode"] != extr.Array(extr.Int64, 30)
    assert fields["unicode"] != extr.Array(extr.Array(extr.Int64, 30), 30)

    shape = data_in._shape
    # (1,)

    assert len(shape) == 1
    assert shape[0] == 1

    frame_descr = [(fieldname, fieldtype) for fieldname, fieldtype in data_in]

    assert len(frame_descr) == 4
    for descr in frame_descr:
        assert len(descr) == 2

    assert frame_descr[0][0] == "timestamp"
    assert frame_descr[1][0] == "unicode"
    assert frame_descr[2][0] == "val1"
    assert frame_descr[3][0] == "val2"

    assert frame_descr[0][1] == extr.Time64
    assert frame_descr[1][1] == extr.Array(extr.Char, 30)
    assert frame_descr[2][1] == extr.Int32
    assert frame_descr[3][1] == extr.Uint16
