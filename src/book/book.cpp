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
 * @file book.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the book interface
 *
 * This file contains declarations of the book interface
 * @see http://www.featuremine.com
 */

extern "C" {
#include "book/book.h"
#include "fmc/decimal128.h"
#include "fmc/time.h"
}

#include "fmc++/decimal128.hpp"
#include "fmc++/time.hpp"

#include <algorithm>
#include <array>
#include <vector>

#include <iostream>

using namespace std;

struct fm_order {
  uint64_t prio = 0;
  uint64_t id = 0;
  fmc::decimal128 qty = fmc::decimal128(0);
  fmc_time64_t rec = {0};
  fmc_time64_t ven = {0};
  uint64_t seq = 0;
};

using fm_orders = vector<fm_order>;

struct fm_level {
  fmc::decimal128 price = fmc::decimal128(0);
  fmc::decimal128 qty = fmc::decimal128(0);
  fm_orders orders;
};

using vector_levels = vector<fm_level>;

struct fm_levels {
  vector_levels levels;
};

struct fm_book {
  array<fm_levels, 2> levels;
  vector<fm_orders> pool;
  bool uncross = false;
  uint64_t uncrossed = 0;
  uint64_t missing = 0;
};

struct fm_book_shared {
  uint64_t ref_count_ = 1;
  fm_book_t book_;
};

vector<fm_level> &levels_vector(fm_book_t *book, bool is_bid) {
  return book->levels[!is_bid].levels;
}

struct compare_levels {
  compare_levels(bool is_bid) : bid_(is_bid) {}
  bool operator()(fmc_decimal128_t a, fmc_decimal128_t b) const {
    return bid_ ? a > b : b > a;
  }
  bool bid_;
};

fm_book_t *fm_book_new() { return new fm_book(); }

void fm_book_del(fm_book_t *book) { delete book; }

void fm_book_uncross_set(fm_book_t *book, bool set) { book->uncross = set; }

template <class Pool> void source_pool(Pool &pool, fm_orders &orders) {
  if (!pool.empty()) {
    swap(pool.back(), orders);
    pool.pop_back();
  }
}

template <class Pool> void recycle_pool(Pool &pool, fm_orders &orders) {
  pool.push_back({});
  swap(pool.back(), orders);
}

template <class Pool>
vector_levels::iterator create_level(vector_levels &lvls, Pool &pool,
                                     vector_levels::iterator it,
                                     fmc_decimal128_t price) {
  auto where = lvls.insert(it, {price, fmc::decimal128(0)});
  source_pool(pool, where->orders);
  return where;
}

auto bounding_level(vector_levels &levels, compare_levels better,
                    fmc_decimal128_t price) {
  auto where = levels.end();
  unsigned idx = 0;
  for (; where != levels.begin() && idx < 4; ++idx, --where) {
    if (better(price, (where - 1)->price))
      break;
  }
  if (idx == 4) {
    where = lower_bound(levels.begin(), where, price,
                        [=](const fm_level &lvl, fmc_decimal128_t a) {
                          return better(a, lvl.price);
                        });
  }
  return where;
}

fm_level &find_or_add(fm_book_t *book, fmc_decimal128_t price, bool is_bid) {
  auto &levels = levels_vector(book, is_bid);
  compare_levels better(is_bid);
  auto where = bounding_level(levels, better, price);
  return where == levels.end() || better(where->price, price)
             ? *create_level(levels, book->pool, where, price)
             : *where;
}

vector_levels::iterator find_level(vector_levels &levels,
                                   fmc_decimal128_t price, bool is_bid) {
  compare_levels better(is_bid);
  auto where = bounding_level(levels, better, price);
  return where == levels.end() || better(where->price, price) ? levels.end()
                                                              : where;
}

