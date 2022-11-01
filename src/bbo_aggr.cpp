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
 * @file bbo_aggr.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definitions of the bbo aggregation computation
 *
 * This file contains definitions of the bbo aggregation computation
 * @see http://www.featuremine.com
 */

extern "C" {
#include "bbo_aggr.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/decimal128.h"
#include "fmc/time.h"
}

#include "fmc++/decimal64.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/side.hpp"

#include <utility>
#include <vector>

using namespace fmc;
using namespace std;

struct bbo_aggr_exec_cl {
  virtual void init(fm_frame_t *result, const fm_frame_t *const argv[]) = 0;
  virtual void exec(fm_frame_t *result, size_t argc, const fm_frame_t *const argv[], fmc_time64_t now) = 0;
  virtual ~bbo_aggr_exec_cl() {}
};

template<typename Price, typename Quantity>
struct bbo_aggr_exec_cl_impl: bbo_aggr_exec_cl {

  template<typename Dec = Quantity>
  bbo_aggr_exec_cl_impl(typename std::enable_if<std::is_same_v<Dec, fmc::decimal128>>::type* = nullptr)
      : zero_(Quantity()) {
  }

  template<typename Dec = Quantity>
  bbo_aggr_exec_cl_impl(typename std::enable_if<!std::is_same_v<Dec, fmc::decimal128>>::type* = nullptr)
      : zero_(0) {
  }

  void init(fm_frame_t *result, const fm_frame_t *const argv[]) override {
    pxs[trade_side::BID] = fm_frame_field(argv[0], "bidprice");
    pxs[trade_side::ASK] = fm_frame_field(argv[0], "askprice");
    qts[trade_side::BID] = fm_frame_field(argv[0], "bidqty");
    qts[trade_side::ASK] = fm_frame_field(argv[0], "askqty");

    rec = fm_frame_field(result, "receive");

    out_pxs[trade_side::BID] = fm_frame_field(result, "bidprice");
    out_pxs[trade_side::ASK] = fm_frame_field(result, "askprice");
    out_qts[trade_side::BID] = fm_frame_field(result, "bidqty");
    out_qts[trade_side::ASK] = fm_frame_field(result, "askqty");

    *(Price *)fm_frame_get_ptr1(result, out_pxs[trade_side::BID], 0) =
        sided<Price>()[trade_side::BID];
    *(Price *)fm_frame_get_ptr1(result, out_pxs[trade_side::ASK], 0) =
        sided<Price>()[trade_side::ASK];
  }

  void exec(fm_frame_t *result, size_t argc, const fm_frame_t *const argv[], fmc_time64_t now) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, rec, 0) = now;
    for (auto side : trade_side::all()) {
      better<Price> cmp(side);
      auto best_px = sided<Price>()[side];
      auto px_idx = pxs[side];
      auto qt_idx = qts[side];
      for (size_t i = 0; i < argc; ++i) {
        auto qt = *(Quantity *)fm_frame_get_cptr1(argv[i], qt_idx, 0);
        auto px = *(Price *)fm_frame_get_cptr1(argv[i], px_idx, 0);
        if ((qt != zero_) && cmp(px, best_px)) {
          best_px = px;
        }
      }
      Quantity qt_tot = zero_;
      for (size_t i = 0; i < argc; ++i) {
        auto px = *(Price *)fm_frame_get_cptr1(argv[i], px_idx, 0);
        if (best_px == px)
          qt_tot += *(Quantity *)fm_frame_get_cptr1(argv[i], qt_idx, 0);
      }
      *(Price *)fm_frame_get_ptr1(result, out_pxs[side], 0) =
          best_px;
      *(Quantity *)fm_frame_get_ptr1(result, out_qts[side], 0) = qt_tot;
    }
  }

  sided<fm_field_t> pxs;
  sided<fm_field_t> qts;
  fm_field_t rec;
  sided<fm_field_t> out_pxs;
  sided<fm_field_t> out_qts;
  Quantity zero_;
};

bool fm_comp_bbo_aggr_call_stream_init(fm_frame_t *result, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx,
                                       fm_call_exec_cl *cl) {
  auto *ctx_cl = (bbo_aggr_exec_cl *)ctx->comp;
  ctx_cl->init(result, argv);
  return true;
}

bool fm_comp_bbo_aggr_stream_exec(fm_frame_t *result, size_t argc,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *ctx_cl = (bbo_aggr_exec_cl *)ctx->comp;
  auto now = fm_stream_ctx_now((fm_stream_ctx_t *)ctx->exec);
  ctx_cl->exec(result, argc, argv, now);
  return true;
}

fm_call_def *fm_comp_bbo_aggr_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_bbo_aggr_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_bbo_aggr_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_bbo_aggr_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
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
      fm_base_type_get(sys, FM_TYPE_DECIMAL64), "askprice",
      fm_base_type_get(sys, FM_TYPE_DECIMAL64), "bidqty",
      fm_base_type_get(sys, FM_TYPE_INT32), "askqty",
      fm_base_type_get(sys, FM_TYPE_INT32), 1);

  auto *type = fm_frame_type_get(
      sys, 5, 1, "receive", fm_base_type_get(sys, FM_TYPE_TIME64), "bidprice",
      fm_base_type_get(sys, FM_TYPE_DECIMAL128), "askprice",
      fm_base_type_get(sys, FM_TYPE_DECIMAL128), "bidqty",
      fm_base_type_get(sys, FM_TYPE_DECIMAL128), "askqty",
      fm_base_type_get(sys, FM_TYPE_DECIMAL128), 1);

  fm_type_decl_cp input = argv[0];

  auto validate_type = [&sys, &input](auto type, auto argv){
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

  if (!fm_args_empty(ptype)) {
    auto *errstr = "expect no parameters";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  bbo_aggr_exec_cl *cl;
  if (fm_type_equal(used_type, compatibility_type)) {
   cl = new bbo_aggr_exec_cl_impl<fm_decimal64_t, int32_t>();
  } else {
   cl = new bbo_aggr_exec_cl_impl<fmc::decimal128, fmc::decimal128>();
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, used_type);
  fm_ctx_def_closure_set(def, cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_bbo_aggr_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_bbo_aggr_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (bbo_aggr_exec_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
