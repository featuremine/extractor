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
 * @file bbo_book_aggr.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definitions of the bbo aggregation computation
 *
 * This file contains definitions of the bbo aggregation computation
 * @see http://www.featuremine.com
 */

#include "bbo_aggr.h"
#include "extractor/arg_stack.h"
#include "extractor/book/book.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/decimal128.h"
#include "fmc/time.h"

#include "fmc++/decimal128.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/side.hpp"

#include <utility>
#include <vector>

using namespace fmc;
using namespace std;

struct bbo_book_aggr_exec_cl {
  virtual void init(size_t argc, const fm_frame_t *const argv[],
                    fm_frame_t *result) = 0;
  virtual void update_book(fm_stream_ctx_t *ctx, size_t idx) = 0;
  virtual void nbbo_to_frame(fm_stream_ctx_t *ctx, fm_frame_t *result) = 0;
  virtual ~bbo_book_aggr_exec_cl(){};
};

template <typename Price, typename Quantity>
struct bbo_book_aggr_exec_cl_impl : bbo_book_aggr_exec_cl {

  bbo_book_aggr_exec_cl_impl(fm_book_shared_t *book, unsigned argc)
      : book_(book),
        data_(argc, {make_pair(sided<Price>()[trade_side::BID], 0),
                     make_pair(sided<Price>()[trade_side::ASK], 0)}),
        zero_(0) {
    fm_book_shared_inc(book_);
  }

  ~bbo_book_aggr_exec_cl_impl() { fm_book_shared_dec(book_); }

  void init(size_t argc, const fm_frame_t *const argv[],
            fm_frame_t *result) override {
    inps_ = vector<const fm_frame_t *>(
        argv, argv + (argc * sizeof(const fm_frame_t *)));

    pxs_[trade_side::BID] = fm_frame_field(argv[0], "bidprice");
    pxs_[trade_side::ASK] = fm_frame_field(argv[0], "askprice");
    qts_[trade_side::BID] = fm_frame_field(argv[0], "bidqty");
    qts_[trade_side::ASK] = fm_frame_field(argv[0], "askqty");
    recv_[trade_side::BID] = fm_frame_field(argv[0], "receive");
    recv_[trade_side::ASK] = fm_frame_field(argv[0], "receive");

    rec_ = fm_frame_field(result, "receive");

    out_pxs_[trade_side::BID] = fm_frame_field(result, "bidprice");
    out_pxs_[trade_side::ASK] = fm_frame_field(result, "askprice");
    out_qts_[trade_side::BID] = fm_frame_field(result, "bidqty");
    out_qts_[trade_side::ASK] = fm_frame_field(result, "askqty");
  }

  void update_book(fm_stream_ctx_t *ctx, size_t idx) override {
    auto now = fm_stream_ctx_now(ctx);
    auto &idx_data = data_[idx];
    auto *book = fm_book_shared_get(book_);
    for (auto side : trade_side::all()) {
      auto recv_idx = recv_[side];
      auto qts_idx = qts_[side];
      auto pxs_idx = pxs_[side];

      auto frame = inps_[idx];
      auto &sided_data = idx_data[side];
      auto &oldpx = sided_data.first;
      auto &oldqty = sided_data.second;
      auto isbid = is_bid(side);
      if (oldqty != zero_) {
        if constexpr (is_same_v<Price, fmc::decimal128> &&
                      is_same_v<Quantity, fmc::decimal128>) {
          fm_book_mod(book, idx, oldpx, oldqty, isbid);
        } else {
          fm_book_mod(book, idx, fmc::decimal128(oldpx),
                      fmc::decimal128(oldqty), isbid);
        }
      }

      auto px = *(Price *)fm_frame_get_cptr1(frame, pxs_idx, 0);
      auto qty = *(Quantity *)fm_frame_get_cptr1(frame, qts_idx, 0);
      if (qty != zero_) {
        auto ven = *(fmc_time64_t *)fm_frame_get_cptr1(frame, recv_idx, 0);
        if constexpr (is_same_v<Price, fmc::decimal128> &&
                      is_same_v<Quantity, fmc::decimal128>) {
          fm_book_add(book, now, ven, 0, idx, px, qty, isbid);
        } else {
          fm_book_add(book, now, ven, 0, idx, decimal128(px), decimal128(qty),
                      isbid);
        }
      }

      oldpx = px;
      oldqty = qty;
    }
  }

