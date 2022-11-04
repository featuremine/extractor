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
    assert not math.isnan(from_int)
    assert not math.isinf(from_int)
    assert math.isfinite(from_int)

    from_string = extr.Decimal128("442.21")
    assert isinstance(from_string, extr.Decimal128)
    assert float(from_string) == 442.21
    assert not from_string.is_signed()
    assert not from_string.is_zero()
    assert not math.isnan(from_string)
    assert not math.isinf(from_string)
    assert math.isfinite(from_string)

    from_double = extr.Decimal128(-5.442)
    assert isinstance(from_double, extr.Decimal128)
    assert float(from_double) == -5.442
    assert from_double.is_signed()
    assert not from_double.is_zero()
    assert not math.isnan(from_double)
    assert not math.isinf(from_double)
    assert math.isfinite(from_double)

    from_decimal = extr.Decimal128(from_double)
    assert isinstance(from_decimal, extr.Decimal128)
    assert float(from_decimal) == -5.442
    assert from_decimal.is_signed()
    assert not from_decimal.is_zero()
    assert not math.isnan(from_decimal)
    assert not math.isinf(from_decimal)
    assert math.isfinite(from_decimal)

    inf = extr.Decimal128(math.inf)
    assert isinstance(inf, extr.Decimal128)
    assert float(inf) == math.inf
    assert not inf.is_signed()
    assert not inf.is_zero()
    assert not math.isnan(inf)
    assert math.isinf(inf)
    assert not math.isfinite(inf)

    inf = extr.Decimal128(-math.inf)
    assert isinstance(inf, extr.Decimal128)
    assert float(inf) == -math.inf
    assert inf.is_signed()
    assert not inf.is_zero()
    assert not math.isnan(inf)
    assert math.isinf(inf)
    assert not math.isfinite(inf)
    assert float(inf) == -math.inf

    nan = extr.Decimal128(math.nan)
    assert isinstance(nan, extr.Decimal128)
    # assert float(nan) == math.nan FIIIIX THIIIS
    assert not nan.is_signed()
    assert not nan.is_zero()
    assert math.isnan(nan)
    assert not math.isinf(nan)
    assert not math.isfinite(nan)

    inf = extr.Decimal128("inf")
    assert isinstance(inf, extr.Decimal128)
    assert float(inf) == math.inf
    assert not math.isnan(inf)
    assert math.isinf(inf)
    assert not math.isfinite(inf)
    assert not inf.is_signed()
    assert not inf.is_zero()

    inf = extr.Decimal128("-inf")
    assert isinstance(inf, extr.Decimal128)
    assert float(inf) == -math.inf
    assert not math.isnan(inf)
    assert math.isinf(inf)
    assert not math.isfinite(inf)
    assert inf.is_signed()
    assert not inf.is_zero()

    nan = extr.Decimal128("nan")
    assert isinstance(nan, extr.Decimal128)
    # assert float(nan) == math.nan FIIIIX THIIIS
    assert math.isnan(nan)
    assert not math.isinf(nan)
    assert not math.isfinite(nan)
    assert not nan.is_signed()
    assert not nan.is_zero()

    assert abs(extr.Decimal128(5)) == extr.Decimal128(5)
    assert abs(extr.Decimal128(-5)) == extr.Decimal128(5)
    assert abs(extr.Decimal128(math.inf)) == extr.Decimal128(math.inf)
    assert abs(extr.Decimal128(-math.inf)) == extr.Decimal128(math.inf)

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
    assert float(v1) == 2.35

def rprice():
    from_int = extr.Rprice(0)
    assert isinstance(from_int, extr.Rprice)
    assert int(from_int) == 0
    assert not from_int.is_signed()
    assert from_int.is_zero()
    assert not math.isnan(from_int)
    assert not math.isinf(from_int)
    assert math.isfinite(from_int)

    from_double = extr.Rprice(-5.442)
    assert isinstance(from_double, extr.Rprice)
    assert float(from_double) == -5.442
    assert from_double.is_signed()
    assert not from_double.is_zero()
    assert not math.isnan(from_double)
    assert not math.isinf(from_double)
    assert math.isfinite(from_double)

    from_decimal = extr.Rprice(from_double)
    assert isinstance(from_decimal, extr.Rprice)
    assert float(from_decimal) == -5.442
    assert from_decimal.is_signed()
    assert not from_decimal.is_zero()
    assert not math.isnan(from_decimal)
    assert not math.isinf(from_decimal)
    assert math.isfinite(from_decimal)

    inf = extr.Rprice(math.inf)
    assert not inf.is_signed()
    assert not inf.is_zero()
    assert not math.isnan(inf)
    assert not math.isinf(inf)
    assert math.isfinite(inf)
    assert float(inf) == 9223372036.854776

    inf = extr.Rprice(-math.inf)
    assert inf.is_signed()
    assert not inf.is_zero()
    assert not math.isnan(inf)
    assert not math.isinf(inf)
    assert math.isfinite(inf)
    assert float(inf) == -9223372036.854776

    nan = extr.Rprice(math.nan)
    assert not nan.is_signed()
    assert nan.is_zero()
    assert not math.isnan(nan)
    assert not math.isnan(nan)
    assert math.isfinite(nan)
    assert float(nan) == 0.0

    assert abs(extr.Rprice(5)) == extr.Rprice(5)
    assert abs(extr.Rprice(-5)) == extr.Rprice(5)

    assert extr.Rprice(5) + extr.Rprice(5) == extr.Rprice(10)
    assert extr.Rprice(5) - extr.Rprice(10) == extr.Rprice(-5)
    assert extr.Rprice(5) * extr.Rprice(2) == extr.Rprice(10)
    assert extr.Rprice(10) / extr.Rprice(2) == extr.Rprice(5)

    v1 = extr.Rprice(5)
    v1 += extr.Rprice(5)
    assert v1 == extr.Rprice(10)
    v1 = extr.Rprice(5)
    v1 -= extr.Rprice(10)
    assert v1 == extr.Rprice(-5)
    v1 = extr.Rprice(5)
    v1 *= extr.Rprice(5)
    assert v1 == extr.Rprice(25)
    v1 = extr.Rprice(10)
    v1 /= extr.Rprice(2)
    assert v1 == extr.Rprice(5)

    assert max(extr.Rprice(10), extr.Rprice(2), extr.Rprice(5)) == extr.Rprice(10)
    assert min(extr.Rprice(10), extr.Rprice(2), extr.Rprice(5)) == extr.Rprice(2)

    v1 = extr.Rprice.from_float(2.35)
    assert float(v1) == 2.35

