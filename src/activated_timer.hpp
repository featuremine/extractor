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
 * @file split_by.cpp
 * @author Andres Rangel
 * @date Feb 4 2019
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "fmc++/time.hpp"

struct act_timer_closure {
  bool updated = 0;
  bool activated = 0;
  fm_field_t scheduled_id;
  fm_field_t actual_id;
  fmc_time64_t period_;
  fmc_time64_t scheduled;
};

bool fm_comp_activated_timer_stream_init(fm_frame_t *result, size_t args,
                                         const fm_frame_t *const argv[],
                                         fm_call_ctx_t *ctx,
                                         fm_call_exec_cl *cl) {
  auto *comp_cl = (act_timer_closure *)ctx->comp;
  comp_cl->scheduled = fmc_time64_end();
  fm_stream_ctx_queue((fm_stream_ctx_t *)ctx->exec, ctx->handle);
  return true;
}

bool fm_comp_activated_timer_stream_exec(fm_frame_t *result, size_t args,
                                         const fm_frame_t *const argv[],
                                         fm_call_ctx_t *ctx,
                                         fm_call_exec_cl cl) {
  auto *comp_cl = (act_timer_closure *)ctx->comp;
  auto updated = comp_cl->updated;
  auto *s_ctx = (fm_stream_ctx_t *)ctx->exec;
  comp_cl->updated = 0;

  auto now = fm_stream_ctx_now(s_ctx);
  bool start = fmc_time64_is_end(comp_cl->scheduled);

  if (!start && (updated || comp_cl->scheduled != now)) {
    return false;
  }

  auto needed = comp_cl->period_ * (now / comp_cl->period_);
  auto next = needed + comp_cl->period_;
  bool done = !start;
  if (start && needed == now) {
    done = true;
    comp_cl->scheduled = needed;
  }
  auto prev = comp_cl->scheduled;
  comp_cl->scheduled = next;
  fm_stream_ctx_schedule(s_ctx, ctx->handle, comp_cl->scheduled);

  if (comp_cl->activated) {
    if (done) {
      *(fmc_time64_t *)fm_frame_get_ptr1(result, comp_cl->scheduled_id, 0) =
          prev;
      *(fmc_time64_t *)fm_frame_get_ptr1(result, comp_cl->actual_id, 0) = now;
    }
    return done;
  }
  return false;
}

fm_call_def *fm_comp_activated_timer_stream_call(fm_comp_def_cl comp_cl,
                                                 const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_activated_timer_stream_init);
  fm_call_def_exec_set(def, fm_comp_activated_timer_stream_exec);
  return def;
}

void fm_comp_activated_timer_queuer(size_t idx, fm_call_ctx_t *ctx) {
  auto *comp_cl = (act_timer_closure *)ctx->comp;
  comp_cl->updated = 1;
  if (!comp_cl->activated) {
    comp_cl->activated = true;
  }
}

fm_ctx_def_t *fm_comp_activated_timer_gen(fm_comp_sys_t *csys,
                                          fm_comp_def_cl closure, unsigned argc,
                                          fm_type_decl_cp argv[],
                                          fm_type_decl_cp ptype,
                                          fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  if (!argc) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS,
                           "expect at least one "
                           "operator argument");
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect a period "
                           "time parameter");
    return nullptr;
  }

  fmc_time64_t period{0};
  if (!fm_arg_try_time64(fm_type_tuple_arg(ptype, 0), &plist, &period)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect first parameter to be a lag time");
    return nullptr;
  }

  const char *names[2] = {"scheduled", "actual"};
  fm_type_decl_cp types[2] = {fm_base_type_get(sys, FM_TYPE_TIME64),
                              fm_base_type_get(sys, FM_TYPE_TIME64)};
  int dims[1] = {1};

  int fm_type_frame_field_idx(fm_type_decl_cp td, const char *);

  fm_type_decl_cp ret_type = fm_frame_type_get1(sys, 2, names, types, 1, dims);
  if (!ret_type) {
    auto *errstr = "unable to create result frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto cl = new act_timer_closure();

  cl->scheduled_id = fm_type_frame_field_idx(ret_type, "scheduled");
  cl->actual_id = fm_type_frame_field_idx(ret_type, "actual");

  cl->period_ = period;

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, ret_type);
  fm_ctx_def_closure_set(def, cl);
  fm_ctx_def_queuer_set(def, &fm_comp_activated_timer_queuer);
  fm_ctx_def_stream_call_set(def, &fm_comp_activated_timer_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_activated_timer_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  if (auto *ctx_cl = (act_timer_closure *)fm_ctx_def_closure(def)) {
    delete ctx_cl;
  }
}

bool fm_comp_activated_timer_add(fm_comp_sys_t *sys) {
  fm_comp_def_t def = {"activated_timer", &fm_comp_activated_timer_gen,
                       &fm_comp_activated_timer_destroy, NULL};

  return fm_comp_type_add(sys, &def);
}
