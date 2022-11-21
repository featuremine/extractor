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
 * @file ore.hpp
 * @authors Maxim Trokhimtchouk
 * @date 28 Oct 2018
 * @brief File contains ore parser
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/book/updates.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/serialization.hpp"
#include "fmc++/time.hpp"

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

namespace fm {
namespace book {
namespace ore {
struct result {
  enum result_enum { SUCCESS, SKIP, TIME, ANNOUNCE, ERR, ENDOFFILE };

  result() : value(SUCCESS) {}
  result(result_enum res) : value(res) {}

  bool is_success() const { return value == SUCCESS; }
  bool is_skip() const { return value == SKIP; }
  bool is_time() const { return value == TIME; }
  bool is_announce() const { return value == ANNOUNCE; }
  bool is_error() const { return value == ERR; }
  bool is_eof() const { return value == ENDOFFILE; }
  bool is_continue() const {
    switch (value) {
    case SUCCESS:
    case TIME:
    case ANNOUNCE:
    case SKIP:
      return true;
    default:
      return false;
    }
  }

  static result value_success() { return result(SUCCESS); }

  result_enum value;
};
struct order_info {
  fmc::decimal128 price;
  fmc::decimal128 qty;
  bool is_bid = 0;
};
using orders_t = unordered_map<uint64_t, order_info>;
struct imnt_info {
  int32_t px_denum;
  int32_t qty_denum = 1;
  int32_t index;
  orders_t orders;
};
using imnt_infos_t = unordered_map<int32_t, imnt_info>;
struct parser {
  parser(imnt_infos_t &infos) : imnts(infos) {}
  fmc_time64_t seconds = {0};
  fmc_time64_t time = {0};
  imnt_info *imnt = nullptr;
  imnt_infos_t &imnts;
  book::message msg;
  book::message expanded;
  bool expand = false;
  std::string error;
  result parse(cmp_ctx_t *ctx, imnt_info *ii = nullptr);

  template <class Msg>
  int32_t parse_hdr0(cmp_ctx_t *ctx, Msg &msg, uint32_t &left);
  template <class Msg>
  result parse_hdr(cmp_ctx_t *ctx, Msg &msg, uint32_t &left);
  result skip_msg(cmp_ctx_t *ctx, uint32_t &left);
  result parse_tme(cmp_ctx_t *ctx, uint32_t &left);
  result parse_add(cmp_ctx_t *ctx, uint32_t &left);
  result parse_ins(cmp_ctx_t *ctx, uint32_t &left);
  result parse_pos(cmp_ctx_t *ctx, uint32_t &left);
  result parse_cxl(cmp_ctx_t *ctx, uint32_t &left);
  result parse_del(cmp_ctx_t *ctx, uint32_t &left);
  result parse_mod(cmp_ctx_t *ctx, uint32_t &left);
  result parse_exe(cmp_ctx_t *ctx, uint32_t &left);
  result parse_epx(cmp_ctx_t *ctx, uint32_t &left);
  result parse_fld(cmp_ctx_t *ctx, uint32_t &left);
  result parse_fpx(cmp_ctx_t *ctx, uint32_t &left);
  result parse_trd(cmp_ctx_t *ctx, uint32_t &left);
  result parse_sta(cmp_ctx_t *ctx, uint32_t &left);
  result parse_ctl(cmp_ctx_t *ctx, uint32_t &left);
  result parse_set(cmp_ctx_t *ctx, uint32_t &left);
  result parse_ann(cmp_ctx_t *ctx, uint32_t &left);
  result parse_hbt(cmp_ctx_t *ctx, uint32_t &left);

  template <class Msg>
  void process_reduce(orders_t &ords, orders_t::iterator it, Msg &msg);
  template <class Msg>
  void process_remove(orders_t &ords, orders_t::iterator it, Msg &msg);

