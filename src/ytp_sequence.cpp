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
 * @file ytp_sequence.cpp
 * @author Federico Ravchina
 * @date 7 Jan 2022
 * @brief File contains C++ definitions of the ytp sequence object
 *
 * This file contains declarations of the ytp record object
 * @see http://www.featuremine.com
 */

extern "C" {
#include "ytp_sequence.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include "mp_util.hpp"
#include "ytp.h"

#include "fmc++/memory.hpp"
#include "fmc/time.h"

#include <optional>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

static ytp_sequence_api_v1 *ytp_; // ytp_api

struct ytp_sequence_cl {
  shared_sequence *seq;
  fm_time64_t polling_time;

  ytp_sequence_cl(shared_sequence *seq, fm_time64_t polling_time)
      : seq(seq), polling_time(polling_time) {
    ytp_->sequence_shared_inc(seq);
  }

  ~ytp_sequence_cl() {
    fmc_error_t *error;
    ytp_->sequence_shared_dec(seq, &error);
  }
};

bool fm_comp_ytp_sequence_call_stream_init(fm_frame_t *result, size_t args,
                                           const fm_frame_t *const argv[],
                                           fm_call_ctx_t *ctx,
                                           fm_call_exec_cl *cl) {
  auto *s_ctx = (fm_stream_ctx *)ctx->exec;
  fm_stream_ctx_schedule(s_ctx, ctx->handle, fm_stream_ctx_now(s_ctx));
  return true;
}

bool fm_comp_ytp_sequence_stream_exec(fm_frame_t *result, size_t,
                                      const fm_frame_t *const argv[],
                                      fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *s_ctx = (fm_stream_ctx *)ctx->exec;
  auto *seq_cl = (ytp_sequence_cl *)ctx->comp;

  fmc_error_t *error;
  auto poll = ytp_->sequence_poll(seq_cl->seq, &error);
  if (error) {
    auto errstr =
        std::string("unable to poll the sequence: ") + fmc_error_msg(error);
    fm_exec_ctx_error_set(ctx->exec, errstr.c_str());
    return false;
  }

  fm_stream_ctx_schedule(
      s_ctx, ctx->handle,
      poll ? fm_stream_ctx_now(s_ctx)
           : fm_time64_add(fm_stream_ctx_now(s_ctx), seq_cl->polling_time));
  return false;
}

fm_call_def *fm_comp_ytp_sequence_stream_call(fm_comp_def_cl comp_cl,
                                              const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_ytp_sequence_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_ytp_sequence_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_ytp_sequence_gen(fm_comp_sys_t *csys,
                                       fm_comp_def_cl closure, unsigned argc,
                                       fm_type_decl_cp argv[],
                                       fm_type_decl_cp ptype,
                                       fm_arg_stack_t plist) {
  ytp_ = get_ytp_api_v1();

  auto *sys = fm_type_sys_get(csys);

  if (argc != 0) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS,
                           "no input features are "
                           "expected");
    return nullptr;
  }

  auto param_error = [&]() {
    auto *errstr =
        "expect a ytp sequence object, and optionally a polling time";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
  };

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) == 0 ||
      fm_type_tuple_size(ptype) > 2) {
    param_error();
    return nullptr;
  }

  auto ytp_arg = fm_type_tuple_arg(ptype, 0);
  auto polling_time_arg =
      fm_type_tuple_size(ptype) == 2 ? fm_type_tuple_arg(ptype, 1) : nullptr;

  auto ytp_rec_t = fm_record_type_get(sys, "ytp_sequence_wrapper",
                                      sizeof(ytp_sequence_wrapper));

  if (!fm_type_is_record(ytp_rec_t) || !fm_type_equal(ytp_rec_t, ytp_arg)) {
    param_error();
    return nullptr;
  }

  auto sequence = STACK_POP(plist, ytp_sequence_wrapper);
  auto *shared_seq = sequence.sequence;

  fm_time64_t polling_time;
  if (polling_time_arg) {
    if (!fm_arg_try_time64(polling_time_arg, &plist, &polling_time)) {
      param_error();
      return nullptr;
    }
  } else {
    polling_time = fm_time64_from_nanos(0);
  }

  auto *cl = new ytp_sequence_cl(shared_seq, polling_time);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_closure_set(def, (void *)cl);
  fm_ctx_def_type_set(def, ptype);
  fm_ctx_def_stream_call_set(def, &fm_comp_ytp_sequence_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_ytp_sequence_destroy(fm_comp_def_cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (ytp_sequence_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr) {
    delete ctx_cl;
  }
}
