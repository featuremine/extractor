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
            def _convtest(repr):
                v1 = extr.Decimal128(repr)
                v2 = Decimal(repr)
                v3 = extr.Decimal128(v2)
                d1 = v1.as_decimal()
                if math.isnan(v1):
                    self.assertTrue(math.isnan(v2))
                    self.assertTrue(math.isnan(v3))
                    self.assertTrue(math.isnan(float(v1)))
                    self.assertTrue(math.isnan(float(v2)))
                    self.assertTrue(math.isnan(float(v3)))
                else:
                    self.assertEqual(v1, v3)
                    self.assertEqual(d1, v2)
                    self.assertAlmostEqual(float(v1), float(v2))
                    self.assertAlmostEqual(float(d1), float(v1))
            _convtest(repr)
            _convtest("-" + repr)

        convtest("2")
        convtest("1")
        convtest("0")
        convtest("5005005005005005005") # low digits, one digit per declet
        convtest("35035035035035035035") # low digits, two digit per declet
        convtest("135135135135135135135") # low digits, three digit per declet
        convtest("2.4")
        convtest("1.24")
        convtest("0.543")
        convtest("5005005005005005005.005005005") # low digits, one digit per declet
        convtest("35035035035035035035.035035035") # low digits, two digit per declet
        convtest("135135135135135135135.135135135") # low digits, three digit per declet

        convtest("5005005005005005005.0050050052") # one extra digit left over in last declet
        convtest("5005005005005005005.00500500522") # two extra digits left over in last declet

        convtest("5005005005005005005005005005005005") # 34 digits, 11 declets (33) and msd (1)

        convtest("inf")
        convtest("nan")

        qnan = Decimal("nan")
        eqnan = extr.Decimal128(qnan)
        cqnan = eqnan.as_decimal()
        self.assertTrue(qnan.is_qnan())
        self.assertFalse(qnan.is_snan())
        self.assertTrue(eqnan.is_qnan())
        self.assertFalse(eqnan.is_snan())
        self.assertTrue(cqnan.is_qnan())
        self.assertFalse(cqnan.is_snan())

        snan = Decimal("snan")
        esnan = extr.Decimal128(snan)
        csnan = esnan.as_decimal()
        self.assertTrue(snan.is_snan())
        self.assertFalse(snan.is_qnan())
        self.assertTrue(esnan.is_snan())
        self.assertFalse(esnan.is_qnan())
        self.assertTrue(csnan.is_snan())
        self.assertFalse(csnan.is_qnan())

    def test_decimal128_identity_double_1(self):
        decimals = []
        integers = []
        signs = [1.0, -1.0]
        decim = 0.0000019073486328125
        while decim < 1.0:
            decimals.append(decim)
            decim *= 2.0

        integ = 1.0
        while integ < 10779215329.0:
            integers.append(integ)
            integ *= 47.0

        decimals.append(0.0)
        integers.append(0.0)

        for sign in signs:
            for decimal in decimals:
                for integer in integers:
                    keep_zeros = 12
                    while keep_zeros >= 0:
                        fval = str((decimal + integer) * sign)
                        v1 = extr.Decimal128(fval)
                        v2 = Decimal(fval)
                        v3 = extr.Decimal128(v2)
                        d1 = v1.as_decimal()

                        self.assertEqual(v1, v3)
                        self.assertEqual(d1, v2)
                        self.assertAlmostEqual(float(v1), float(v2))
                        self.assertAlmostEqual(float(d1), float(v1))

                        keep_zeros-=3

    def test_decimal128_identity_double_2(self):
        original = Decimal(1.0)
        converted = Decimal(-1.0)
        def convert(number):
            nonlocal original
            nonlocal converted
            original = number
            dnumber = extr.Decimal128(number)
            converted = dnumber.as_decimal()
        convert(Decimal("232863465.5218646228313446044921875"))
        self.assertEqual(converted, original)
        convert(Decimal("297581596.830785691738128662109375"))
        self.assertEqual(converted, original)
        convert(Decimal("364181915.17104339599609375"))
        self.assertEqual(converted, original)
        convert(Decimal("-445464211.388862311840057373046875"))
        self.assertEqual(converted, original)
        convert(Decimal("505788598.10503494739532470703125"))
        self.assertEqual(converted, original)
        convert(Decimal("538651182.4585511684417724609375"))
        self.assertEqual(converted, original)
        convert(Decimal("762722659.79106426239013671875"))
        self.assertEqual(converted, original)
        convert(Decimal("962612880.8451635837554931640625"))
        self.assertEqual(converted, original)
        convert(Decimal("976451324.891252994537353515625"))
        self.assertEqual(converted, original)
        convert(Decimal("-2277953182515.47265625"))
        self.assertEqual(converted, original)
        convert(Decimal("2696170217385.513671875"))
        self.assertEqual(converted, original)
        convert(Decimal("2926970179449.04052734375"))
        self.assertEqual(converted, original)
        convert(Decimal("3969252460837.57568359375"))
        self.assertEqual(converted, original)
        convert(Decimal("5888875542576.8427734375"))
        self.assertEqual(converted, original)
        convert(Decimal("6676174390470.93359375"))
        self.assertEqual(converted, original)
        convert(Decimal("6872537948155.7353515625"))
        self.assertEqual(converted, original)
        convert(Decimal("-7422573333287.1533203125"))
        self.assertEqual(converted, original)
        convert(Decimal("8030198688345.638671875"))
        self.assertEqual(converted, original)
        convert(Decimal("9901494043192.171875"))
        self.assertEqual(converted, original)
        convert(Decimal("82889918991823536.0"))
        self.assertEqual(converted, original)
        convert(Decimal("124536052943002528.0"))
        self.assertEqual(converted, original)
        convert(Decimal("144156884135999968.0"))
        self.assertEqual(converted, original)
        convert(Decimal("212839053581483936.0"))
        self.assertEqual(converted, original)
        convert(Decimal("218801490682184032.0"))
        self.assertEqual(converted, original)
        convert(Decimal("-536470472374601792.0"))
        self.assertEqual(converted, original)
        convert(Decimal("537654832563489984.0"))
        self.assertEqual(converted, original)
        convert(Decimal("652938785621731200.0"))
        self.assertEqual(converted, original)
        convert(Decimal("-659372859226975744.0"))
        self.assertEqual(converted, original)
        convert(Decimal("793418214483531648.0"))
        self.assertEqual(converted, original)

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

    def test_fixedpoint128(self):
        from_int = extr.FixedPoint128(0)
        self.assertTrue(isinstance(from_int, extr.FixedPoint128))
        self.assertEqual(int(from_int), 0)
        self.assertAlmostEqual(float(from_int), 0.0)
        self.assertEqual(str(from_int), "0")
        self.assertFalse(from_int.is_signed())
        self.assertTrue(from_int.is_zero())
        self.assertFalse(math.isnan(from_int))
        self.assertFalse(math.isinf(from_int))
        self.assertTrue(math.isfinite(from_int))

        from_double = extr.FixedPoint128(-5.442)
        self.assertTrue(isinstance(from_double, extr.FixedPoint128))
        self.assertEqual(int(from_double), -5)
        self.assertAlmostEqual(float(from_double), -5.442)
        self.assertEqual(str(from_double), "-5.442")
        self.assertTrue(from_double.is_signed())
        self.assertFalse(from_double.is_zero())
        self.assertFalse(math.isnan(from_double))
        self.assertFalse(math.isinf(from_double))
        self.assertTrue(math.isfinite(from_double))

        from_decimal = extr.FixedPoint128(from_double)
        self.assertTrue(isinstance(from_decimal, extr.FixedPoint128))
        self.assertEqual(int(from_decimal), -5)
        self.assertAlmostEqual(float(from_decimal), -5.442)
        self.assertEqual(str(from_decimal), "-5.442")
        self.assertTrue(from_decimal.is_signed())
        self.assertFalse(from_decimal.is_zero())
        self.assertFalse(math.isnan(from_decimal))
        self.assertFalse(math.isinf(from_decimal))
        self.assertTrue(math.isfinite(from_decimal))

        self.assertEqual(abs(extr.FixedPoint128(5)), extr.FixedPoint128(5))
        self.assertEqual(abs(extr.FixedPoint128(-5)), extr.FixedPoint128(5))

        self.assertEqual(extr.FixedPoint128(5) + extr.FixedPoint128(5), extr.FixedPoint128(10))
        self.assertEqual(extr.FixedPoint128(5) - extr.FixedPoint128(10), extr.FixedPoint128(-5))
        self.assertEqual(extr.FixedPoint128(5) * extr.FixedPoint128(2), extr.FixedPoint128(10))
        self.assertEqual(extr.FixedPoint128(10) / extr.FixedPoint128(2), extr.FixedPoint128(5))

        v1 = extr.FixedPoint128(5)
        v1 += extr.FixedPoint128(5)
        self.assertTrue(v1 == extr.FixedPoint128(10))
        v1 = extr.FixedPoint128(5)
        v1 -= extr.FixedPoint128(10)
        self.assertTrue(v1 == extr.FixedPoint128(-5))
        v1 = extr.FixedPoint128(5)
        v1 *= extr.FixedPoint128(5)
        self.assertTrue(v1 == extr.FixedPoint128(25))
        v1 = extr.FixedPoint128(10)
        v1 /= extr.FixedPoint128(2)
        self.assertTrue(v1 == extr.FixedPoint128(5))

        self.assertEqual(max(extr.FixedPoint128(10), extr.FixedPoint128(2), extr.FixedPoint128(5)), extr.FixedPoint128(10))
        self.assertEqual(min(extr.FixedPoint128(10), extr.FixedPoint128(2), extr.FixedPoint128(5)), extr.FixedPoint128(2))

        v1 = extr.FixedPoint128.from_float(2.35)
        self.assertAlmostEqual(float(v1), 2.35)

if __name__ == "__main__":
    suite = unittest.defaultTestLoader.loadTestsFromTestCase(NumericalTests)
    assert unittest.TextTestRunner().run(suite).wasSuccessful(), 'Test runner failed'
