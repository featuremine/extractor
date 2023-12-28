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
 * @file book_msg.cpp
 * @author Andres Rangel
 * @date 10 Jan 2018
 * @brief File contains C implementation of the book message operator
 *
 * This file contains implementation of the book message operator, which
 * take book updates as input and returns a frame with the desired the
 * book updates of the desired type.
 */

#include "book_msg.h"
#include "extractor/arg_stack.h"
#include "extractor/book/book.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"

#include "extractor/book/updates.hpp"
#include "fmc++/fxpt128.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/time.hpp"

#include <string>
#include <variant>
#include <vector>

using namespace std;
using namespace fm;
using namespace book;

class book_event_op {
public:
  virtual ~book_event_op() {}
  fm_type_decl_cp type() { return type_; }
  virtual void init(fm_frame_t *result) = 0;
  virtual bool exec(const book::message &, fm_frame_t *result) = 0;
  fm_type_decl_cp type_ = nullptr;
};

class add_event_op : public book_event_op {
public:
  add_event_op(fm_type_sys_t *sys) {
    const int nf = 7;
    int dims[1] = {1};
    const char *names[nf] = {"vendor", "seqn",   "id",   "price",
                             "qty",    "is_bid", "batch"};
    fm_type_decl_cp types[nf] = {
        fm_base_type_get(sys, FM_TYPE_TIME64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_UINT16),
        fm_base_type_get(sys, FM_TYPE_UINT16),
    };
    type_ = fm_frame_type_get1(sys, nf, names, types, 1, dims);
    vendor_field_ = fm_type_frame_field_idx(type_, "vendor");
    seqn_field_ = fm_type_frame_field_idx(type_, "seqn");
    id_field_ = fm_type_frame_field_idx(type_, "id");
    price_field_ = fm_type_frame_field_idx(type_, "price");
    qty_field_ = fm_type_frame_field_idx(type_, "qty");
    is_bid_field_ = fm_type_frame_field_idx(type_, "is_bid");
    batch_field_ = fm_type_frame_field_idx(type_, "batch");
  }
  void init(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fmc_time64_start();
    *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = 0UL;
    *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = 0UL;
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0), 0);
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0), 0);
    *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = 0;
    *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = 0;
  }
  bool exec(const book::message &msg, fm_frame_t *result) override {
    if (auto pval = std::get_if<book::updates::add>(&msg)) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
          pval->vendor;
      *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = pval->seqn;
      *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = pval->id;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0) =
          pval->price;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0) = pval->qty;
      *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = pval->is_bid;
      *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = pval->batch;
      return true;
    }
    return false;
  }
  fm_field_t vendor_field_, seqn_field_, id_field_, price_field_, qty_field_,
      is_bid_field_, batch_field_;
};

class insert_event_op : public book_event_op {
public:
  insert_event_op(fm_type_sys_t *sys) {
    const int nf = 8;
    int dims[1] = {1};
    const char *names[nf] = {"vendor", "seqn", "id",     "prio",
                             "price",  "qty",  "is_bid", "batch"};
    fm_type_decl_cp types[nf] = {
        fm_base_type_get(sys, FM_TYPE_TIME64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_UINT16),
        fm_base_type_get(sys, FM_TYPE_UINT16),
    };
    type_ = fm_frame_type_get1(sys, nf, names, types, 1, dims);
    vendor_field_ = fm_type_frame_field_idx(type_, "vendor");
    seqn_field_ = fm_type_frame_field_idx(type_, "seqn");
    id_field_ = fm_type_frame_field_idx(type_, "id");
    prio_field_ = fm_type_frame_field_idx(type_, "prio");
    price_field_ = fm_type_frame_field_idx(type_, "price");
    qty_field_ = fm_type_frame_field_idx(type_, "qty");
    is_bid_field_ = fm_type_frame_field_idx(type_, "is_bid");
    batch_field_ = fm_type_frame_field_idx(type_, "batch");
  }
  void init(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fmc_time64_start();
    *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = 0UL;
    *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = 0UL;
    *(uint64_t *)fm_frame_get_ptr1(result, prio_field_, 0) = 0UL;
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0), 0);
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0), 0);
    *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = 0;
    *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = 0;
  }
  bool exec(const book::message &msg, fm_frame_t *result) override {
    if (auto pval = std::get_if<book::updates::insert>(&msg)) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
          pval->vendor;
      *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = pval->seqn;
      *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = pval->id;
      *(uint64_t *)fm_frame_get_ptr1(result, prio_field_, 0) = pval->prio;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0) =
          pval->price;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0) = pval->qty;
      *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = pval->is_bid;
      *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = pval->batch;
      return true;
    }
    return false;
  }
  fm_field_t vendor_field_, seqn_field_, id_field_, prio_field_, price_field_,
      qty_field_, is_bid_field_, batch_field_;
};

