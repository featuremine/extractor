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
import unittest

class NumericalTests(unittest.TestCase):
    def test_decimal128(self):
        from_int = extr.Decimal128(0)
        self.assertTrue(isinstance(from_int, extr.Decimal128))
        self.assertTrue(int(from_int) == 0)
        self.assertTrue(not from_int.is_signed())
        self.assertTrue(from_int.is_zero())
        self.assertTrue(not math.isnan(from_int))
        self.assertTrue(not math.isinf(from_int))
        self.assertTrue(math.isfinite(from_int))

        from_string = extr.Decimal128("442.21")
        self.assertTrue(isinstance(from_string, extr.Decimal128))
        self.assertTrue(float(from_string) == 442.21)
        self.assertTrue(not from_string.is_signed())
        self.assertTrue(not from_string.is_zero())
        self.assertTrue(not math.isnan(from_string))
        self.assertTrue(not math.isinf(from_string))
        self.assertTrue(math.isfinite(from_string))

        from_double = extr.Decimal128(-5.442)
        self.assertTrue(isinstance(from_double, extr.Decimal128))
        self.assertTrue(float(from_double) == -5.442)
        self.assertTrue(from_double.is_signed())
        self.assertTrue(not from_double.is_zero())
        self.assertTrue(not math.isnan(from_double))
        self.assertTrue(not math.isinf(from_double))
        self.assertTrue(math.isfinite(from_double))

        from_decimal = extr.Decimal128(from_double)
        self.assertTrue(isinstance(from_decimal, extr.Decimal128))
        self.assertTrue(float(from_decimal) == -5.442)
        self.assertTrue(from_decimal.is_signed())
        self.assertTrue(not from_decimal.is_zero())
        self.assertTrue(not math.isnan(from_decimal))
        self.assertTrue(not math.isinf(from_decimal))
        self.assertTrue(math.isfinite(from_decimal))

        inf = extr.Decimal128(math.inf)
        self.assertTrue(isinstance(inf, extr.Decimal128))
        self.assertTrue(float(inf) == math.inf)
        self.assertTrue(not inf.is_signed())
        self.assertTrue(not inf.is_zero())
        self.assertTrue(not math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertTrue(not math.isfinite(inf))

        inf = extr.Decimal128(-math.inf)
        self.assertTrue(isinstance(inf, extr.Decimal128))
        self.assertTrue(float(inf) == -math.inf)
        self.assertTrue(inf.is_signed())
        self.assertTrue(not inf.is_zero())
        self.assertTrue(not math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertTrue(not math.isfinite(inf))
        self.assertTrue(float(inf) == -math.inf)

        nan = extr.Decimal128(math.nan)
        self.assertTrue(isinstance(nan, extr.Decimal128))
        self.assertTrue(not nan.is_signed())
        self.assertTrue(not nan.is_zero())
        self.assertTrue(math.isnan(nan))
        self.assertTrue(not math.isinf(nan))
        self.assertTrue(not math.isfinite(nan))

        inf = extr.Decimal128("inf")
        self.assertTrue(isinstance(inf, extr.Decimal128))
        self.assertTrue(float(inf) == math.inf)
        self.assertTrue(not math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertTrue(not math.isfinite(inf))
        self.assertTrue(not inf.is_signed())
        self.assertTrue(not inf.is_zero())

        inf = extr.Decimal128("-inf")
        self.assertTrue(isinstance(inf, extr.Decimal128))
        self.assertTrue(float(inf) == -math.inf)
        self.assertTrue(not math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertTrue(not math.isfinite(inf))
        self.assertTrue(inf.is_signed())
        self.assertTrue(not inf.is_zero())

        nan = extr.Decimal128("nan")
        self.assertTrue(isinstance(nan, extr.Decimal128))
        self.assertTrue(math.isnan(nan))
        self.assertTrue(not math.isinf(nan))
        self.assertTrue(not math.isfinite(nan))
        self.assertTrue(not nan.is_signed())
        self.assertTrue(not nan.is_zero())

        self.assertTrue(abs(extr.Decimal128(5)) == extr.Decimal128(5))
        self.assertTrue(abs(extr.Decimal128(-5)) == extr.Decimal128(5))
        self.assertTrue(abs(extr.Decimal128(math.inf)) == extr.Decimal128(math.inf))
        self.assertTrue(abs(extr.Decimal128(-math.inf)) == extr.Decimal128(math.inf))

        self.assertTrue(extr.Decimal128(5) + extr.Decimal128(5) == extr.Decimal128(10))
        self.assertTrue(extr.Decimal128(5) - extr.Decimal128(10) == extr.Decimal128(-5))
        self.assertTrue(extr.Decimal128(5) * extr.Decimal128(2) == extr.Decimal128(10))
        self.assertTrue(extr.Decimal128(10) / extr.Decimal128(2) == extr.Decimal128(5))

        v1 = extr.Decimal128(5)
        v1 += extr.Decimal128(5)
        self.assertTrue(v1 == extr.Decimal128(10))
        v1 = extr.Decimal128(5)
        v1 -= extr.Decimal128(10)
        self.assertTrue(v1 == extr.Decimal128(-5))
        v1 = extr.Decimal128(5)
        v1 *= extr.Decimal128(5)
        self.assertTrue(v1 == extr.Decimal128(25))
        v1 = extr.Decimal128(10)
        v1 /= extr.Decimal128(2)
        self.assertTrue(v1 == extr.Decimal128(5))

        self.assertTrue(max(extr.Decimal128(10), extr.Decimal128(2), extr.Decimal128(5)) == extr.Decimal128(10))
        self.assertTrue(min(extr.Decimal128(10), extr.Decimal128(2), extr.Decimal128(5)) == extr.Decimal128(2))

        v1 = extr.Decimal128.from_float(2.35)
        self.assertTrue(float(v1) == 2.35)

    def test_rprice(self):
        from_int = extr.Rprice(0)
        self.assertTrue(isinstance(from_int, extr.Rprice))
        self.assertTrue(int(from_int) == 0)
        self.assertTrue(not from_int.is_signed())
        self.assertTrue(from_int.is_zero())
        self.assertTrue(not math.isnan(from_int))
        self.assertTrue(not math.isinf(from_int))
        self.assertTrue(math.isfinite(from_int))

        from_double = extr.Rprice(-5.442)
        self.assertTrue(isinstance(from_double, extr.Rprice))
        self.assertTrue(float(from_double) == -5.442)
        self.assertTrue(from_double.is_signed())
        self.assertTrue(not from_double.is_zero())
        self.assertTrue(not math.isnan(from_double))
        self.assertTrue(not math.isinf(from_double))
        self.assertTrue(math.isfinite(from_double))

        from_decimal = extr.Rprice(from_double)
        self.assertTrue(isinstance(from_decimal, extr.Rprice))
        self.assertTrue(float(from_decimal) == -5.442)
        self.assertTrue(from_decimal.is_signed())
        self.assertTrue(not from_decimal.is_zero())
        self.assertTrue(not math.isnan(from_decimal))
        self.assertTrue(not math.isinf(from_decimal))
        self.assertTrue(math.isfinite(from_decimal))

        inf = extr.Rprice(math.inf)
        self.assertTrue(not inf.is_signed())
        self.assertTrue(not inf.is_zero())
        self.assertTrue(not math.isnan(inf))
        self.assertTrue(not math.isinf(inf))
        self.assertTrue(math.isfinite(inf))
        self.assertTrue(float(inf) == 9223372036.854776)

        inf = extr.Rprice(-math.inf)
        self.assertTrue(inf.is_signed())
        self.assertTrue(not inf.is_zero())
        self.assertTrue(not math.isnan(inf))
        self.assertTrue(not math.isinf(inf))
        self.assertTrue(math.isfinite(inf))
        self.assertTrue(float(inf) == -9223372036.854776)

        nan = extr.Rprice(math.nan)
        self.assertTrue(not nan.is_signed())
        self.assertTrue(nan.is_zero())
        self.assertTrue(not math.isnan(nan))
        self.assertTrue(not math.isnan(nan))
        self.assertTrue(math.isfinite(nan))
        self.assertTrue(float(nan) == 0.0)

        self.assertTrue(abs(extr.Rprice(5)) == extr.Rprice(5))
        self.assertTrue(abs(extr.Rprice(-5)) == extr.Rprice(5))

        self.assertTrue(extr.Rprice(5) + extr.Rprice(5) == extr.Rprice(10))
        self.assertTrue(extr.Rprice(5) - extr.Rprice(10) == extr.Rprice(-5))
        self.assertTrue(extr.Rprice(5) * extr.Rprice(2) == extr.Rprice(10))
        self.assertTrue(extr.Rprice(10) / extr.Rprice(2) == extr.Rprice(5))

        v1 = extr.Rprice(5)
        v1 += extr.Rprice(5)
        self.assertTrue(v1 == extr.Rprice(10))
        v1 = extr.Rprice(5)
        v1 -= extr.Rprice(10)
        self.assertTrue(v1 == extr.Rprice(-5))
        v1 = extr.Rprice(5)
        v1 *= extr.Rprice(5)
        self.assertTrue(v1 == extr.Rprice(25))
        v1 = extr.Rprice(10)
        v1 /= extr.Rprice(2)
        self.assertTrue(v1 == extr.Rprice(5))

        self.assertTrue(max(extr.Rprice(10), extr.Rprice(2), extr.Rprice(5)) == extr.Rprice(10))
        self.assertTrue(min(extr.Rprice(10), extr.Rprice(2), extr.Rprice(5)) == extr.Rprice(2))

        v1 = extr.Rprice.from_float(2.35)
        self.assertTrue(float(v1) == 2.35)

    def test_rational64(self):
        from_int = extr.Rational64(0)
        self.assertTrue(isinstance(from_int, extr.Rational64))
        self.assertTrue(int(from_int) == 0)
        self.assertTrue(not from_int.is_signed())
        self.assertTrue(from_int.is_zero())
        self.assertTrue(not math.isnan(from_int))
        self.assertTrue(not math.isinf(from_int))
        self.assertTrue(math.isfinite(from_int))

        from_double = extr.Rational64(-5.442)
        self.assertTrue(isinstance(from_double, extr.Rational64))
        self.assertAlmostEqual(float(from_double), -5.442)
        self.assertTrue(from_double.is_signed())
        self.assertTrue(not from_double.is_zero())
        self.assertTrue(not math.isnan(from_double))
        self.assertTrue(not math.isinf(from_double))
        self.assertTrue(math.isfinite(from_double))

        from_decimal = extr.Rational64(from_double)
        self.assertTrue(isinstance(from_decimal, extr.Rational64))
        self.assertAlmostEqual(float(from_decimal), -5.442)
        self.assertTrue(from_decimal.is_signed())
        self.assertTrue(not from_decimal.is_zero())
        self.assertTrue(not math.isnan(from_decimal))
        self.assertTrue(not math.isinf(from_decimal))
        self.assertTrue(math.isfinite(from_decimal))

        inf = extr.Rational64(math.inf)
        self.assertTrue(isinstance(inf, extr.Rational64))
        self.assertTrue(float(inf) == math.inf)
        self.assertTrue(not inf.is_signed())
        self.assertTrue(not inf.is_zero())
        self.assertTrue(not math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertTrue(not math.isfinite(inf))

        inf = extr.Rational64(-math.inf)
        self.assertTrue(isinstance(inf, extr.Rational64))
        self.assertTrue(float(inf) == -math.inf)
        self.assertTrue(inf.is_signed())
        self.assertTrue(not inf.is_zero())
        self.assertTrue(not math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertTrue(not math.isfinite(inf))
        self.assertTrue(float(inf) == -math.inf)

        nan = extr.Rational64(math.nan)
        self.assertTrue(isinstance(nan, extr.Rational64))
        self.assertTrue(not nan.is_signed())
        self.assertTrue(not nan.is_zero())
        self.assertTrue(math.isnan(nan))
        self.assertTrue(not math.isinf(nan))
        self.assertTrue(not math.isfinite(nan))

        self.assertTrue(abs(extr.Rational64(5)) == extr.Rational64(5))
        self.assertTrue(abs(extr.Rational64(-5)) == extr.Rational64(5))
        self.assertTrue(abs(extr.Rational64(math.inf)) == extr.Rational64(math.inf))
        self.assertTrue(abs(extr.Rational64(-math.inf)) == extr.Rational64(math.inf))

        self.assertTrue(extr.Rational64(5) + extr.Rational64(5) == extr.Rational64(10))
        self.assertTrue(extr.Rational64(5) - extr.Rational64(10) == extr.Rational64(-5))
        self.assertTrue(extr.Rational64(5) * extr.Rational64(2) == extr.Rational64(10))
        self.assertTrue(extr.Rational64(10) / extr.Rational64(2) == extr.Rational64(5))

        v1 = extr.Rational64(5)
        v1 += extr.Rational64(5)
        self.assertTrue(v1 == extr.Rational64(10))
        v1 = extr.Rational64(5)
        v1 -= extr.Rational64(10)
        self.assertTrue(v1 == extr.Rational64(-5))
        v1 = extr.Rational64(5)
        v1 *= extr.Rational64(5)
        self.assertTrue(v1 == extr.Rational64(25))
        v1 = extr.Rational64(10)
        v1 /= extr.Rational64(2)
        self.assertTrue(v1 == extr.Rational64(5))

        self.assertTrue(max(extr.Rational64(10), extr.Rational64(2), extr.Rational64(5)) == extr.Rational64(10))
        self.assertTrue(min(extr.Rational64(10), extr.Rational64(2), extr.Rational64(5)) == extr.Rational64(2))

        v1 = extr.Rational64.from_float(2.35)
        self.assertAlmostEqual(float(v1), 2.35)

if __name__ == "__main__":
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(NumericalTests)
    assert unittest.TextTestRunner().run(suite).wasSuccessful(), 'Test runner failed'
