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
 * @file rational64.hpp
 * @author Maxim Trokhimtchouk
 * @date 29 Dec 2018
 * @brief File contains C++ definitions for the rational64 object
 *
 * This file contains C++ declarations of the rational64 object operations
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "extractor/rational64.h"
}

#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>

inline fm_rational64_t operator/(fm_rational64_t a, fm_rational64_t b) {
  return fm_rational64_div(a, b);
}

inline bool operator==(fm_rational64_t a, fm_rational64_t b) {
  return fm_rational64_equal(a, b);
}

inline bool operator!=(fm_rational64_t a, fm_rational64_t b) {
  return fm_rational64_notequal(a, b);
}

inline fm_rational64_t operator+(fm_rational64_t a, fm_rational64_t b) {
  return fm_rational64_add(a, b);
}

inline fm_rational64_t operator-(fm_rational64_t a, fm_rational64_t b) {
  return fm_rational64_sub(a, b);
}

inline bool operator<(fm_rational64_t a, fm_rational64_t b) {
  return fm_rational64_less(a, b);
}

inline bool operator>(fm_rational64_t a, fm_rational64_t b) {
  return fm_rational64_greater(a, b);
}

inline bool operator<=(fm_rational64_t a, fm_rational64_t b) {
  return !fm_rational64_greater(a, b);
}

inline bool operator>=(fm_rational64_t a, fm_rational64_t b) {
  return !fm_rational64_less(a, b);
}

inline fm_rational64_t operator*(fm_rational64_t a, fm_rational64_t b) {
  return fm_rational64_mul(a, b);
}

namespace std {
template <> class numeric_limits<fm_rational64_t> {
  static constexpr bool is_specialized = true;
  static constexpr bool is_signed = true;
  static constexpr bool is_integer = false;
  static constexpr bool is_exact = true;
  static constexpr bool has_infinity = true;
  static constexpr bool has_quiet_NaN = true;
  static constexpr bool has_signaling_NaN = false;
  static constexpr std::float_denorm_style has_denorm = std::denorm_absent;
  static constexpr bool has_denorm_loss = false;
  static constexpr std::float_round_style round_style = std::round_toward_zero;
  static constexpr bool is_iec559 = false;
  static constexpr bool is_bounded = true;
  static constexpr bool is_modulo = false;
  static constexpr int radix = 2;
  static constexpr int min_exponent = 0;
  static constexpr int min_exponent10 = 0;
  static constexpr int max_exponent = 0;
  static constexpr int max_exponent10 = 0;
  static constexpr bool traps = false;
  static constexpr bool tinyness_before = true;
  static constexpr fm_rational64_t min() noexcept {
    return fm_rational64_t{1, numeric_limits<int32_t>::max()};
  }
  static constexpr fm_rational64_t lowest() noexcept {
    return fm_rational64_t{numeric_limits<int32_t>::lowest(), 1};
  }
  static constexpr fm_rational64_t max() noexcept {
    return fm_rational64_t{numeric_limits<int32_t>::max(), 1};
  }
  static constexpr fm_rational64_t epsilon() noexcept {
    return fm_rational64_t{1, numeric_limits<int32_t>::max()};
  }
  static constexpr fm_rational64_t round_error() noexcept {
    return fm_rational64_t{1, 2};
  }
};

inline ostream &operator<<(ostream &s, const fm_rational64_t &x) {
  return s << x.num << "/" << x.den;
}
inline istream &operator>>(istream &s, fm_rational64_t &x) {
  string str;
  s >> str;
  size_t div_pos = str.find("/");
  if (div_pos == string::npos) {
    div_pos = str.find(".");
    auto den = str.substr(div_pos + 1, str.size() - div_pos - 1);
    int32_t buff = 10;
    for (size_t i = 0; i < den.size() - 1; ++i)
      buff *= 10;
    x.num = stoi(str.substr(0, div_pos)) * buff + stoi(den);
    x.den = buff;
  } else {
    x.num = stoi(str.substr(0, div_pos));
    x.den = stoi(str.substr(div_pos + 1, str.size() - div_pos - 1));
  }
  return s;
}

inline bool isnan(fm_rational64_t x) { return fm_rational64_isnan(x); }

inline string to_string(const fm_rational64_t &x) {
  return to_string(x.num) + "/" + to_string(x.den);
}
template <> struct hash<fm_rational64_t> {
  std::size_t operator()(const fm_rational64_t &val) const {
    return fmc_hash_combine(std::hash<int32_t>{}(val.num),
                            std::hash<int32_t>{}(val.den));
  }
};

}; // namespace std