class position_event_op : public book_event_op {
public:
  position_event_op(fm_type_sys_t *sys) {
    const int nf = 8;
    int dims[1] = {1};
    const char *names[nf] = {"vendor", "seqn", "id",     "price",
                             "pos",    "qty",  "is_bid", "batch"};
    fm_type_decl_cp types[nf] = {
        fm_base_type_get(sys, FM_TYPE_TIME64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_UINT32),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_UINT16),
        fm_base_type_get(sys, FM_TYPE_UINT16),
    };
    type_ = fm_frame_type_get1(sys, nf, names, types, 1, dims);
    vendor_field_ = fm_type_frame_field_idx(type_, "vendor");
    seqn_field_ = fm_type_frame_field_idx(type_, "seqn");
    id_field_ = fm_type_frame_field_idx(type_, "id");
    price_field_ = fm_type_frame_field_idx(type_, "price");
    pos_field_ = fm_type_frame_field_idx(type_, "pos");
    qty_field_ = fm_type_frame_field_idx(type_, "qty");
    is_bid_field_ = fm_type_frame_field_idx(type_, "is_bid");
    batch_field_ = fm_type_frame_field_idx(type_, "batch");
  }
  void init(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fmc_time64_start();
    *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = 0UL;
    *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = 0UL;
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0), 0);
    *(uint32_t *)fm_frame_get_ptr1(result, pos_field_, 0) = 0;
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0), 0);
    *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = 0;
    *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = 0;
  }
  bool exec(const book::message &msg, fm_frame_t *result) override {
    if (auto pval = std::get_if<book::updates::position>(&msg)) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
          pval->vendor;
      *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = pval->seqn;
      *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = pval->id;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0) =
          pval->price;
      *(uint32_t *)fm_frame_get_ptr1(result, pos_field_, 0) = pval->pos;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0) = pval->qty;
      *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = pval->is_bid;
      *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = pval->batch;
      return true;
    }
    return false;
  }
  fm_field_t vendor_field_, seqn_field_, id_field_, price_field_, pos_field_,
      qty_field_, is_bid_field_, batch_field_;
};

class cancel_event_op : public book_event_op {
public:
  cancel_event_op(fm_type_sys_t *sys) {
    const int nf = 7;
    int dims[1] = {1};
    const char *names[nf] = {"vendor", "seqn",   "id",   "price",
                             "qty",    "is_bid", "batch"};
    fm_type_decl_cp types[nf] = {
        fm_base_type_get(sys, FM_TYPE_TIME64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_UINT16),
        fm_base_type_get(sys, FM_TYPE_UINT16),
    };
    type_ = fm_frame_type_get1(sys, nf, names, types, 1, dims);
    vendor_field_ = fm_type_frame_field_idx(type_, "vendor");
    seqn_field_ = fm_type_frame_field_idx(type_, "seqn");
    id_field_ = fm_type_frame_field_idx(type_, "id");
    price_field_ = fm_type_frame_field_idx(type_, "price");
    qty_field_ = fm_type_frame_field_idx(type_, "qty");
    is_bid_field_ = fm_type_frame_field_idx(type_, "is_bid");
    batch_field_ = fm_type_frame_field_idx(type_, "batch");
  }
  void init(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fmc_time64_start();
    *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = 0UL;
    *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = 0UL;
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0), 0);
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0), 0);
    *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = 0;
    *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = 0;
  }
  bool exec(const book::message &msg, fm_frame_t *result) override {
    if (auto pval = std::get_if<book::updates::cancel>(&msg)) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
          pval->vendor;
      *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = pval->seqn;
      *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = pval->id;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0) =
          pval->price;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0) = pval->qty;
      *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = pval->is_bid;
      *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = pval->batch;
      return true;
    }
    return false;
  }
  fm_field_t vendor_field_, seqn_field_, id_field_, price_field_, qty_field_,
      is_bid_field_, batch_field_;
};

