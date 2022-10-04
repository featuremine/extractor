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
 * @file rprice.hpp
 * @author Maxim Trokhimtchouk
 * @date 14 Oct 2017
 * @brief File contains C++ definition of a rational price
 */

#pragma once

#include "decimal64.hpp"
#include "fmc++/convert.hpp"
#include "side.hpp"

#include <iomanip>
#include <iostream>

namespace fm {
using namespace std;

using rprice = fm_decimal64_t;

template <> struct sided_initializer<rprice> {
  static constexpr bool is_specialized = true;
  static constexpr rprice min() noexcept { return FM_DECIMAL64_MIN; }
  static constexpr rprice max() noexcept { return FM_DECIMAL64_MAX; }
};
} // namespace fm

namespace fmc {
template <> struct conversion<fm::rprice, double> {
  double operator()(fm::rprice x) { return fm_decimal64_to_double(x); }
};

template <> struct conversion<double, fm::rprice> {
  fm::rprice operator()(double x) { return fm_decimal64_from_double(x); }
};
} // namespace fmc

namespace std {
inline ostream &operator<<(ostream &s, const fm::rprice &x) {
  using namespace std;
  return s << setprecision(15) << fmc::to<double>(x);
}

inline istream &operator>>(istream &s, fm::rprice &x) {
  using namespace std;
  double d;
  s >> d;
  x = fmc::to<fm::rprice>(d);
  return s;
}

inline string to_string(fm::rprice &x) { return to_string(fmc::to<double>(x)); }

} // namespace std