vector_levels::iterator front_level(vector_levels &levels,
                                    fmc_decimal128_t price, bool is_bid,
                                    bool uncross, uint64_t &uncrossed) {
  compare_levels better(is_bid);
  auto where = levels.end();
  if (where != levels.begin() &&
      (where - 1)->price == fmc::decimal128::upcast(price)) {
    return where - 1;
  }
  if (uncross) {
    while (where != levels.begin() && better((where - 1)->price, price)) {
      where = levels.erase(where - 1);
      ++uncrossed;
    }
    if (where == levels.end() || better(price, (where - 1)->price)) {
      return levels.end();
    }
    return where;
  } else {
    return find_level(levels, price, is_bid);
  }
}

fm_order &append_order(fm_orders &orders) {
  orders.push_back({});
  return orders.back();
}

fm_order &position_order(fm_orders &orders, uint32_t pos) {
  auto idx = std::min((size_t)pos, orders.size());
  return *orders.insert(orders.begin() + idx, fm_order{});
}

fm_order &insert_order(fm_orders &orders, uint64_t prio) {
  auto where = upper_bound(orders.begin(), orders.end(), prio,
                           [](uint64_t p, auto &o) { return p < o.prio; });
  return *orders.insert(where, fm_order{});
}

fm_orders::iterator back_find(fm_orders &orders, uint64_t id) {
  for (auto where = orders.end(); where != orders.begin();) {
    if ((--where)->id == id) {
      return where;
    }
  }
  return orders.end();
}

fm_orders::iterator front_find(fm_orders &orders, uint64_t id) {
  for (auto where = orders.begin(); where != orders.end(); ++where) {
    if (where->id == id) {
      return where;
    }
  }
  return orders.end();
}

void fm_book_add(fm_book_t *book, fmc_time64_t rec, fmc_time64_t ven,
                 uint64_t seq, uint64_t id, fmc_decimal128_t price,
                 fmc_decimal128_t qty, bool is_bid) {
  auto &level = find_or_add(book, price, is_bid);
  level.qty += qty;
  auto &order = append_order(level.orders);
  order.prio = 0;
  order.id = id;
  order.qty = qty;
  order.rec = rec;
  order.ven = ven;
  order.seq = seq;
}

void fm_book_ins(fm_book_t *book, fmc_time64_t rec, fmc_time64_t ven,
                 uint64_t seq, uint64_t id, uint64_t prio,
                 fmc_decimal128_t price, fmc_decimal128_t qty, bool is_bid) {
  auto &level = find_or_add(book, price, is_bid);
  level.qty += qty;
  auto &order = insert_order(level.orders, prio);
  order.prio = prio;
  order.id = id;
  order.qty = qty;
  order.rec = rec;
  order.ven = ven;
  order.seq = seq;
}

void fm_book_pos(fm_book_t *book, fmc_time64_t rec, fmc_time64_t ven,
                 uint64_t seq, uint64_t id, uint32_t pos,
                 fmc_decimal128_t price, fmc_decimal128_t qty, bool is_bid) {
  auto &level = find_or_add(book, price, is_bid);
  level.qty += qty;
  auto &order = position_order(level.orders, pos);
  order.prio = 0;
  order.id = id;
  order.qty = qty;
  order.rec = rec;
  order.ven = ven;
  order.seq = seq;
}

bool fm_book_mod(fm_book_t *book, uint64_t id, fmc_decimal128_t price,
                 fmc_decimal128_t qty, bool is_bid) {
  auto &levels = levels_vector(book, is_bid);
  auto level_it = find_level(levels, price, is_bid);
  if (level_it == levels.end()) {
    ++book->missing;
    return false;
  }
  auto &level = *level_it;
  auto &orders = level.orders;
  auto order_it = back_find(orders, id);
  if (order_it == orders.end()) {
    ++book->missing;
    return false;
  }
  auto &order = *order_it;
  if (fmc::decimal128::upcast(qty) < order.qty) {
    level.qty -= qty;
    order.qty -= qty;
    return true;
  }
  level.qty -= order.qty;
  orders.erase(order_it);
  if (orders.empty()) {
    recycle_pool(book->pool, level.orders);
    levels.erase(level_it);
  }
  return true;
}