class execute_event_op : public book_event_op {
public:
  execute_event_op(fm_type_sys_t *sys) {
    const int nf = 8;
    int dims[1] = {1};
    const char *names[nf] = {"vendor",      "seqn", "id",     "price",
                             "trade_price", "qty",  "is_bid", "batch"};
    fm_type_decl_cp types[nf] = {
        fm_base_type_get(sys, FM_TYPE_TIME64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_UINT16),
        fm_base_type_get(sys, FM_TYPE_UINT16),
    };
    type_ = fm_frame_type_get1(sys, nf, names, types, 1, dims);
    vendor_field_ = fm_type_frame_field_idx(type_, "vendor");
    seqn_field_ = fm_type_frame_field_idx(type_, "seqn");
    id_field_ = fm_type_frame_field_idx(type_, "id");
    price_field_ = fm_type_frame_field_idx(type_, "price");
    trade_price_field_ = fm_type_frame_field_idx(type_, "trade_price");
    qty_field_ = fm_type_frame_field_idx(type_, "qty");
    is_bid_field_ = fm_type_frame_field_idx(type_, "is_bid");
    batch_field_ = fm_type_frame_field_idx(type_, "batch");
  }
  void init(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fmc_time64_start();
    *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = 0UL;
    *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = 0UL;
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0), 0);
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, trade_price_field_, 0), 0);
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0), 0);
    *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = 0;
    *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = 0;
  }
  bool exec(const book::message &msg, fm_frame_t *result) override {
    if (auto pval = std::get_if<book::updates::execute>(&msg)) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
          pval->vendor;
      *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = pval->seqn;
      *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = pval->id;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0) =
          pval->price;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, trade_price_field_, 0) =
          pval->trade_price;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0) = pval->qty;
      *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = pval->is_bid;
      *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = pval->batch;
      return true;
    }
    return false;
  }
  fm_field_t vendor_field_, seqn_field_, id_field_, price_field_,
      trade_price_field_, qty_field_, is_bid_field_, batch_field_;
};

