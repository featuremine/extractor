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
 * @file side.hpp
 * @author Maxim Trokhimtchouk
 * @date 15 Dec 2017
 * @brief File contains C++ definition of a rational price
 */

#pragma once

#include "fmc++/convert.hpp"

#include <array>
#include <chrono>
#include <iostream>

namespace fm {
using namespace std;

struct trade_side {
  enum SIDE { UNKNOWN = 0, BID = 1, ASK = 2 } value;
  trade_side() : value(UNKNOWN) {}
  trade_side(SIDE s) : value(s) {}
  bool operator==(const trade_side &a) const { return a.value == value; }
  bool operator!=(const trade_side &a) const { return a.value != value; }
  static array<trade_side, 2> all() { return {BID, ASK}; }
};
inline bool is_bid(trade_side side) { return side.value == trade_side::BID; }

inline bool is_ask(trade_side side) { return side.value == trade_side::ASK; }

inline bool is_unknown(trade_side side) {
  return side.value == trade_side::UNKNOWN;
}

inline trade_side other_side(trade_side side) {
  return side.value == trade_side::UNKNOWN
             ? trade_side::UNKNOWN
             : (side.value == trade_side::BID ? trade_side::ASK
                                              : trade_side::BID);
}

template <class T> struct sided_initializer {
  static constexpr bool is_specialized = false;
  static constexpr T min() noexcept { return T(); }
  static constexpr T max() noexcept { return T(); }
};

template <class T, template <class> class Initializer = sided_initializer>
struct sided : array<T, 2> {
  sided(initializer_list<T> list)
      : array<T, 2>(from_const_ptr(std::begin(list))) {}
  sided() : array<T, 2>({Initializer<T>::min(), Initializer<T>::max()}) {}
  auto &operator[](trade_side side) {
    return array<T, 2>::operator[](!is_bid(side));
  }
  auto &operator[](trade_side side) const {
    return array<T, 2>::operator[](!is_bid(side));
  }
  bool operator==(sided<T> &other) {
    return (*this)[trade_side::BID] == other[trade_side::BID] &&
           (*this)[trade_side::ASK] == other[trade_side::ASK];
  }
  bool operator!=(sided<T> &other) {
    return (*this)[trade_side::BID] != other[trade_side::BID] ||
           (*this)[trade_side::ASK] != other[trade_side::ASK];
  }

private:
  static array<T, 2> from_const_ptr(const T *it) {
    return array<T, 2>({*it, *(++it)});
  }
};

template <class T> struct better {
  better(trade_side side) : side_(side) {}
  bool operator()(const T &a, const T &b) const {
    return is_bid(side_) ? (a > b) : (a < b);
  }
  trade_side side_;
};
} // namespace fm

namespace std {
inline ostream &operator<<(ostream &s, const fm::trade_side x) {
  switch (x.value) {
  case fm::trade_side::UNKNOWN:
    s << 'U';
    break;
  case fm::trade_side::BID:
    s << 'B';
    break;
  case fm::trade_side::ASK:
    s << 'A';
    break;
  }
  return s;
}

template <class T> inline ostream &operator<<(ostream &s, fm::sided<T> &x) {
  s << "{" << x[fm::trade_side::BID] << "," << x[fm::trade_side::ASK] << "}";
  return s;
}

} // namespace std
