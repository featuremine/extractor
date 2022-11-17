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
 * @file perf_timer.cpp
 * @author Andres Rangel
 * @date 16 Jan 2019
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "extractor/perf_timer.h"
#include "extractor/comp_sys.h"

#include "fmc++/counters.hpp"

using perf_timer = fmc::counter::rdtsc_avg;

bool fm_comp_perf_timer_start_call_stream_init(fm_frame_t *result, size_t args,
                                               const fm_frame_t *const argv[],
                                               fm_call_ctx_t *ctx,
                                               fm_call_exec_cl *cl) {
  return true;
}

bool fm_comp_perf_timer_start_stream_exec(fm_frame_t *result, size_t args,
                                          const fm_frame_t *const argv[],
                                          fm_call_ctx_t *ctx,
                                          fm_call_exec_cl cl) {
  auto *closure = (perf_timer *)ctx->comp;
  closure->start();
  return true;
}

fm_call_def *fm_comp_perf_timer_start_stream_call(fm_comp_def_cl comp_cl,
                                                  const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_perf_timer_start_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_perf_timer_start_stream_exec);
  return def;
}

fm_ctx_def_t *
fm_comp_perf_timer_start_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                             unsigned argc, fm_type_decl_cp argv[],
                             fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    auto *errstr = "expect a sample name as parameter";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto *name = fm_arg_try_cstring(fm_type_tuple_arg(ptype, 0), &plist);

  if (!name) {
    auto *errstr = "argument provided must be a cstring";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto &comp_cl = *(fmc::counter::samples *)closure;
  auto &cl = comp_cl.get<perf_timer>(name);
  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, true);
  fm_ctx_def_type_set(def, argv[0]);
  fm_ctx_def_closure_set(def, &cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_perf_timer_start_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_perf_timer_start_destroy(fm_comp_def_cl, fm_ctx_def_t *) {}

bool fm_comp_perf_timer_stop_call_stream_init(fm_frame_t *result, size_t args,
                                              const fm_frame_t *const argv[],
                                              fm_call_ctx_t *ctx,
                                              fm_call_exec_cl *cl) {
  return true;
}

bool fm_comp_perf_timer_stop_stream_exec(fm_frame_t *result, size_t args,
                                         const fm_frame_t *const argv[],
                                         fm_call_ctx_t *ctx,
                                         fm_call_exec_cl cl) {
  auto *closure = (perf_timer *)ctx->comp;
  closure->stop();
  return true;
}

fm_call_def *fm_comp_perf_timer_stop_stream_call(fm_comp_def_cl comp_cl,
                                                 const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_perf_timer_stop_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_perf_timer_stop_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_perf_timer_stop_gen(fm_comp_sys_t *csys,
                                          fm_comp_def_cl closure, unsigned argc,
                                          fm_type_decl_cp argv[],
                                          fm_type_decl_cp ptype,
                                          fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    auto *errstr = "expect a sample name as parameter";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto *name = fm_arg_try_cstring(fm_type_tuple_arg(ptype, 0), &plist);

  if (!name) {
    auto *errstr = "argument provided must be a cstring";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto &comp_cl = *(fmc::counter::samples *)closure;
  auto &cl = comp_cl.get<perf_timer>(name);
  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, true);
  fm_ctx_def_type_set(def, argv[0]);
  fm_ctx_def_closure_set(def, &cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_perf_timer_stop_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_perf_timer_stop_destroy(fm_comp_def_cl, fm_ctx_def_t *) {}

bool fm_comp_perf_timer_add(fm_comp_sys_t *sys, void *samples) {
  const fm_comp_def_t fm_comp_perf_timer_start = {
      "perf_timer_start", &fm_comp_perf_timer_start_gen,
      &fm_comp_perf_timer_start_destroy, samples};
  const fm_comp_def_t fm_comp_perf_timer_stop = {
      "perf_timer_stop", &fm_comp_perf_timer_stop_gen,
      &fm_comp_perf_timer_stop_destroy, samples};
  return fm_comp_type_add(sys, &fm_comp_perf_timer_start) &&
         fm_comp_type_add(sys, &fm_comp_perf_timer_stop);
}