class trade_event_op : public book_event_op {
public:
  trade_event_op(fm_type_sys_t *sys) {
    const int nf = 10;
    int dims[1] = {1};
    const char *names[nf] = {"vendor",          "seqn",
                             "trade_price",     "qty",
                             "batch",           "decoration",
                             "sale_condition",  "sale_condition2",
                             "sale_condition3", "sale_condition4"};
    fm_type_decl_cp types[nf] = {
        fm_base_type_get(sys, FM_TYPE_TIME64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_UINT16),
        fm_array_type_get(sys, fm_base_type_get(sys, FM_TYPE_CHAR), 4),
        fm_base_type_get(sys, FM_TYPE_UINT8),
        fm_base_type_get(sys, FM_TYPE_UINT8),
        fm_base_type_get(sys, FM_TYPE_UINT8),
        fm_base_type_get(sys, FM_TYPE_UINT8)};
    type_ = fm_frame_type_get1(sys, nf, names, types, 1, dims);
    vendor_field_ = fm_type_frame_field_idx(type_, "vendor");
    seqn_field_ = fm_type_frame_field_idx(type_, "seqn");
    trade_price_field_ = fm_type_frame_field_idx(type_, "trade_price");
    qty_field_ = fm_type_frame_field_idx(type_, "qty");
    batch_field_ = fm_type_frame_field_idx(type_, "batch");
    decoration_field_ = fm_type_frame_field_idx(type_, "decoration");
    sale_condition_field_ = fm_type_frame_field_idx(type_, "sale_condition");
    sale_condition2_field_ = fm_type_frame_field_idx(type_, "sale_condition2");
    sale_condition3_field_ = fm_type_frame_field_idx(type_, "sale_condition3");
    sale_condition4_field_ = fm_type_frame_field_idx(type_, "sale_condition4");
  }
  void init(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fmc_time64_start();
    *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = 0UL;
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, trade_price_field_, 0), 0);
    *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0) = fmc::fxpt128();
    *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = 0;
    memset((char *)fm_frame_get_ptr1(result, decoration_field_, 0), 0,
           sizeof(char) * 4);
    *(uint8_t *)fm_frame_get_ptr1(result, sale_condition_field_, 0) = 0;
    *(uint8_t *)fm_frame_get_ptr1(result, sale_condition2_field_, 0) = 0;
    *(uint8_t *)fm_frame_get_ptr1(result, sale_condition3_field_, 0) = 0;
    *(uint8_t *)fm_frame_get_ptr1(result, sale_condition4_field_, 0) = 0;
  }
  bool exec(const book::message &msg, fm_frame_t *result) override {
    if (auto pval = std::get_if<book::updates::trade>(&msg)) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
          pval->vendor;
      *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = pval->seqn;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, trade_price_field_, 0) =
          pval->trade_price;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0) = pval->qty;
      *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = pval->batch;
      memcpy((char *)fm_frame_get_ptr1(result, decoration_field_, 0),
             pval->decoration, sizeof(char) * 4);
      *(uint8_t *)fm_frame_get_ptr1(result, sale_condition_field_, 0) =
          pval->decoration[4];
      *(uint8_t *)fm_frame_get_ptr1(result, sale_condition2_field_, 0) =
          pval->decoration[5];
      *(uint8_t *)fm_frame_get_ptr1(result, sale_condition3_field_, 0) =
          pval->decoration[6];
      *(uint8_t *)fm_frame_get_ptr1(result, sale_condition4_field_, 0) =
          pval->decoration[7];
      return true;
    }
    return false;
  }
  fm_field_t vendor_field_, seqn_field_, trade_price_field_, qty_field_,
      batch_field_, decoration_field_, sale_condition_field_,
      sale_condition2_field_, sale_condition3_field_, sale_condition4_field_;
};

class state_event_op : public book_event_op {
public:
  state_event_op(fm_type_sys_t *sys) {
    const int nf = 7;
    int dims[1] = {1};
    const char *names[nf] = {"vendor", "seqn",   "id",   "price",
                             "state",  "is_bid", "batch"};
    fm_type_decl_cp types[nf] = {
        fm_base_type_get(sys, FM_TYPE_TIME64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_UINT32),
        fm_base_type_get(sys, FM_TYPE_UINT16),
        fm_base_type_get(sys, FM_TYPE_UINT16),
    };
    type_ = fm_frame_type_get1(sys, nf, names, types, 1, dims);

    vendor_field_ = fm_type_frame_field_idx(type_, "vendor");
    seqn_field_ = fm_type_frame_field_idx(type_, "seqn");
    id_field_ = fm_type_frame_field_idx(type_, "id");
    price_field_ = fm_type_frame_field_idx(type_, "price");
    state_field_ = fm_type_frame_field_idx(type_, "state");
    is_bid_field_ = fm_type_frame_field_idx(type_, "is_bid");
    batch_field_ = fm_type_frame_field_idx(type_, "batch");
  }
  void init(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fmc_time64_start();
    *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = 0UL;
    *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = 0UL;
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0), 0);
    *(uint32_t *)fm_frame_get_ptr1(result, state_field_, 0) = 0;
    *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = 0;
    *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = 0;
  }
  bool exec(const book::message &msg, fm_frame_t *result) override {
    if (auto pval = std::get_if<book::updates::state>(&msg)) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
          pval->vendor;
      *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = pval->seqn;
      *(uint64_t *)fm_frame_get_ptr1(result, id_field_, 0) = pval->id;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0) =
          pval->price;
      *(uint32_t *)fm_frame_get_ptr1(result, state_field_, 0) = pval->state;
      *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = pval->is_bid;
      *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = pval->batch;
      return true;
    }
    return false;
  }
  fm_field_t vendor_field_, seqn_field_, id_field_, price_field_, state_field_,
      is_bid_field_, batch_field_;
};

