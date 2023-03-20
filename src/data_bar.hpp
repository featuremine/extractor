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
 * @file data_bar.cpp
 * @author Maxim Trokhimtchouk
 * @date Nov 7 2022
 * @brief File contains C++ definitions of the data timer
 *
 * Data timer
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
}

struct data_bar_closure {
  fm_field_t start_id;
  fm_field_t skipped_id;
  fmc_time64_t period_;
  fmc_time64_t offset_;
  fmc_time64_t last_;
};

bool fm_comp_data_bar_stream_init(fm_frame_t *result, size_t args,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto *comp_cl = (data_bar_closure *)ctx->comp;
  comp_cl->last_ = fmc_time64_start();
  return true;
}

bool fm_comp_data_bar_stream_exec(fm_frame_t *result, size_t args,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *comp_cl = (data_bar_closure *)ctx->comp;

  fmc_time64_t now = *(const fmc_time64_t *)fm_frame_get_cptr1(argv[0], 0, 0);
  fmc_time64_t start =
      comp_cl->period_ * ((now - comp_cl->offset_) / comp_cl->period_) +
      comp_cl->offset_;
  fmc_time64_t last = comp_cl->last_;
  comp_cl->last_ = now;

  if (last < start) {
    fmc_time64_t close =
        comp_cl->period_ * ((last - comp_cl->offset_) / comp_cl->period_ + 1) +
        comp_cl->offset_;
    *(fmc_time64_t *)fm_frame_get_ptr1(result, comp_cl->start_id, 0) = close;
    bool skipped = (last + comp_cl->period_) < close;
    *(bool *)fm_frame_get_ptr1(result, comp_cl->skipped_id, 0) = skipped;
    return true;
  }
  return false;
}

fm_call_def *fm_comp_data_bar_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_data_bar_stream_init);
  fm_call_def_exec_set(def, fm_comp_data_bar_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_data_bar_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
                                   fm_type_decl_cp ptype,
                                   fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  if (argc != 1 || fm_type_frame_nfields(argv[0]) != 1 ||
      fm_type_base_enum(fm_type_frame_field_type(argv[0], 0)) !=
          FM_TYPE_TIME64) {
    fm_type_sys_err_custom(
        sys, FM_TYPE_ERROR_ARGS,
        "expect exactly one operator with a single time field");
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype) ||
      !(fm_type_tuple_size(ptype) == 1 || fm_type_tuple_size(ptype) == 2)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect a period and an optional offset"
                           " time parameter");
    return nullptr;
  }

  fmc_time64_t period{0};
  if (!fm_arg_try_time64(fm_type_tuple_arg(ptype, 0), &plist, &period)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect first parameter to be a period time");
    return nullptr;
  }

  fmc_time64_t offset{0};
  if (fm_type_tuple_size(ptype) == 2) {
    if (!fm_arg_try_time64(fm_type_tuple_arg(ptype, 1), &plist, &offset)) {
      fm_type_sys_err_custom(
          sys, FM_TYPE_ERROR_PARAMS,
          "expect optional second parameter to be an offset time");
      return nullptr;
    }
  }

  const char *names[2] = {"start", "skipped"};
  fm_type_decl_cp types[2] = {fm_base_type_get(sys, FM_TYPE_TIME64),
                              fm_base_type_get(sys, FM_TYPE_BOOL)};
  int dims[1] = {1};

  int fm_type_frame_field_idx(fm_type_decl_cp td, const char *);

  fm_type_decl_cp ret_type = fm_frame_type_get1(sys, 2, names, types, 1, dims);
  if (!ret_type) {
    auto *errstr = "unable to create result frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto cl = new data_bar_closure();

  cl->start_id = fm_type_frame_field_idx(ret_type, "start");
  cl->skipped_id = fm_type_frame_field_idx(ret_type, "skipped");

  cl->period_ = period;
  cl->offset_ = offset;

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, ret_type);
  fm_ctx_def_closure_set(def, cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_data_bar_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_data_bar_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  if (auto *ctx_cl = (data_bar_closure *)fm_ctx_def_closure(def)) {
    delete ctx_cl;
  }
}

bool fm_comp_data_bar_add(fm_comp_sys_t *sys) {
  fm_comp_def_t def = {"data_bar", &fm_comp_data_bar_gen,
                       &fm_comp_data_bar_destroy, NULL};

  return fm_comp_type_add(sys, &def);
}