def rational64():
    from_int = extr.Rational64(0)
    assert isinstance(from_int, extr.Rational64)
    assert int(from_int) == 0
    assert not from_int.is_signed()
    assert from_int.is_zero()
    assert not math.isnan(from_int)
    assert not math.isinf(from_int)
    assert math.isfinite(from_int)

    from_double = extr.Rational64(-5.442)
    assert isinstance(from_double, extr.Rational64)
    assert float(from_double) == -5.46875
    assert from_double.is_signed()
    assert not from_double.is_zero()
    assert not math.isnan(from_double)
    assert not math.isinf(from_double)
    assert math.isfinite(from_double)

    from_decimal = extr.Rational64(from_double)
    assert isinstance(from_decimal, extr.Rational64)
    assert float(from_decimal) == -5.46875
    assert from_decimal.is_signed()
    assert not from_decimal.is_zero()
    assert not math.isnan(from_decimal)
    assert not math.isinf(from_decimal)
    assert math.isfinite(from_decimal)

    inf = extr.Rational64(math.inf)
    assert isinstance(inf, extr.Rational64)
    # assert float(inf) == math.inf FIX THIS
    # assert not inf.is_signed()
    assert not inf.is_zero()
    assert not math.isnan(inf)
    # assert math.isinf(inf)
    # assert not math.isfinite(inf)

    inf = extr.Rational64(-math.inf)
    assert isinstance(inf, extr.Rational64)
    #assert float(inf) == -math.inf FIX THIS
    #assert inf.is_signed()
    #assert not inf.is_zero()
    assert not math.isnan(inf)
    #assert math.isinf(inf)
    #assert not math.isfinite(inf)
    #assert float(inf) == -math.inf

    nan = extr.Rational64(math.nan)
    assert isinstance(nan, extr.Rational64)
    #assert float(nan) == math.nan
    assert not nan.is_signed()
    assert nan.is_zero()
    assert not math.isnan(nan)
    assert not math.isnan(nan)
    assert math.isfinite(nan)

    assert abs(extr.Rational64(5)) == extr.Rational64(5)
    assert abs(extr.Rational64(-5)) == extr.Rational64(5)
    #assert abs(extr.Rational64(math.inf)) == extr.Rational64(math.inf)
    #assert abs(extr.Rational64(-math.inf)) == extr.Rational64(math.inf)

    assert extr.Rational64(5) + extr.Rational64(5) == extr.Rational64(10)
    assert extr.Rational64(5) - extr.Rational64(10) == extr.Rational64(-5)
    assert extr.Rational64(5) * extr.Rational64(2) == extr.Rational64(10)
    assert extr.Rational64(10) / extr.Rational64(2) == extr.Rational64(5)

    v1 = extr.Rational64(5)
    v1 += extr.Rational64(5)
    assert v1 == extr.Rational64(10)
    v1 = extr.Rational64(5)
    v1 -= extr.Rational64(10)
    assert v1 == extr.Rational64(-5)
    v1 = extr.Rational64(5)
    v1 *= extr.Rational64(5)
    assert v1 == extr.Rational64(25)
    v1 = extr.Rational64(10)
    v1 /= extr.Rational64(2)
    assert v1 == extr.Rational64(5)

    assert max(extr.Rational64(10), extr.Rational64(2), extr.Rational64(5)) == extr.Rational64(10)
    assert min(extr.Rational64(10), extr.Rational64(2), extr.Rational64(5)) == extr.Rational64(2)

    v1 = extr.Rational64.from_float(2.35)
    assert float(v1) == 2.34375

if __name__ == "__main__":

    decimal128()
    rprice()
    rational64()
