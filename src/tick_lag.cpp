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
 * @file tick_lag.cpp
 * @author Maxim Trokhimtchouk
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "tick_lag.h"
#include "arg_stack.h"
#include "comp_def.h"
#include "comp_sys.h"
#include "stream_ctx.h"
#include "time64.h"
}

#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

struct tick_lag_comp_cl {
  uint64_t cursor_ = 0;
  vector<fm_frame_t *> buffer_;
};

bool fm_comp_tick_lag_call_stream_init(fm_frame_t *result, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx,
                                       fm_call_exec_cl *cl) {
  auto *info = (tick_lag_comp_cl *)ctx->comp;
  auto *frames = fm_exec_ctx_frames((fm_exec_ctx *)ctx->exec);
  auto type = fm_frame_type(result);
  for (auto &frame : info->buffer_) {
    frame = fm_frame_from_type(frames, type);
  }
  fm_frame_assign(result, argv[0]);
  return true;
}

bool fm_comp_tick_lag_stream_exec(fm_frame_t *result, size_t,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *info = (tick_lag_comp_cl *)ctx->comp;
  auto &cursor = info->cursor_;
  auto &buffer = info->buffer_;
  auto lag = buffer.size();
  auto idx = cursor % lag;
  bool done = false;
  if (cursor >= lag) {
    fm_frame_swap(result, buffer[idx]);
    done = true;
  }
  fm_frame_assign(buffer[idx], argv[0]);
  ++cursor;
  return done;
}

fm_call_def *fm_comp_tick_lag_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_tick_lag_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_tick_lag_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_tick_lag_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
                                   fm_type_decl_cp ptype,
                                   fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect a lag offset "
                           "as first parameter");
    return nullptr;
  }

  uint64_t lag = 0;
  if (!fm_arg_try_uinteger(fm_type_tuple_arg(ptype, 0), &plist, &lag)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect a positive integer as a lag parameter");
    return nullptr;
  }

  auto *ctx_cl = new tick_lag_comp_cl();
  ctx_cl->buffer_.resize(lag, nullptr);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, argv[0]);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_tick_lag_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_tick_lag_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (tick_lag_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
