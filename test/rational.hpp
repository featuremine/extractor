/******************************************************************************

        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

 *****************************************************************************/

/**
 * @file rational.hpp
 * @author Andres Rangel
 * @date 8 Jan 2019
 * @brief File contains tests for rational serialization
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/decimal64.hpp"
#include "extractor/comp_def.hpp"

#include <fmc++/gtestwrap.hpp>
#include <sstream>

TEST(rational, api) {
  auto zero = fm_rational64_zero();
  ASSERT_EQ(zero.num, 0);
  ASSERT_EQ(zero.den, 1);

  auto sample = fm_rational64_new(31, 32);
  ASSERT_EQ(sample.num, 31);
  ASSERT_EQ(sample.den, 32);

  double val = fm_rational64_to_double(sample);
  ASSERT_DOUBLE_EQ(val, 0.96875);

  auto sample_from_double = fm_rational64_from_double(val, 32);
  ASSERT_EQ(sample_from_double.num, 31);
  ASSERT_EQ(sample_from_double.den, 32);

  ASSERT_TRUE(fm_rational64_equal(sample, sample_from_double));

  auto sample_from_int = fm_rational64_from_int(68);
  ASSERT_EQ(sample_from_int.num, 68);
  ASSERT_EQ(sample_from_int.den, 1);

  ASSERT_TRUE(fm_rational64_less(zero, sample));
  ASSERT_TRUE(zero < sample);
  ASSERT_TRUE(zero <= sample);

  ASSERT_TRUE(zero <= zero);
  ASSERT_TRUE(zero >= zero);

  ASSERT_TRUE(fm_rational64_greater(sample_from_int, sample));
  ASSERT_TRUE(sample_from_int > sample);
  ASSERT_TRUE(sample_from_int >= sample);

  ASSERT_TRUE(fm_rational64_notequal(sample_from_int, sample_from_double));
  ASSERT_TRUE(sample_from_int != sample_from_double);

  auto res_div = fm_rational64_div(sample_from_int, sample_from_double);
  ASSERT_EQ(res_div.num, 2176);
  ASSERT_EQ(res_div.den, 31);
  ASSERT_EQ(res_div, sample_from_int / sample_from_double);

  auto inf_div = fm_rational64_div(sample_from_int, zero);
  ASSERT_EQ(inf_div.num, 1);
  ASSERT_EQ(inf_div.den, 0);
  ASSERT_TRUE(inf_div == sample_from_int / zero);

  auto res_add = fm_rational64_add(sample, sample_from_double);
  ASSERT_EQ(res_add.num, 31);
  ASSERT_EQ(res_add.den, 16);
  ASSERT_EQ(res_add, sample + sample_from_double);

  auto res_mul = fm_rational64_mul(sample_from_int, sample_from_double);
  ASSERT_EQ(res_mul.num, 527);
  ASSERT_EQ(res_mul.den, 8);
  ASSERT_EQ(res_mul, sample_from_int * sample_from_double);

  auto res_sub = fm_rational64_sub(res_add, res_mul);
  ASSERT_EQ(res_sub.num, -1023);
  ASSERT_EQ(res_sub.den, 16);
  ASSERT_EQ(res_sub, res_add - res_mul);

  auto nan = fm_rational64_nan();
  ASSERT_EQ(nan.num, 0);
  ASSERT_EQ(nan.den, 0);

  ASSERT_TRUE(fm_rational64_isnan(nan));
  ASSERT_TRUE(std::isnan(nan));
  ASSERT_FALSE(fm_rational64_isnan(res_div));

  auto inf = fm_rational64_inf();
  ASSERT_EQ(inf.num, 1);
  ASSERT_EQ(inf.den, 0);

  ASSERT_TRUE(fm_rational64_isinf(inf));
  ASSERT_TRUE(fm_rational64_isinf(inf_div));
  ASSERT_FALSE(fm_rational64_isinf(res_sub));
}

TEST(rational, decimal_conversions) {
  double val = -9 - (31 / 32);
  auto d = fm_decimal64_from_double(val);
  auto r = fm_rational64_from_double(val, 32);
  ASSERT_EQ(r, fm_rational64_from_decimal64(d));
  ASSERT_EQ(d, fm_rational64_to_decimal64(r));
  ASSERT_DOUBLE_EQ(fm_rational64_to_double(r), val);
  ASSERT_DOUBLE_EQ(fm_decimal64_to_double(d), val);
}

TEST(rational, serialization) {
  auto a = fm_rational64_new(4, 5);
  std::stringstream s1;
  s1 << a;
  ASSERT_EQ(s1.str().compare("4/5"), 0);
  std::stringstream s2;
  s2 << "6/8";
  s2 >> a;
  std::stringstream s3;
  s3 << a;
  ASSERT_EQ(s3.str().compare("6/8"), 0);
  std::stringstream s4;
  s4 << "3.257";
  s4 >> a;
  std::stringstream s5;
  s5 << a;
  std::stringstream s6;
  s6 << fm_rational64_to_double(a);
  ASSERT_EQ(s5.str().compare("3257/1000"), 0);
  ASSERT_EQ(s6.str().compare("3.257"), 0);
}