class control_event_op : public book_event_op {
public:
  control_event_op(fm_type_sys_t *sys) {
    const int nf = 5;
    int dims[1] = {1};
    const char *names[nf] = {"vendor", "seqn", "batch", "uncross", "command"};
    fm_type_decl_cp types[nf] = {fm_base_type_get(sys, FM_TYPE_TIME64),
                                 fm_base_type_get(sys, FM_TYPE_UINT64),
                                 fm_base_type_get(sys, FM_TYPE_UINT16),
                                 fm_base_type_get(sys, FM_TYPE_UINT8),
                                 fm_base_type_get(sys, FM_TYPE_CHAR)};
    type_ = fm_frame_type_get1(sys, nf, names, types, 1, dims);
    vendor_field_ = fm_type_frame_field_idx(type_, "vendor");
    seqn_field_ = fm_type_frame_field_idx(type_, "seqn");
    batch_field_ = fm_type_frame_field_idx(type_, "batch");
    uncross_field_ = fm_type_frame_field_idx(type_, "uncross");
    command_field_ = fm_type_frame_field_idx(type_, "command");
  }
  void init(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fmc_time64_start();
    *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = 0UL;
    *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = 0;
    *(uint8_t *)fm_frame_get_ptr1(result, uncross_field_, 0) = 0;
    *(char *)fm_frame_get_ptr1(result, command_field_, 0) = 0;
  }
  bool exec(const book::message &msg, fm_frame_t *result) override {
    if (auto pval = std::get_if<book::updates::control>(&msg)) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
          pval->vendor;
      *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = pval->seqn;
      *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = pval->batch;
      *(uint8_t *)fm_frame_get_ptr1(result, uncross_field_, 0) = pval->uncross;
      *(char *)fm_frame_get_ptr1(result, command_field_, 0) = pval->command;
      return true;
    }
    return false;
  }
  fm_field_t vendor_field_, seqn_field_, batch_field_, uncross_field_,
      command_field_;
};

class set_event_op : public book_event_op {
public:
  set_event_op(fm_type_sys_t *sys) {
    const int nf = 6;
    int dims[1] = {1};
    const char *names[nf] = {"vendor", "seqn",   "price",
                             "qty",    "is_bid", "batch"};
    fm_type_decl_cp types[nf] = {
        fm_base_type_get(sys, FM_TYPE_TIME64),
        fm_base_type_get(sys, FM_TYPE_UINT64),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_FIXEDPOINT128),
        fm_base_type_get(sys, FM_TYPE_UINT16),
        fm_base_type_get(sys, FM_TYPE_UINT16),
    };
    type_ = fm_frame_type_get1(sys, nf, names, types, 1, dims);
    vendor_field_ = fm_type_frame_field_idx(type_, "vendor");
    seqn_field_ = fm_type_frame_field_idx(type_, "seqn");
    price_field_ = fm_type_frame_field_idx(type_, "price");
    qty_field_ = fm_type_frame_field_idx(type_, "qty");
    is_bid_field_ = fm_type_frame_field_idx(type_, "is_bid");
    batch_field_ = fm_type_frame_field_idx(type_, "batch");
  }
  void init(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fmc_time64_start();
    *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = 0UL;
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0), 0);
    fmc_fxpt128_from_int(
        (fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0), 0);
    *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = 0;
    *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = 0;
  }
  bool exec(const book::message &msg, fm_frame_t *result) override {
    if (auto pval = std::get_if<book::updates::set>(&msg)) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
          pval->vendor;
      *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = pval->seqn;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, price_field_, 0) =
          pval->price;
      *(fmc_fxpt128_t *)fm_frame_get_ptr1(result, qty_field_, 0) = pval->qty;
      *(uint16_t *)fm_frame_get_ptr1(result, is_bid_field_, 0) = pval->is_bid;
      *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = pval->batch;
      return true;
    }
    return false;
  }
  fm_field_t vendor_field_, seqn_field_, price_field_, qty_field_,
      is_bid_field_, batch_field_;
};

