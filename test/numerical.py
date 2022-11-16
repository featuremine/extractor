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
@date 8 Nov 2022
@brief File contains tests for numerical types
"""

import extractor as extr
import math
import unittest
from decimal import Decimal

class NumericalTests(unittest.TestCase):
    def test_decimal128(self):
        from_int = extr.Decimal128(0)
        self.assertTrue(isinstance(from_int, extr.Decimal128))
        self.assertEqual(int(from_int), 0)
        self.assertAlmostEqual(float(from_int), 0.0)
        self.assertEqual(str(from_int), "0")
        self.assertFalse(from_int.is_signed())
        self.assertTrue(from_int.is_zero())
        self.assertFalse(math.isnan(from_int))
        self.assertFalse(math.isinf(from_int))
        self.assertTrue(math.isfinite(from_int))

        from_string = extr.Decimal128("442.21")
        self.assertTrue(isinstance(from_string, extr.Decimal128))
        self.assertEqual(int(from_string), 442)
        self.assertAlmostEqual(float(from_string), 442.21)
        self.assertEqual(str(from_string), "442.21")
        self.assertFalse(from_string.is_signed())
        self.assertFalse(from_string.is_zero())
        self.assertFalse(math.isnan(from_string))
        self.assertFalse(math.isinf(from_string))
        self.assertTrue(math.isfinite(from_string))

        from_double = extr.Decimal128(-5.442)
        self.assertTrue(isinstance(from_double, extr.Decimal128))
        self.assertEqual(int(from_double), -5)
        self.assertAlmostEqual(float(from_double), -5.442)
        self.assertEqual(str(from_double), "-5.442000000000000170530256582424045")
        self.assertTrue(from_double.is_signed())
        self.assertFalse(from_double.is_zero())
        self.assertFalse(math.isnan(from_double))
        self.assertFalse(math.isinf(from_double))
        self.assertTrue(math.isfinite(from_double))

        from_decimal = extr.Decimal128(from_double)
        self.assertTrue(isinstance(from_decimal, extr.Decimal128))
        self.assertEqual(int(from_decimal), -5)
        self.assertAlmostEqual(float(from_decimal), -5.442)
        self.assertEqual(str(from_decimal), "-5.442000000000000170530256582424045")
        self.assertTrue(from_decimal.is_signed())
        self.assertFalse(from_decimal.is_zero())
        self.assertFalse(math.isnan(from_decimal))
        self.assertFalse(math.isinf(from_decimal))
        self.assertTrue(math.isfinite(from_decimal))

        inf = extr.Decimal128(math.inf)
        self.assertTrue(isinstance(inf, extr.Decimal128))
        self.assertAlmostEqual(float(inf), math.inf)
        self.assertEqual(str(inf), "inf")
        self.assertFalse(inf.is_signed())
        self.assertFalse(inf.is_zero())
        self.assertFalse(math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertFalse(math.isfinite(inf))

        inf = extr.Decimal128(-math.inf)
        self.assertTrue(isinstance(inf, extr.Decimal128))
        self.assertAlmostEqual(float(inf), -math.inf)
        self.assertEqual(str(inf), "-inf")
        self.assertTrue(inf.is_signed())
        self.assertFalse(inf.is_zero())
        self.assertFalse(math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertFalse(math.isfinite(inf))

        nan = extr.Decimal128(math.nan)
        self.assertTrue(isinstance(nan, extr.Decimal128))
        self.assertEqual(str(nan), "nan")
        self.assertFalse(nan.is_signed())
        self.assertFalse(nan.is_zero())
        self.assertTrue(math.isnan(nan))
        self.assertFalse(math.isinf(nan))
        self.assertFalse(math.isfinite(nan))

        inf = extr.Decimal128("inf")
        self.assertTrue(isinstance(inf, extr.Decimal128))
        self.assertAlmostEqual(float(inf), math.inf)
        self.assertEqual(str(inf), "inf")
        self.assertFalse(inf.is_signed())
        self.assertFalse(inf.is_zero())
        self.assertFalse(math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertFalse(math.isfinite(inf))

        inf = extr.Decimal128("-inf")
        self.assertTrue(isinstance(inf, extr.Decimal128))
        self.assertAlmostEqual(float(inf), -math.inf)
        self.assertEqual(str(inf), "-inf")
        self.assertTrue(inf.is_signed())
        self.assertFalse(inf.is_zero())
        self.assertFalse(math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertFalse(math.isfinite(inf))

        nan = extr.Decimal128("nan")
        self.assertTrue(isinstance(nan, extr.Decimal128))
        self.assertEqual(str(nan), "nan")
        self.assertFalse(nan.is_signed())
        self.assertFalse(nan.is_zero())
        self.assertTrue(math.isnan(nan))
        self.assertFalse(math.isinf(nan))
        self.assertFalse(math.isfinite(nan))

        self.assertEqual(abs(extr.Decimal128(5)), extr.Decimal128(5))
        self.assertEqual(abs(extr.Decimal128(-5)), extr.Decimal128(5))
        self.assertEqual(abs(extr.Decimal128(math.inf)), extr.Decimal128(math.inf))
        self.assertEqual(abs(extr.Decimal128(-math.inf)), extr.Decimal128(math.inf))

        self.assertEqual(extr.Decimal128(5) + extr.Decimal128(5), extr.Decimal128(10))
        self.assertEqual(extr.Decimal128(5) - extr.Decimal128(10), extr.Decimal128(-5))
        self.assertEqual(extr.Decimal128(5) * extr.Decimal128(2), extr.Decimal128(10))
        self.assertEqual(extr.Decimal128(10) / extr.Decimal128(2), extr.Decimal128(5))

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

        self.assertEqual(max(extr.Decimal128(10), extr.Decimal128(2), extr.Decimal128(5)), extr.Decimal128(10))
        self.assertEqual(min(extr.Decimal128(10), extr.Decimal128(2), extr.Decimal128(5)), extr.Decimal128(2))

        v1 = extr.Decimal128.from_float(2.35)
        self.assertAlmostEqual(float(v1), 2.35)
        def convtest(repr):
            v1 = extr.Decimal128(repr)
            v2 = Decimal(repr)
            v3 = extr.Decimal128(v2)
            self.assertEqual(v1, v3)
            self.assertAlmostEqual(float(v1), float(v2))
            d1 = v1.as_decimal()
            self.assertEqual(d1, v2)
            self.assertAlmostEqual(float(d1), float(v1))

        convtest("2")
        convtest("1")
        convtest("0")
        convtest("5005005005005005005") # low digits, one digit per declet
        convtest("35035035035035035035") # low digits, two digit per declet
        convtest("135135135135135135135") # low digits, three digit per declet

    def test_rprice(self):
        from_int = extr.Rprice(0)
        self.assertTrue(isinstance(from_int, extr.Rprice))
        self.assertEqual(int(from_int), 0)
        self.assertAlmostEqual(float(from_int), 0.0)
        self.assertEqual(str(from_int), "0.000000")
        self.assertFalse(from_int.is_signed())
        self.assertTrue(from_int.is_zero())
        self.assertFalse(math.isnan(from_int))
        self.assertFalse(math.isinf(from_int))
        self.assertTrue(math.isfinite(from_int))

        from_double = extr.Rprice(-5.442)
        self.assertTrue(isinstance(from_double, extr.Rprice))
        self.assertEqual(int(from_double), -5)
        self.assertAlmostEqual(float(from_double), -5.442)
        self.assertEqual(str(from_double), "-5.442000")
        self.assertTrue(from_double.is_signed())
        self.assertFalse(from_double.is_zero())
        self.assertFalse(math.isnan(from_double))
        self.assertFalse(math.isinf(from_double))
        self.assertTrue(math.isfinite(from_double))

        from_decimal = extr.Rprice(from_double)
        self.assertTrue(isinstance(from_decimal, extr.Rprice))
        self.assertEqual(int(from_decimal), -5)
        self.assertAlmostEqual(float(from_decimal), -5.442)
        self.assertEqual(str(from_decimal), "-5.442000")
        self.assertTrue(from_decimal.is_signed())
        self.assertFalse(from_decimal.is_zero())
        self.assertFalse(math.isnan(from_decimal))
        self.assertFalse(math.isinf(from_decimal))
        self.assertTrue(math.isfinite(from_decimal))

        self.assertEqual(abs(extr.Rprice(5)), extr.Rprice(5))
        self.assertEqual(abs(extr.Rprice(-5)), extr.Rprice(5))

        self.assertEqual(extr.Rprice(5) + extr.Rprice(5), extr.Rprice(10))
        self.assertEqual(extr.Rprice(5) - extr.Rprice(10), extr.Rprice(-5))
        self.assertEqual(extr.Rprice(5) * extr.Rprice(2), extr.Rprice(10))
        self.assertEqual(extr.Rprice(10) / extr.Rprice(2), extr.Rprice(5))

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

        self.assertEqual(max(extr.Rprice(10), extr.Rprice(2), extr.Rprice(5)), extr.Rprice(10))
        self.assertEqual(min(extr.Rprice(10), extr.Rprice(2), extr.Rprice(5)), extr.Rprice(2))

        v1 = extr.Rprice.from_float(2.35)
        self.assertAlmostEqual(float(v1), 2.35)

    def test_rational64(self):
        from_int = extr.Rational64(0)
        self.assertTrue(isinstance(from_int, extr.Rational64))
        self.assertEqual(int(from_int), 0)
        self.assertAlmostEqual(float(from_int), 0.0)
        self.assertEqual(str(from_int), "0/1")
        self.assertFalse(from_int.is_signed())
        self.assertTrue(from_int.is_zero())
        self.assertFalse(math.isnan(from_int))
        self.assertFalse(math.isinf(from_int))
        self.assertTrue(math.isfinite(from_int))

        from_double = extr.Rational64(-5.442)
        self.assertTrue(isinstance(from_double, extr.Rational64))
        self.assertEqual(int(from_double), -5)
        self.assertAlmostEqual(float(from_double), -5.442)
        self.assertEqual(str(from_double), "-1460825751/268435456")
        self.assertTrue(from_double.is_signed())
        self.assertFalse(from_double.is_zero())
        self.assertFalse(math.isnan(from_double))
        self.assertFalse(math.isinf(from_double))
        self.assertTrue(math.isfinite(from_double))

        from_decimal = extr.Rational64(from_double)
        self.assertTrue(isinstance(from_decimal, extr.Rational64))
        self.assertEqual(int(from_decimal), -5)
        self.assertAlmostEqual(float(from_decimal), -5.442)
        self.assertEqual(str(from_decimal), "-1460825751/268435456")
        self.assertTrue(from_decimal.is_signed())
        self.assertFalse(from_decimal.is_zero())
        self.assertFalse(math.isnan(from_decimal))
        self.assertFalse(math.isinf(from_decimal))
        self.assertTrue(math.isfinite(from_decimal))

        inf = extr.Rational64(math.inf)
        self.assertTrue(isinstance(inf, extr.Rational64))
        self.assertAlmostEqual(float(inf), math.inf)
        self.assertEqual(str(inf), "1/0")
        self.assertFalse(inf.is_signed())
        self.assertFalse(inf.is_zero())
        self.assertFalse(math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertFalse(math.isfinite(inf))

        inf = extr.Rational64(-math.inf)
        self.assertTrue(isinstance(inf, extr.Rational64))
        self.assertAlmostEqual(float(inf), -math.inf)
        self.assertEqual(str(inf), "-1/0")
        self.assertTrue(inf.is_signed())
        self.assertFalse(inf.is_zero())
        self.assertFalse(math.isnan(inf))
        self.assertTrue(math.isinf(inf))
        self.assertFalse(math.isfinite(inf))

        nan = extr.Rational64(math.nan)
        self.assertTrue(isinstance(nan, extr.Rational64))
        self.assertEqual(str(nan), "0/0")
        self.assertFalse(nan.is_signed())
        self.assertFalse(nan.is_zero())
        self.assertTrue(math.isnan(nan))
        self.assertFalse(math.isinf(nan))
        self.assertFalse(math.isfinite(nan))

        outside_of_bounds = extr.Rational64(9999999999.0)
        self.assertTrue(isinstance(outside_of_bounds, extr.Rational64))
        self.assertAlmostEqual(float(outside_of_bounds), math.inf)
        self.assertEqual(str(outside_of_bounds), "1/0")
        self.assertFalse(outside_of_bounds.is_signed())
        self.assertFalse(outside_of_bounds.is_zero())
        self.assertFalse(math.isnan(outside_of_bounds))
        self.assertTrue(math.isinf(outside_of_bounds))
        self.assertFalse(math.isfinite(outside_of_bounds))

        outside_of_bounds = extr.Rational64(-9999999999.0)
        self.assertTrue(isinstance(outside_of_bounds, extr.Rational64))
        self.assertAlmostEqual(float(outside_of_bounds), -math.inf)
        self.assertEqual(str(outside_of_bounds), "-1/0")
        self.assertTrue(outside_of_bounds.is_signed())
        self.assertFalse(outside_of_bounds.is_zero())
        self.assertFalse(math.isnan(outside_of_bounds))
        self.assertTrue(math.isinf(outside_of_bounds))
        self.assertFalse(math.isfinite(outside_of_bounds))

        self.assertEqual(abs(extr.Rational64(5)), extr.Rational64(5))
        self.assertEqual(abs(extr.Rational64(-5)), extr.Rational64(5))
        self.assertEqual(abs(extr.Rational64(math.inf)), extr.Rational64(math.inf))
        self.assertEqual(abs(extr.Rational64(-math.inf)), extr.Rational64(math.inf))

        self.assertEqual(extr.Rational64(5) + extr.Rational64(5), extr.Rational64(10))
        self.assertEqual(extr.Rational64(5) - extr.Rational64(10), extr.Rational64(-5))
        self.assertEqual(extr.Rational64(5) * extr.Rational64(2), extr.Rational64(10))
        self.assertEqual(extr.Rational64(10) / extr.Rational64(2), extr.Rational64(5))

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

        self.assertEqual(max(extr.Rational64(10), extr.Rational64(2), extr.Rational64(5)), extr.Rational64(10))
        self.assertEqual(min(extr.Rational64(10), extr.Rational64(2), extr.Rational64(5)), extr.Rational64(2))

        v1 = extr.Rational64.from_float(2.35)
        self.assertAlmostEqual(float(v1), 2.35)

if __name__ == "__main__":
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(NumericalTests)
    assert unittest.TextTestRunner().run(suite).wasSuccessful(), 'Test runner failed'