  void nbbo_to_frame(fm_stream_ctx_t *ctx, fm_frame_t *result) override {
    auto now = fm_stream_ctx_now(ctx);

    auto *book = fm_book_shared_get(book_);
    for (auto side : trade_side::all()) {
      fm_levels_t *lvls = fm_book_levels(book, is_bid(side));

      Quantity qty(0);
      Price px = sided<Price>()[side];

      if (fm_book_levels_size(lvls) != 0) {
        fm_level_t *lvl = fm_book_level(lvls, 0);
        qty = (Quantity)fmc::decimal128::upcast(fm_book_level_shr(lvl));
        px = (Price)fmc::decimal128::upcast(fm_book_level_prx(lvl));
      }

      *(fmc_time64_t *)fm_frame_get_ptr1(result, rec_, 0) = now;
      *(Price *)fm_frame_get_ptr1(result, out_pxs_[side], 0) = px;
      *(Quantity *)fm_frame_get_ptr1(result, out_qts_[side], 0) = qty;
    }
  }

  fm_book_shared_t *book_;
  fm_field_t rec_;
  sided<fm_field_t> recv_;
  sided<fm_field_t> pxs_;
  sided<fm_field_t> qts_;
  sided<fm_field_t> out_pxs_;
  sided<fm_field_t> out_qts_;
  vector<sided<pair<Price, Quantity>>> data_;
  vector<const fm_frame_t *> inps_;
  Quantity zero_;
};

bool fm_comp_bbo_book_aggr_call_stream_init(fm_frame_t *result, size_t args,
                                            const fm_frame_t *const argv[],
                                            fm_call_ctx_t *ctx,
                                            fm_call_exec_cl *cl) {
  auto *ctx_cl = (bbo_book_aggr_exec_cl *)ctx->comp;
  ctx_cl->init(args, argv, result);
  ctx_cl->nbbo_to_frame((fm_stream_ctx_t *)ctx->exec, result);
  return true;
}

bool fm_comp_bbo_book_aggr_stream_exec(fm_frame_t *result, size_t argc,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *ctx_cl = (bbo_book_aggr_exec_cl *)ctx->comp;
  ctx_cl->nbbo_to_frame((fm_stream_ctx_t *)ctx->exec, result);
  return true;
}

void fm_comp_bbo_book_aggr_queuer(size_t idx, fm_call_ctx_t *ctx) {
  auto *cl = (bbo_book_aggr_exec_cl *)ctx->comp;
  cl->update_book((fm_stream_ctx_t *)ctx->exec, idx);
}