class time_event_op : public book_event_op {
public:
  time_event_op(fm_type_sys_t *sys) {
    const int nf = 1;
    int dims[1] = {1};
    const char *names[nf] = {"seconds"};
    fm_type_decl_cp types[nf] = {fm_base_type_get(sys, FM_TYPE_TIME64)};
    type_ = fm_frame_type_get1(sys, nf, names, types, 1, dims);
    seconds_field_ = fm_type_frame_field_idx(type_, "seconds");
  }
  void init(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, seconds_field_, 0) =
        fmc_time64_start();
  }
  bool exec(const book::message &msg, fm_frame_t *result) override {
    if (auto pval = std::get_if<book::updates::time>(&msg)) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, seconds_field_, 0) =
          pval->seconds;
      return true;
    }
    return false;
  }
  fm_field_t seconds_field_;
};

bool fm_comp_book_msg_call_stream_init(fm_frame_t *result, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx,
                                       fm_call_exec_cl *cl) {
  auto &comp = (*(book_event_op *)ctx->comp);
  comp.init(result);
  return true;
}

bool fm_comp_book_msg_stream_exec(fm_frame_t *result, size_t args,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &box = *(book::message *)fm_frame_get_cptr1(argv[0], 0, 0);
  if (std::holds_alternative<book::updates::none>(box))
    return false;
  auto &comp = (*(book_event_op *)ctx->comp);
  return comp.exec(box, result);
}

fm_call_def *fm_comp_book_msg_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_book_msg_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_book_msg_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_book_msg_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
                                   fm_type_decl_cp ptype,
                                   fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  auto rec_t =
      fm_record_type_get(sys, "fm::book::message", sizeof(book::message));
  auto in_type = fm_frame_type_get(sys, 1, 1, "update", rec_t, 1);
  if (!in_type) {
    return nullptr;
  }

  if (argc != 1 || !fm_type_equal(argv[0], in_type)) {
    auto *errstr = "expect book updates as input";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    auto *errstr = "expect message name as an argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto *name = fm_arg_try_cstring(fm_type_tuple_arg(ptype, 0), &plist);
  string str_name(name);

  book_event_op *cl = nullptr;

  if (str_name == "add") {
    cl = new add_event_op(sys);
  } else if (str_name == "insert") {
    cl = new insert_event_op(sys);
  } else if (str_name == "position") {
    cl = new position_event_op(sys);
  } else if (str_name == "cancel") {
    cl = new cancel_event_op(sys);
  } else if (str_name == "execute") {
    cl = new execute_event_op(sys);
  } else if (str_name == "trade") {
    cl = new trade_event_op(sys);
  } else if (str_name == "state") {
    cl = new state_event_op(sys);
  } else if (str_name == "control") {
    cl = new control_event_op(sys);
  } else if (str_name == "set") {
    cl = new set_event_op(sys);
  } else if (str_name == "time") {
    cl = new time_event_op(sys);
  }

  if (!cl) {
    auto *errstr = "message name provided not available";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  fm_type_decl_cp type = cl->type();

  if (!type) {
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, (void *)cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_book_msg_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_book_msg_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (book_event_op *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
