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
 * @file heartbeat.cpp
 * @author Federico Ravchina
 * @date 7 Jan 2022
 * @brief File contains C++ definitions of the frame decode object
 *
 * This file contains declarations of the ytp record object
 * @see http://www.featuremine.com
 */

extern "C" {
#include "heartbeat.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include <fmc/time.h>
#include <ytp/sequence.h>

#include <array>
#include <chrono>
#include <optional>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <thread>
#include <unordered_map>

struct heartbeat_cl {
  fm_time64_t heartbeat_period;
  fm_time64_t heartbeat_on = fm_time64_start();
  fm_time64_t scheduled_on = fm_time64_end();
  bool updated = false;
};

bool fm_comp_heartbeat_call_stream_init(fm_frame_t *result, size_t args,
                                        const fm_frame_t *const argv[],
                                        fm_call_ctx_t *ctx,
                                        fm_call_exec_cl *cl) {
  auto &exec_cl = *(heartbeat_cl *)ctx->comp;
  auto *s_ctx = (fm_stream_ctx *)ctx->exec;

  exec_cl.heartbeat_on = fm_stream_ctx_now(s_ctx);
  exec_cl.scheduled_on = exec_cl.heartbeat_on;
  fm_stream_ctx_schedule(s_ctx, ctx->handle, exec_cl.scheduled_on);
  return true;
}

bool fm_comp_heartbeat_stream_exec(fm_frame_t *result, size_t,
                                   const fm_frame_t *const argv[],
                                   fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &exec_cl = *(heartbeat_cl *)ctx->comp;
  auto *s_ctx = (fm_stream_ctx *)ctx->exec;

  auto now = fm_stream_ctx_now(s_ctx);

  auto schedule_event = !fm_time64_greater(exec_cl.scheduled_on, now);
  auto time_to_heartbeat = !fm_time64_greater(exec_cl.heartbeat_on, now);
  auto heartbeat_now = time_to_heartbeat && !exec_cl.updated;

  if (exec_cl.updated || time_to_heartbeat) {
    exec_cl.updated = false;

    auto new_heartbeat = fm_time64_add(now, exec_cl.heartbeat_period);
    if (fm_time64_less(exec_cl.heartbeat_on, new_heartbeat)) {
      exec_cl.heartbeat_on = new_heartbeat;
    }
  }

  if (schedule_event) {
    exec_cl.scheduled_on = exec_cl.heartbeat_on;
    fm_stream_ctx_schedule(s_ctx, ctx->handle, exec_cl.scheduled_on);
  }

  if (heartbeat_now) {
    memcpy(fm_frame_get_ptr1(result, 0, 0), &now,
           fm_base_type_sizeof(FM_TYPE_TIME64));
  }

  return heartbeat_now;
}

void fm_comp_heartbeat_stream_queuer(size_t idx, fm_call_ctx_t *ctx) {
  auto &exec_cl = *(heartbeat_cl *)ctx->comp;
  exec_cl.updated = true;
}

fm_call_def *fm_comp_heartbeat_stream_call(fm_comp_def_cl comp_cl,
                                           const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_heartbeat_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_heartbeat_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_heartbeat_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                    unsigned argc, fm_type_decl_cp argv[],
                                    fm_type_decl_cp ptype,
                                    fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  auto param_error = [&]() {
    auto *errstr = "expect a heartbeat period as parameter";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
  };

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    param_error();
    return nullptr;
  }

  auto stream_arg = fm_type_tuple_arg(ptype, 0);

  fm_time64_t heartbeat;
  if (!fm_arg_try_time64(stream_arg, &plist, &heartbeat)) {
    param_error();
    return nullptr;
  }

  std::array<const char *, 1> names = {"heartbeat"};
  std::array<fm_type_decl_cp, 1> types = {
      fm_base_type_get(sys, FM_TYPE_TIME64)};

  std::array<int, 1> dims = {1};
  auto type = fm_frame_type_get1(sys, names.size(), names.data(), types.data(),
                                 dims.size(), dims.data());

  auto *cl = new heartbeat_cl{
      heartbeat,
  };

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_closure_set(def, (void *)cl);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_queuer_set(def, &fm_comp_heartbeat_stream_queuer);
  fm_ctx_def_stream_call_set(def, &fm_comp_heartbeat_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_heartbeat_destroy(fm_comp_def_cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (heartbeat_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr) {
    delete ctx_cl;
  }
}
