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
 * @file time_lag.cpp
 * @author Maxim Trokhimtchouk
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "time_lag.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "fmc++/time.hpp"

#include "fmc/time.h"
#include <deque>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
using namespace std;

struct time_lag_comp_cl {
  fmc_time64_t lag;
  fmc_time64_t resolution;
  bool enqueue = false;
  deque<pair<fmc_time64_t, fm_frame_t *>> queue;
  vector<fm_frame_t *> pool;
};

constexpr auto DEFAULT_POOL_SIZE = 4l;

bool fm_comp_time_lag_call_stream_init(fm_frame_t *result, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx,
                                       fm_call_exec_cl *cl) {
  auto *info = (time_lag_comp_cl *)ctx->comp;
  auto *frames = fm_exec_ctx_frames((fm_exec_ctx *)ctx->exec);
  auto type = fm_frame_type(result);
  for (auto &frame : info->pool) {
    frame = fm_frame_from_type(frames, type);
  }
  fm_frame_assign(result, argv[0]);
  return true;
}

bool fm_comp_time_lag_stream_exec(fm_frame_t *result, size_t,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *info = (time_lag_comp_cl *)ctx->comp;
  auto &queue = info->queue;
  auto *stream_ctx = (fm_stream_ctx_t *)ctx->exec;
  auto now = fm_stream_ctx_now(stream_ctx);
  fm_frame_t *clone_frame = nullptr;
  if (!queue.empty() && queue.front().first <= now) {
    clone_frame = queue.front().second;
    info->pool.push_back(clone_frame);
    queue.pop_front();
    fm_frame_swap(result, clone_frame);
  }
  if (!queue.empty() && queue.front().first <= now) {
    fm_stream_ctx_schedule(stream_ctx, ctx->handle, now);
  }
  if (info->enqueue == true) {
    info->enqueue = false;
    auto next_time = now + info->lag;
    if (queue.empty() || queue.back().first + info->resolution <= next_time) {
      fm_frame_t *frame;
      if (!info->pool.empty()) {
        frame = info->pool.back();
        info->pool.pop_back();
      } else {
        auto *frames = fm_exec_ctx_frames((fm_exec_ctx *)ctx->exec);
        auto type = fm_frame_type(result);
        frame = fm_frame_from_type(frames, type);
      }
      fm_frame_assign(frame, argv[0]);
      queue.emplace_back(next_time, frame);
      fm_stream_ctx_schedule(stream_ctx, ctx->handle, next_time);
    }
  }
  return clone_frame != nullptr;
}

fm_call_def *fm_comp_time_lag_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_time_lag_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_time_lag_stream_exec);
  return def;
}

void fm_comp_time_lag_queuer(size_t idx, fm_call_ctx_t *ctx) {
  auto *info = (time_lag_comp_cl *)ctx->comp;
  info->enqueue = true;
}

fm_ctx_def_t *fm_comp_time_lag_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
                                   fm_type_decl_cp ptype,
                                   fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 2) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect a lag time "
                           "and resolution as a "
                           "parameters");
    return nullptr;
  }

  fmc_time64_t lag{0};
  if (!fm_arg_try_time64(fm_type_tuple_arg(ptype, 0), &plist, &lag)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect first parameter to be a lag time");
    return nullptr;
  }

  fmc_time64_t resolution{0};
  if (!fm_arg_try_time64(fm_type_tuple_arg(ptype, 1), &plist, &resolution)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect second "
                           "parameter to be a "
                           "resolution time");
    return nullptr;
  }

  auto count = resolution.value > 0 ? 1 + lag / resolution : DEFAULT_POOL_SIZE;
  if (count > 1000) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "lag is more than the allowed 1000 "
                           "times greater than the resolution");
    return nullptr;
  }
  auto *ctx_cl = new time_lag_comp_cl();
  ctx_cl->lag = lag;
  ctx_cl->resolution = resolution;
  ctx_cl->pool.resize(count, nullptr);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, argv[0]);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_queuer_set(def, &fm_comp_time_lag_queuer);
  fm_ctx_def_stream_call_set(def, &fm_comp_time_lag_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_time_lag_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (time_lag_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