fm_call_def *fm_comp_bbo_book_aggr_stream_call(fm_comp_def_cl comp_cl,
                                               const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_bbo_book_aggr_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_bbo_book_aggr_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_bbo_book_aggr_gen(fm_comp_sys_t *csys,
                                        fm_comp_def_cl closure, unsigned argc,
                                        fm_type_decl_cp argv[],
                                        fm_type_decl_cp ptype,
                                        fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc == 0) {
    auto *errstr = "expect at least one operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto *compatibility_type = fm_frame_type_get(
      sys, 5, 1, "receive", fm_base_type_get(sys, FM_TYPE_TIME64), "bidprice",
      fm_base_type_get(sys, FM_TYPE_RPRICE), "askprice",
      fm_base_type_get(sys, FM_TYPE_RPRICE), "bidqty",
      fm_base_type_get(sys, FM_TYPE_INT32), "askqty",
      fm_base_type_get(sys, FM_TYPE_INT32), 1);

  auto *type = fm_frame_type_get(
      sys, 5, 1, "receive", fm_base_type_get(sys, FM_TYPE_TIME64), "bidprice",
      fm_base_type_get(sys, FM_TYPE_DECIMAL128), "askprice",
      fm_base_type_get(sys, FM_TYPE_DECIMAL128), "bidqty",
      fm_base_type_get(sys, FM_TYPE_DECIMAL128), "askqty",
      fm_base_type_get(sys, FM_TYPE_DECIMAL128), 1);

  fm_type_decl_cp input = argv[0];

  auto validate_type = [&sys, &input](auto type, auto argv) {
    if (!fm_type_is_subframe(type, argv)) {
      auto *type_str1 = fm_type_to_str(type);
      auto *type_str2 = fm_type_to_str(argv);
      auto errstr = string("the inputs must contain BBO frame\n");
      errstr += type_str1;
      errstr.append("\ninstead got\n");
      errstr += type_str2;
      free(type_str1);
      free(type_str2);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr.c_str());
      return false;
    }
    if (!fm_type_equal(input, argv)) {
      auto *type_str1 = fm_type_to_str(input);
      auto *type_str2 = fm_type_to_str(argv);
      auto errstr =
          string("the inputs must be of the same type, instead got \n");
      errstr += type_str1;
      errstr.append("\nand\n");
      errstr += type_str2;
      free(type_str1);
      free(type_str2);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr.c_str());
      return false;
    }
    return true;
  };

  fm_type_decl_cp used_type = nullptr;

  if (validate_type(compatibility_type, input)) {
    used_type = compatibility_type;
  } else {
    fm_type_sys_err_set(sys, FM_TYPE_ERROR_OK);
    if (validate_type(type, input)) {
      used_type = type;
    } else {
      auto *type_str0 = fm_type_to_str(compatibility_type);
      auto *type_str1 = fm_type_to_str(type);
      auto *type_str2 = fm_type_to_str(input);
      auto errstr = string("the inputs must contain BBO frame\n");
      errstr += type_str0;
      errstr.append("\nor\n");
      errstr += type_str1;
      errstr.append("\ninstead got\n");
      errstr += type_str2;
      free(type_str0);
      free(type_str1);
      free(type_str2);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr.c_str());
      return nullptr;
    }
  }

  for (size_t i = 1; i < argc; ++i) {
    if (!validate_type(used_type, argv[i])) {
      return nullptr;
    }
  }

  auto param_error = [&]() {
    auto *errstr = "expect a python book object as argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!ptype || fm_args_empty(ptype)) {
    return param_error();
  }

  if (!fm_type_is_tuple(ptype)) {
    return param_error();
  }

  if (fm_type_tuple_size(ptype) != 1) {
    return param_error();
  }

  auto book_rec_t =
      fm_record_type_get(sys, "fm_book_shared_t*", sizeof(fm_book_shared_t *));

  fm_book_shared_t *book = nullptr;

  auto param = fm_type_tuple_arg(ptype, 0);
  if (fm_type_is_record(param)) {
    if (!fm_type_equal(book_rec_t, param)) {
      return param_error();
    }
  }

  book = STACK_POP(plist, fm_book_shared_t *);

  bbo_book_aggr_exec_cl *cl;
  if (fm_type_equal(used_type, compatibility_type)) {
    cl = new bbo_book_aggr_exec_cl_impl<fmc::rprice, int32_t>(book, argc);
  } else {
    cl = new bbo_book_aggr_exec_cl_impl<fmc::decimal128, fmc::decimal128>(book,
                                                                          argc);
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, used_type);
  fm_ctx_def_closure_set(def, cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_bbo_book_aggr_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  fm_ctx_def_queuer_set(def, &fm_comp_bbo_book_aggr_queuer);
  return def;
}

void fm_comp_bbo_book_aggr_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (bbo_book_aggr_exec_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