bool fm_book_exe(fm_book_t *book, uint64_t id, fmc_decimal128_t price,
                 fmc_decimal128_t qty, bool is_bid) {
  auto &levels = levels_vector(book, is_bid);
  auto level_it =
      front_level(levels, price, is_bid, book->uncross, book->uncrossed);
  if (level_it == levels.end()) {
    ++book->missing;
    return false;
  }
  auto &level = *level_it;
  auto &orders = level.orders;
  auto order_it = front_find(orders, id);
  if (order_it == orders.end()) {
    ++book->missing;
    return false;
  }
  auto &order = *order_it;
  if (fmc::decimal128::upcast(qty) < order.qty) {
    level.qty -= qty;
    order.qty -= qty;
    return true;
  }
  level.qty -= order.qty;
  orders.erase(order_it);
  if (orders.empty()) {
    recycle_pool(book->pool, level.orders);
    levels.erase(level_it);
  }
  return true;
}

bool fm_book_pla(fm_book_t *book, fmc_time64_t rec, fmc_time64_t ven,
                 uint64_t seq, fmc_decimal128_t price, fmc_decimal128_t qty,
                 bool is_bid) {
  if (fmc::decimal128::upcast(qty) > fmc::decimal128(0)) {
    auto &level = find_or_add(book, price, is_bid);
    level.qty = qty;
    level.orders.resize(1);
    auto &order = level.orders.back();
    order.prio = 0;
    order.id = 0;
    order.qty = qty;
    order.rec = rec;
    order.ven = ven;
    order.seq = seq;
  } else {
    auto &levels = levels_vector(book, is_bid);
    auto level_it = find_level(levels, price, is_bid);
    if (level_it == levels.end()) {
      ++book->missing;
      return false;
    }
    recycle_pool(book->pool, level_it->orders);
    levels.erase(level_it);
  }
  return true;
}

void fm_book_clr(fm_book_t *book) {
  for (auto &level : book->levels) {
    for (auto &in_level : level.levels) {
      in_level.orders.clear();
      recycle_pool(book->pool, in_level.orders);
    }
    level.levels.clear();
  }
}

fm_levels_t *fm_book_levels(fm_book_t *book, bool is_bid) {
  return &book->levels[!is_bid];
}

unsigned fm_book_levels_size(fm_levels_t *lvls) { return lvls->levels.size(); }

fm_level_t *fm_book_level(fm_levels_t *lvls, unsigned idx) {
  return &(*(lvls->levels.end() - 1 - idx));
}

fmc_decimal128_t fm_book_level_prx(fm_level_t *lvl) { return lvl->price; }

fmc_decimal128_t fm_book_level_shr(fm_level_t *lvl) { return lvl->qty; }

uint32_t fm_book_level_ord(fm_level_t *lvl) { return lvl->orders.size(); }

fm_order_t *fm_book_level_order(fm_level_t *lvl, unsigned idx) {
  return &(*(lvl->orders.end() - 1 - idx));
}

uint64_t fm_book_order_prio(fm_order_t *ord) { return ord->prio; }

uint64_t fm_book_order_id(fm_order_t *ord) { return ord->id; }

fmc_decimal128_t fm_book_order_qty(fm_order_t *ord) { return ord->qty; }

fmc_time64_t fm_book_order_rec(fm_order_t *ord) { return ord->rec; }

fmc_time64_t fm_book_order_ven(fm_order_t *ord) { return ord->ven; }

uint64_t fm_book_order_seq(fm_order_t *ord) { return ord->seq; }

fm_book_shared_t *fm_book_shared_new() { return new fm_book_shared; }

void fm_book_shared_inc(fm_book_shared_t *shared_book) {
  ++shared_book->ref_count_;
}

void fm_book_shared_dec(fm_book_shared_t *shared_book) {
  if (--shared_book->ref_count_ == 0) {
    delete shared_book;
  }
}

fm_book_t *fm_book_shared_get(fm_book_shared_t *shared_book) {
  return &shared_book->book_;
}
