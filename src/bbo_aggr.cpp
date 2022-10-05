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
#include "extractor/decimal64.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include "extractor/rprice.hpp"
#include "extractor/side.hpp"

#include <utility>
#include <vector>

using namespace fm;
using namespace std;

struct bbo_aggr_exec_cl {
  sided<fm_field_t> pxs;
  sided<fm_field_t> qts;
  fm_field_t rec;
  sided<fm_field_t> out_pxs;
  sided<fm_field_t> out_qts;
};

bool fm_comp_bbo_aggr_call_stream_init(fm_frame_t *result, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx,
                                       fm_call_exec_cl *cl) {
  using namespace std;
  auto *exec_cl = new bbo_aggr_exec_cl();

  exec_cl->pxs[trade_side::BID] = fm_frame_field(argv[0], "bidprice");
  exec_cl->pxs[trade_side::ASK] = fm_frame_field(argv[0], "askprice");
  exec_cl->qts[trade_side::BID] = fm_frame_field(argv[0], "bidqty");
  exec_cl->qts[trade_side::ASK] = fm_frame_field(argv[0], "askqty");

  exec_cl->rec = fm_frame_field(result, "receive");

  exec_cl->out_pxs[trade_side::BID] = fm_frame_field(result, "bidprice");
  exec_cl->out_pxs[trade_side::ASK] = fm_frame_field(result, "askprice");
  exec_cl->out_qts[trade_side::BID] = fm_frame_field(result, "bidqty");
  exec_cl->out_qts[trade_side::ASK] = fm_frame_field(result, "askqty");

  *(fm_decimal64_t *)fm_frame_get_ptr1(
      result, exec_cl->out_pxs[trade_side::BID], 0) = FM_DECIMAL64_MIN;
  *(fm_decimal64_t *)fm_frame_get_ptr1(
      result, exec_cl->out_pxs[trade_side::ASK], 0) = FM_DECIMAL64_MAX;

  *cl = exec_cl;
  return true;
}

void fm_comp_bbo_aggr_call_stream_destroy(fm_call_exec_cl cl) {
  auto *exec_cl = (bbo_aggr_exec_cl *)cl;
  if (exec_cl)
    delete exec_cl;
}

bool fm_comp_bbo_aggr_stream_exec(fm_frame_t *result, size_t argc,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *exec_cl = (bbo_aggr_exec_cl *)cl;
  auto now = fm_stream_ctx_now((fm_stream_ctx_t *)ctx->exec);
  *(fm_time64_t *)fm_frame_get_ptr1(result, exec_cl->rec, 0) = now;
  for (auto side : trade_side::all()) {
    better<rprice> cmp(side);
    auto best_px = sided<rprice>()[side];
    auto px_idx = exec_cl->pxs[side];
    auto qt_idx = exec_cl->qts[side];
    for (size_t i = 0; i < argc; ++i) {
      auto qt = *(int32_t *)fm_frame_get_cptr1(argv[i], qt_idx, 0);
      auto px = *(fm_decimal64_t *)fm_frame_get_cptr1(argv[i], px_idx, 0);
      if ((qt != 0) && cmp(px, best_px))
        best_px = px;
    }
    int32_t qt_tot = 0;
    for (size_t i = 0; i < argc; ++i) {
      auto px = *(fm_decimal64_t *)fm_frame_get_cptr1(argv[i], px_idx, 0);
      if (best_px == px)
        qt_tot += *(int32_t *)fm_frame_get_cptr1(argv[i], qt_idx, 0);
    }
    *(fm_decimal64_t *)fm_frame_get_ptr1(result, exec_cl->out_pxs[side], 0) =
        best_px;
    *(int32_t *)fm_frame_get_ptr1(result, exec_cl->out_qts[side], 0) = qt_tot;
  }

  return true;
}

fm_call_def *fm_comp_bbo_aggr_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_bbo_aggr_call_stream_init);
  fm_call_def_destroy_set(def, fm_comp_bbo_aggr_call_stream_destroy);
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

  auto *type = fm_frame_type_get(
      sys, 5, 1, "receive", fm_base_type_get(sys, FM_TYPE_TIME64), "bidprice",
      fm_base_type_get(sys, FM_TYPE_DECIMAL64), "askprice",
      fm_base_type_get(sys, FM_TYPE_DECIMAL64), "bidqty",
      fm_base_type_get(sys, FM_TYPE_INT32), "askqty",
      fm_base_type_get(sys, FM_TYPE_INT32), 1);

  fm_type_decl_cp input = nullptr;
  for (size_t i = 0; i < argc; ++i) {
    if (i == 0) {
      input = argv[i];
    }
    if (!fm_type_is_subframe(type, argv[i])) {
      auto *type_str1 = fm_type_to_str(type);
      auto *type_str2 = fm_type_to_str(argv[i]);
      auto errstr = string("the inputs must contain BBO frame\n");
      errstr += type_str1;
      errstr.append("\ninstead got\n");
      errstr += type_str2;
      free(type_str1);
      free(type_str2);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr.c_str());
      return nullptr;
    }
    if (!fm_type_equal(input, argv[i])) {
      auto *type_str1 = fm_type_to_str(input);
      auto *type_str2 = fm_type_to_str(argv[i]);
      auto errstr =
          string("the inputs must be of the same type, instead got \n");
      errstr += type_str1;
      errstr.append("\nand\n");
      errstr += type_str2;
      free(type_str1);
      free(type_str2);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr.c_str());
      return nullptr;
    }
  }

  if (!fm_args_empty(ptype)) {
    auto *errstr = "expect no parameters";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_stream_call_set(def, &fm_comp_bbo_aggr_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_bbo_aggr_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {}