  result set_error(const std::string &msg);
  const std::string &get_error();
};

inline result parser::set_error(const std::string &msg) {
  error = msg;
  return result::ERR;
}

inline const std::string &parser::get_error() { return error; }

inline result parser::parse_tme(cmp_ctx_t *ctx, uint32_t &left) {
  int64_t sec = 0;
  if (!cmp_read_many(ctx, &left, &sec)) {
    return result::ERR;
  }
  seconds = fmc_time64_from_seconds(sec);

  book::updates::time msg{seconds};
  this->msg = msg;
  return result::TIME;
}

template <class Msg>
int32_t parser::parse_hdr0(cmp_ctx_t *ctx, Msg &msg, uint32_t &left) {
  int64_t nanoseconds = 0;
  int64_t vendoroff = 0;
  int32_t imnt_idx = 0;
  if (!cmp_read_many(ctx, &left, &nanoseconds, &vendoroff, &msg.seqn,
                     &msg.batch, &imnt_idx)) {
    return -1;
  }
  time = seconds + fmc_time64_from_nanos(nanoseconds);
  msg.vendor = time - fmc_time64_from_nanos(vendoroff);
  return imnt_idx;
}

template <class Msg>
inline result parser::parse_hdr(cmp_ctx_t *ctx, Msg &msg, uint32_t &left) {
  auto imnt_idx = parse_hdr0(ctx, msg, left);
  if (imnt_idx < 0) {
    return result::ERR;
  }
  if (imnt)
    return result::SUCCESS;
  auto where = imnts.find(imnt_idx);
  if (where == imnts.end()) {
    imnt = nullptr;
    return result::SKIP;
  }
  imnt = &where->second;
  return result::SUCCESS;
}

inline result parser::skip_msg(cmp_ctx_t *ctx, uint32_t &left) {
  struct {
    fmc_time64_t vendor;
    uint64_t seqn;
    uint64_t id;
    uint16_t batch;
  } msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  return result::SKIP;
}

template <class Msg> void add_order(imnt_info *imnt, const Msg &msg) {
  auto &ord = imnt->orders[msg.id];
  ord.price = msg.price;
  ord.qty = msg.qty;
  ord.is_bid = msg.is_bid;
}

inline result parser::parse_add(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::add msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  bool is_bid = false;
  if (!cmp_read_many(ctx, &left, &msg.id, &msg.price, &msg.qty, &is_bid))
    return result::ERR;
  msg.is_bid = is_bid;
  if (imnt->px_denum != 1)
    msg.price = msg.price / imnt->px_denum;
  if (imnt->qty_denum != 1)
    msg.qty = msg.qty / imnt->qty_denum;
  add_order(imnt, msg);
  this->msg = msg;
  return result::SUCCESS;
}

inline result parser::parse_ins(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::insert msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  bool is_bid = false;
  if (!cmp_read_many(ctx, &left, &msg.id, &msg.prio, &msg.price, &msg.qty,
                     &is_bid))
    return result::ERR;
  msg.is_bid = is_bid;
  if (imnt->px_denum != 1)
    msg.price = msg.price / imnt->px_denum;
  if (imnt->qty_denum != 1)
    msg.qty = msg.qty / imnt->qty_denum;
  add_order(imnt, msg);
  this->msg = msg;
  return result::SUCCESS;
};

inline result parser::parse_pos(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::position msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  bool is_bid = false;
  if (!cmp_read_many(ctx, &left, &msg.id, &msg.pos, &msg.price, &msg.qty,
                     &is_bid))
    return result::ERR;
  msg.is_bid = is_bid;
  if (imnt->px_denum != 1)
    msg.price = msg.price / imnt->px_denum;
  if (imnt->qty_denum != 1)
    msg.qty = msg.qty / imnt->qty_denum;
  add_order(imnt, msg);
  this->msg = msg;
  return result::SUCCESS;
};

template <class Msg>
void parser::process_reduce(orders_t &ords, orders_t::iterator it, Msg &msg) {
  auto &ord = it->second;
  msg.price = ord.price;
  msg.is_bid = ord.is_bid;
  if (ord.qty <= msg.qty) {
    ords.erase(it);
  }
}

template <class Msg>
void parser::process_remove(orders_t &ords, orders_t::iterator it, Msg &msg) {
  auto &ord = it->second;
  msg.price = ord.price;
  msg.qty = ord.qty;
  msg.is_bid = ord.is_bid;
  ords.erase(it);
}

inline result parser::parse_cxl(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::cancel msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }

