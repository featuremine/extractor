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

def decimal128():
    from_int = extr.Decimal128(0)
    assert isinstance(from_int, extr.Decimal128)
    assert int(from_int) == 0
    assert not from_int.is_signed()
    assert from_int.is_zero()

    from_string = extr.Decimal128("442.21")
    assert isinstance(from_string, extr.Decimal128)
    assert float(from_string) == 442.21
    assert not from_string.is_signed()
    assert not from_string.is_zero()

    from_double = extr.Decimal128(-5.442)
    assert isinstance(from_double, extr.Decimal128)
    assert float(from_double) == -5.442
    assert from_double.is_signed()
    assert not from_double.is_zero()
    
    assert abs(from_int) == from_int
    assert not math.isnan(from_int)
    assert not math.isinf(from_int)
    assert math.isfinite(from_int)

    inf = extr.Decimal128("inf")
    assert not math.isnan(inf)
    assert math.isinf(inf)
    assert not math.isfinite(inf)
    assert not inf.is_signed()
    assert not inf.is_zero()

    inf = extr.Decimal128("-inf")
    assert not math.isnan(inf)
    assert math.isinf(inf)
    assert not math.isfinite(inf)
    assert inf.is_signed()
    assert not inf.is_zero()

    nan = extr.Decimal128("nan")
    assert math.isnan(nan)
    assert not math.isinf(nan)
    assert not math.isfinite(nan)
    assert not nan.is_signed()
    assert not nan.is_zero()

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
    v1 /= extr.Decimal128(2)
    assert v1 == extr.Decimal128(5)

    assert max(extr.Decimal128(10), extr.Decimal128(2), extr.Decimal128(5)) == extr.Decimal128(10)
    assert min(extr.Decimal128(10), extr.Decimal128(2), extr.Decimal128(5)) == extr.Decimal128(2)

    v1 = extr.Decimal128.from_float(2.35)
    assert float(v1) == 2.35, float(v1)

if __name__ == "__main__":

    decimal128()
