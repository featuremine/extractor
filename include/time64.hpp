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
 * @file time64.hpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definitions for the time object
 *
 * This file contains C++ declarations of the time object operations
 * @see http://www.featuremine.com
 */

#pragma once
#include <fmc/platform.h>
#if defined(FMC_SYS_WIN)
#define timegm _mkgmtime
#endif

extern "C" {
#include "time64.h"
}

#include "fmc++/time.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <time.h>

inline int64_t operator/(fm_time64_t a, fm_time64_t b) {
  return fm_time64_div(a, b);
}

inline bool operator==(fm_time64_t a, fm_time64_t b) {
  return fm_time64_equal(a, b);
}

inline bool operator!=(fm_time64_t a, fm_time64_t b) {
  return !fm_time64_equal(a, b);
}

inline fm_time64_t operator+(fm_time64_t a, fm_time64_t b) {
  return fm_time64_add(a, b);
}

inline fm_time64_t &operator+=(fm_time64_t &a, const fm_time64_t &b) {
  fm_time64_inc(&a, b);
  return a;
}

inline fm_time64_t operator-(fm_time64_t a, fm_time64_t b) {
  return fm_time64_sub(a, b);
}

inline bool operator<(fm_time64_t a, fm_time64_t b) {
  return fm_time64_less(a, b);
}

inline bool operator>(fm_time64_t a, fm_time64_t b) {
  return fm_time64_less(b, a);
}

inline bool operator<=(fm_time64_t a, fm_time64_t b) {
  return !fm_time64_less(b, a);
}

inline bool operator>=(fm_time64_t a, fm_time64_t b) {
  return !fm_time64_less(a, b);
}

inline fm_time64_t operator*(fm_time64_t a, int64_t b) {
  return fm_time64_mul(a, b);
}

inline fm_time64_t operator*(int64_t a, fm_time64_t b) {
  return fm_time64_mul(b, a);
}

inline fm_time64_t operator/(fm_time64_t a, int64_t b) {
  return fm_time64_int_div(a, b);
}

namespace std {
inline ostream &operator<<(ostream &s, const fm_time64_t &x) {
  using namespace std;
  using namespace chrono;
  auto nanos = nanoseconds(fm_time64_to_nanos(x));
  auto epoch = time_point<system_clock>(
      duration_cast<time_point<system_clock>::duration>(nanos));
  auto t = system_clock::to_time_t(epoch);
  auto tm = *gmtime(&t);
  return s << put_time(&tm, "%F %T") << '.' << setw(9) << setfill('0')
           << (nanos % seconds(1)).count();
}
inline istream &operator>>(istream &s, fm_time64_t &x) {
  using namespace std;
  using namespace chrono;
  std::tm t = {};
  unsigned nanos;
  s >> get_time(&t, "%Y-%m-%d %H:%M:%S.") >> setw(9) >> nanos;
  auto epoch_sec = system_clock::from_time_t(timegm(&t)).time_since_epoch();
  auto dur = duration_cast<nanoseconds>(epoch_sec) + nanoseconds(nanos);
  x = fm_time64_from_nanos(dur.count());
  return s;
}

/**
 * @brief Smaller than operator overload for fmc time and fm_time64_t
 * objects
 *
 * @param jt Platform time object.
 * @param et fm_time64_t object.
 *
 * @return result of comparison.
 */
inline bool operator<(const fmc::time &jt, const fm_time64_t &et) {
  return jt < std::chrono::nanoseconds(fm_time64_to_nanos(et));
}

/**
 * @brief Smaller than operator overload for fm_time64_t and fmc time
 * objects
 *
 * @param et fm_time64_t object.
 * @param jt Platform time object.
 *
 * @return Result of comparison.
 */
inline bool operator<(const fm_time64_t &et, const fmc::time &jt) {
  return std::chrono::nanoseconds(fm_time64_to_nanos(et)) < jt;
}

/**
 * @brief Greater than operator overload for fmc time and fm_time64_t
 * objects
 *
 * @param jt Platform time object.
 * @param et fm_time64_t object.
 *
 * @return Result of comparison.
 */
inline bool operator>(const fmc::time &jt, const fm_time64_t &et) {
  return jt > std::chrono::nanoseconds(fm_time64_to_nanos(et));
}

/**
 * @brief Greater than operator overload for fm_time64_t and fmc time
 * objects
 *
 * @param jt Platform time object.
 * @param et fm_time64_t object.
 *
 * @return Result of comparison.
 */
inline bool operator>(const fm_time64_t &et, const fmc::time &jt) {
  return std::chrono::nanoseconds(fm_time64_to_nanos(et)) > jt;
}

} // namespace std

namespace fmc {
template <> struct conversion<fm_time64_t, fmc::time> {
  fmc::time operator()(fm_time64_t x) {
    return std::chrono::nanoseconds(fm_time64_to_nanos(x));
  }
};

template <> struct conversion<fmc::time, fm_time64_t> {
  fm_time64_t operator()(fmc::time x) {
    return fm_time64_from_nanos(std::chrono::nanoseconds(x).count());
  }
};
} // namespace fmc

namespace std {
template <> struct hash<fm_time64_t> {
  using argument_type = fm_time64_t;
  using result_type = std::size_t;
  result_type operator()(argument_type const &obj) const {
    return std::hash<decltype(obj.value)>{}(obj.value);
  }
};
} // namespace std