  if (!cmp_read_many(ctx, &left, &msg.id, &msg.qty))
    return result::ERR;
  if (imnt->qty_denum != 1)
    msg.qty = msg.qty / imnt->qty_denum;
  auto &ords = imnt->orders;
  if (auto it = ords.find(msg.id); it == ords.end()) {
    return result::SKIP;
  } else {
    process_reduce(ords, it, msg);
    this->msg = msg;
    return result::SUCCESS;
  }
};

inline result parser::parse_del(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::cancel msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  if (!cmp_read_many(ctx, &left, &msg.id))
    return result::ERR;
  auto &ords = imnt->orders;
  if (auto it = ords.find(msg.id); it == ords.end()) {
    return result::SKIP;
  } else {
    process_remove(ords, it, msg);
    this->msg = msg;
    return result::SUCCESS;
  }
};

inline result parser::parse_mod(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::cancel cancel;
  if (auto res = parse_hdr(ctx, cancel, left); !res.is_success()) {
    return res;
  }
  book::updates::add add;
  add.vendor = cancel.vendor;
  add.seqn = cancel.seqn;
  add.batch = cancel.batch;
  if (!cmp_read_many(ctx, &left, &cancel.id, &add.id, &add.price, &add.qty))
    return result::ERR;
  if (imnt->px_denum != 1)
    add.price = add.price / imnt->px_denum;
  if (imnt->qty_denum != 1)
    add.qty = add.qty / imnt->qty_denum;
  auto &ords = imnt->orders;
  if (auto it = ords.find(cancel.id); it == ords.end()) {
    return result::SKIP;
  } else {
    // first cancel message is always inside a batch
    // we do not wan't to update the book with this 'fake' cancel
    cancel.batch = 1;
    // add.batch is equal to the batch value of the modify message

    process_remove(ords, it, cancel);
    add.is_bid = cancel.is_bid;
    msg = cancel;
    add_order(imnt, add);
    expand = true;
    expanded = add;
    return result::SUCCESS;
  }
};

inline result parser::parse_exe(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::execute msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  if (!cmp_read_many(ctx, &left, &msg.id))
    return result::ERR;
  auto &ords = imnt->orders;
  if (auto it = ords.find(msg.id); it == ords.end()) {
    return result::SKIP;
  } else {
    process_remove(ords, it, msg);
    msg.trade_price = msg.price;
    this->msg = msg;
    return result::SUCCESS;
  }
};

inline result parser::parse_epx(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::execute msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  if (!cmp_read_many(ctx, &left, &msg.id, &msg.trade_price))
    return result::ERR;
  if (imnt->px_denum != 1)
    msg.trade_price = msg.trade_price / imnt->px_denum;
  auto &ords = imnt->orders;
  if (auto it = ords.find(msg.id); it == ords.end()) {
    return result::SKIP;
  } else {
    process_remove(ords, it, msg);
    this->msg = msg;
    return result::SUCCESS;
  }
};

inline result parser::parse_fld(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::execute msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  if (!cmp_read_many(ctx, &left, &msg.id, &msg.qty))
    return result::ERR;
  if (imnt->qty_denum != 1)
    msg.qty = msg.qty / imnt->qty_denum;
  auto &ords = imnt->orders;
  if (auto it = ords.find(msg.id); it == ords.end()) {
    return result::SKIP;
  } else {
    process_reduce(ords, it, msg);
    msg.trade_price = msg.price;
    this->msg = msg;
    return result::SUCCESS;
  }
};

inline result parser::parse_fpx(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::execute msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  if (!cmp_read_many(ctx, &left, &msg.id, &msg.trade_price, &msg.qty))
    return result::ERR;
  if (imnt->px_denum != 1)
    msg.trade_price = msg.trade_price / imnt->px_denum;
  if (imnt->qty_denum != 1)
    msg.qty = msg.qty / imnt->qty_denum;
  auto &ords = imnt->orders;
  if (auto it = ords.find(msg.id); it == ords.end()) {
    return result::SKIP;
  } else {
    process_reduce(ords, it, msg);
    this->msg = msg;
    return result::SUCCESS;
  }
};

inline result parser::parse_trd(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::trade msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  if (!cmp_read_many(ctx, &left, &msg.trade_price, &msg.qty))
    return result::ERR;
  if (imnt->px_denum != 1)
    msg.trade_price = msg.trade_price / imnt->px_denum;
  if (imnt->qty_denum != 1)
    msg.qty = msg.qty / imnt->qty_denum;

  if (left == 0) {
    this->msg = msg;
    return result::SUCCESS;
  }

  uint32_t size = 0;
  if (!cmp_read_str_size(ctx, &size)) {
    return result::ERR;
  }

  if (size > 8 || !ctx->read(ctx, msg.decoration, size))
    return result::ERR;

  --left;

  this->msg = msg;
  return result::SUCCESS;
};

inline result parser::parse_sta(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::state msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  bool is_bid = false;
  if (!cmp_read_many(ctx, &left, &msg.id, &msg.price, &msg.state, &is_bid))
    return result::ERR;
  msg.is_bid = is_bid;
  if (imnt->px_denum != 1)
    msg.price = msg.price / imnt->px_denum;
  this->msg = msg;
  return result::SUCCESS;
}

inline result parser::parse_ctl(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::control msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  if (!cmp_read_many(ctx, &left, &msg.uncross)) {
    return result::ERR;
  }
  if (left > 0 && !cmp_read_many(ctx, &left, &msg.command)) {
    return result::ERR;
  }
  this->msg = msg;
  return result::SUCCESS;
}

inline result parser::parse_set(cmp_ctx_t *ctx, uint32_t &left) {
  book::updates::set msg;
  if (auto res = parse_hdr(ctx, msg, left); !res.is_success()) {
    return res;
  }
  bool is_bid = false;
  if (!cmp_read_many(ctx, &left, &msg.price, &msg.qty, &is_bid))
    return result::ERR;
  msg.is_bid = is_bid;
  if (imnt->px_denum != 1)
    msg.price = msg.price / imnt->px_denum;
  if (imnt->qty_denum != 1)
    msg.qty = msg.qty / imnt->qty_denum;
  this->msg = msg;
  return result::SUCCESS;
}

inline result parser::parse_ann(cmp_ctx_t *ctx, uint32_t &left) {
  struct {
    fmc_time64_t vendor;
    uint64_t seqn;
    uint64_t id;
    uint16_t batch;
  } tmp;
  auto imnt_idx = parse_hdr0(ctx, tmp, left);
  if (imnt_idx < 0) {
    return result::ERR;
  }
  book::updates::announce msg;
  msg.imnt_idx = imnt_idx;
  if (!cmp_read_many(ctx, &left, &msg.symbol, &msg.tick))
    return result::ERR;
  if (left > 0) {
    int32_t qty_tick;
    if (cmp_read_many(ctx, &left, &qty_tick)) {
      msg.qty_tick = qty_tick;
    }
  }
  this->msg = msg;
  return result::ANNOUNCE;
}

inline result parser::parse_hbt(cmp_ctx_t *ctx, uint32_t &left) {
  int64_t nanoseconds = 0;
  int64_t vendor = 0;
  if (!cmp_read_many(ctx, &left, &nanoseconds, &vendor)) {
    return result::ERR;
  }

  time = seconds + fmc_time64_from_nanos(nanoseconds);

  book::updates::heartbeat msg;
  msg.vendor = fmc_time64_from_nanos(vendor);
  this->msg = msg;

  return result::SUCCESS;
}

inline result parser::parse(cmp_ctx_t *ctx, imnt_info *ii) {
  result res;
  uint8_t type = 0;
  uint32_t left = 0;
  imnt = ii;
  if (!cmp_read_array(ctx, &left)) {
    if (ctx->error == TYPE_MARKER_READING_ERROR && feof((FILE *)ctx->buf)) {
      return result::ENDOFFILE;
    }
    return set_error("failed to read ORE message header, expecting array");
  }

  if (!cmp_read_many(ctx, &left, &type))
    return set_error("failed to read ORE message type");

  switch (type) {
  case 0:
    res = parse_tme(ctx, left);
    break;
  case 1:
    res = parse_add(ctx, left);
    break;
  case 2:
    res = parse_ins(ctx, left);
    break;
  case 3:
    res = parse_pos(ctx, left);
    break;
  case 4:
    res = parse_cxl(ctx, left);
    break;
  case 5:
    res = parse_del(ctx, left);
    break;
  case 6:
    res = parse_mod(ctx, left);
    break;
  case 7:
    res = parse_exe(ctx, left);
    break;
  case 8:
    res = parse_epx(ctx, left);
    break;
  case 9:
    res = parse_fld(ctx, left);
    break;
  case 10:
    res = parse_fpx(ctx, left);
    break;
  case 11:
    res = parse_trd(ctx, left);
    break;
  case 12:
    res = parse_sta(ctx, left);
    break;
  case 13:
    res = parse_ctl(ctx, left);
    break;
  case 14:
    res = parse_set(ctx, left);
    break;
  case 15:
    res = parse_ann(ctx, left);
    break;
  case 16:
    res = parse_hbt(ctx, left);
    break;
  default:
    res = skip_msg(ctx, left);
    break;
  };
  if (res.is_error()) {
    std::string error_msg("failed to parse ORE message of type ");
    error_msg += std::to_string(type);
    return set_error(error_msg);
  } else {
    if (!cmp_read_rest(ctx, left)) {
      std::string error_msg(
          "failed to parse the rest of the ORE message of type ");
      error_msg += std::to_string(type);
      return set_error(error_msg);
    }
  }
  return res;
}

inline bool read_version(cmp_ctx_t *ctx, uint16_t ver[3]) {
  uint32_t left = 0;
  return cmp_read_array(ctx, &left) && left == 3 &&
         cmp_read_many(ctx, &left, &ver[0], &ver[1], &ver[2]);
}

constexpr uint16_t version[3] = {1, 1, 3};

// @note parser can read files with the same major version and higher minor
// version
inline bool validate_version(uint16_t ver[3]) {
  return ver[0] == version[0] && ver[1] <= version[1];
}

struct symbol_info {
  int32_t px_denum;
  int32_t qty_denum = 1;
  uint32_t index;
};

using header = unordered_map<string, symbol_info>;
inline bool read_hdr(cmp_ctx_t *ctx, header &hdr) {
  uint32_t left = 0;
  if (!cmp_read_array(ctx, &left))
    return false;
  string name;
  name.reserve(256);
  for (unsigned i = 0; i < left; ++i) {
    uint32_t map_sz = 0;
    if (!cmp_read_map(ctx, &map_sz))
      return false;
    int count = 0;
    auto where = hdr.end();
    symbol_info info;
    info.index = i;
    for (unsigned eidx = 0; eidx < map_sz; ++eidx) {
      if (!cmp_read_string(ctx, name))
        return false;
      if (name == "symbol") {
        if (!cmp_read_string(ctx, name))
          return false;
        where = hdr.emplace(name, info).first;
      } else if (name == "price_tick") {
        if (!cmp_read_item(ctx, &info.px_denum))
          ;
      } else if (name == "qty_tick") {
        if (!cmp_read_item(ctx, &info.qty_denum))
          ;
      }

      else {
        if (!cmp_skip_object(ctx, NULL))
          return false;
        continue;
      }
      ++count;
    }
    if ((count < 2 || count > 3) || where == hdr.end())
      return false;
    where->second = info;
  }
  return true;
}

} // namespace ore
} // namespace book
} // namespace fm
