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
@package callbacks.py
@author Andres Rangel
@date 10 May 2019
@brief File contains extractor python sample
"""

import extractor as extr
import math

if __name__ == "__main__":

    from_int = extr.Decimal128(5)
    assert isinstance(from_int, extr.Decimal128)
    assert from_int == extr.Decimal128(5)
    assert int(from_int) == 5

    from_string = extr.Decimal128("5")
    assert isinstance(from_string, extr.Decimal128)
    assert from_string == extr.Decimal128(5)
    assert int(from_string) == 5

    from_double = extr.Decimal128(5.442)
    assert isinstance(from_double, extr.Decimal128)
    assert from_double == extr.Decimal128(5.442)
    assert float(from_double) == 5.442
    
    assert abs(from_int) == from_int
    assert not math.isnan(from_int)
    assert not math.isinf(from_int)
    assert math.isfinite(from_int)

    inf = extr.Decimal128("inf")
    assert not math.isnan(inf)
    assert math.isinf(inf)
    assert not math.isfinite(inf)

    inf = extr.Decimal128("-inf")
    assert not math.isnan(inf)
    assert math.isinf(inf)
    assert not math.isfinite(inf)

    nan = extr.Decimal128("nan")
    assert math.isnan(nan)
    assert not math.isinf(nan)
    assert not math.isfinite(nan)

    assert extr.Decimal128(5) + extr.Decimal128(5) == extr.Decimal128(10)
    assert extr.Decimal128(5) - extr.Decimal128(10) == extr.Decimal128(-5)
    assert extr.Decimal128(5) * extr.Decimal128(2) == extr.Decimal128(10)
    assert extr.Decimal128(10) / extr.Decimal128(2) == extr.Decimal128(5)

    v1 = extr.Decimal128(5)
    v1 += extr.Decimal128(5)
    assert v1 == extr.Decimal128(10)
    v1 = extr.Decimal128(5)
    v1 -= extr.Decimal128(10)
    assert v1 == extr.Decimal128(-5)
    v1 = extr.Decimal128(5)
    v1 *= extr.Decimal128(5)
    assert v1 == extr.Decimal128(25)
    v1 = extr.Decimal128(10)
    v1 *= extr.Decimal128(2)
    assert v1 == extr.Decimal128(5)
