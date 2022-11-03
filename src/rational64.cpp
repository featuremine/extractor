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
 * @file rational.cpp
 * @author Maxim Trokhimtchouk
 * @date 28 Dec 2018
 * @brief File contains C definitions of the rational object
 *
 * This file contains declarations of the rational object
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/rational64.h"
#include "extractor/decimal64.h"
}
#include <numeric>

fm_rational64_t fm_rational64_zero() { return fm_rational64_t{0, 1}; }

fm_rational64_t fm_rational64_new(int32_t num, int32_t den) {
  auto mult = -1 * (den < 0);
  den *= mult;
  num *= mult;
  auto div = std::gcd(num, den);
  return div ? fm_rational64_t{num / div, den / div} : fm_rational64_t{0, 0};
}

fm_rational64_t fm_rational64_new2(int64_t num, int64_t den) {
  auto mult = -1 * (den < 0);
  den *= mult;
  num *= mult;
  auto div = std::gcd(num, den);

  if (!div)
    return fm_rational64_t{0, 0};

  auto num_n = num / div;
  auto den_n = den / div;
  if (num_n > std::numeric_limits<int32_t>::max() ||
      num_n < std::numeric_limits<int32_t>::lowest() ||
      den_n > std::numeric_limits<int32_t>::max()) {
    num_n = 0;
    den_n = 0;
    // TODO: set overflow exception here
  }
  return fm_rational64_t{int32_t(num_n), int32_t(den_n)};
}

fm_rational64_t fm_rational64_from_double(double value, int32_t base) {
  return fm_rational64_t{(int32_t)lround(floor(value * double(base))), base};
}

fm_rational64_t fm_rational64_from_decimal64(fm_decimal64_t t) {
  return fm_rational64_new2(t.value, DECIMAL64_FRACTION);
}

fm_rational64_t fm_rational64_from_int(int value) {
  return fm_rational64_t{value, 1};
}

double fm_rational64_to_double(fm_rational64_t t) {
  return double(t.num) / double(t.den);
}

fm_decimal64_t fm_rational64_to_decimal64(fm_rational64_t t) {
  return fm_decimal64_from_ratio(int64_t(t.num), int64_t(t.den));
}

fm_rational64_t fm_rational64_div(fm_rational64_t a, fm_rational64_t b) {
  auto num = int64_t(a.num) * int64_t(b.den);
  auto den = int64_t(a.den) * int64_t(b.num);
  return fm_rational64_new(num, den);
}

fm_rational64_t fm_rational64_mul(fm_rational64_t a, fm_rational64_t b) {
  auto num = int64_t(a.num) * int64_t(b.num);
  auto den = int64_t(a.den) * int64_t(b.den);
  return fm_rational64_new(num, den);
}

fm_rational64_t fm_rational64_add(fm_rational64_t a, fm_rational64_t b) {
  auto num = int64_t(a.num) * int64_t(b.den) + int64_t(b.num) * int64_t(a.den);
  auto den = int64_t(a.den) * int64_t(b.den);
  return fm_rational64_new(num, den);
}

fm_rational64_t fm_rational64_sub(fm_rational64_t a, fm_rational64_t b) {
  auto num = int64_t(a.num) * int64_t(b.den) - int64_t(b.num) * int64_t(a.den);
  auto den = int64_t(a.den) * int64_t(b.den);
  return fm_rational64_new(num, den);
}

bool fm_rational64_less(fm_rational64_t a, fm_rational64_t b) {
  return int64_t(a.num) * int64_t(b.den) < int64_t(b.num) * int64_t(a.den);
}

bool fm_rational64_greater(fm_rational64_t a, fm_rational64_t b) {
  return int64_t(a.num) * int64_t(b.den) > int64_t(b.num) * int64_t(a.den);
}

bool fm_rational64_equal(fm_rational64_t a, fm_rational64_t b) {
  return int64_t(a.num) * int64_t(b.den) == int64_t(b.num) * int64_t(a.den);
}

bool fm_rational64_notequal(fm_rational64_t a, fm_rational64_t b) {
  return int64_t(a.num) * int64_t(b.den) != int64_t(b.num) * int64_t(a.den);
}

fm_rational64_t fm_rational64_inf() { return fm_rational64_t{1, 0}; }

fm_rational64_t fm_rational64_nan() { return fm_rational64_t{0, 0}; }

bool fm_rational64_isnan(fm_rational64_t a) { return a.num == 0 && a.den == 0; }

bool fm_rational64_isinf(fm_rational64_t a) {
  return (a.num == 1 || a.num == -1) && a.den == 0;
}
