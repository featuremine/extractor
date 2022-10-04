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
 * @file updates.hpp
 * @authors Maxim Trokhimtchouk
 * @date 29 Oct 2018
 * @brief File contains C++ definitions of book updates structures
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include "decimal64.hpp"
#include "time64.hpp"

#include <optional>
#include <string>
#include <variant>

namespace fm {
namespace book {
namespace updates {
using namespace std;
struct add {
  fm_time64_t vendor;
  uint64_t seqn;
  uint64_t id;
  fm_decimal64_t price;
  fm_decimal64_t qty;
  uint16_t is_bid;
  uint16_t batch;
};

struct insert {
  fm_time64_t vendor;
  uint64_t seqn;
  uint64_t id;
  uint64_t prio;
  fm_decimal64_t price;
  fm_decimal64_t qty;
  uint16_t is_bid;
  uint16_t batch;
};

struct position {
  fm_time64_t vendor;
  uint64_t seqn;
  uint64_t id;
  fm_decimal64_t price;
  uint32_t pos;
  fm_decimal64_t qty;
  uint16_t is_bid;
  uint16_t batch;
};

struct cancel {
  fm_time64_t vendor;
  uint64_t seqn;
  uint64_t id;
  fm_decimal64_t price;
  fm_decimal64_t qty;
  uint16_t is_bid;
  uint16_t batch;
};

struct execute {
  fm_time64_t vendor;
  uint64_t seqn;
  uint64_t id;
  fm_decimal64_t price;
  fm_decimal64_t trade_price;
  fm_decimal64_t qty;
  uint16_t is_bid;
  uint16_t batch;
};

struct trade {
  fm_time64_t vendor;
  uint64_t seqn;
  fm_decimal64_t trade_price;
  fm_decimal64_t qty;
  uint16_t batch;
  char decoration[8] = {0, 0, 0, 0, 0, 0, 0, 0};
};

struct state {
  fm_time64_t vendor;
  uint64_t seqn;
  uint64_t id;
  fm_decimal64_t price;
  uint32_t state;
  uint16_t is_bid;
  uint16_t batch;
};

struct control {
  fm_time64_t vendor;
  uint64_t seqn;
  uint16_t batch;
  uint8_t uncross;
  char command = 0;
};

struct set {
  fm_time64_t vendor;
  uint64_t seqn;
  fm_decimal64_t price;
  fm_decimal64_t qty;
  uint16_t is_bid;
  uint16_t batch;
};

struct announce {
  string symbol;
  uint32_t imnt_idx;
  int32_t tick;
  int32_t qty_tick = 1;
  uint16_t batch;
};

struct time {
  fm_time64_t seconds;
};

struct none {};
} // namespace updates
using message = std::variant<updates::add, updates::insert, updates::position,
                             updates::cancel, updates::execute, updates::trade,
                             updates::state, updates::control, updates::set,
                             updates::announce, updates::time, updates::none>;

static_assert(sizeof(message) <= 64, "expecting book update message to be less "
                                     "than 64B");

} // namespace book
} // namespace fm
