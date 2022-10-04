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
 * @file accumulate.cpp
 * @author Andres Rangel
 * @date 18 Sep 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "accumulate.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include "extractor/time64.hpp"

#include "extractor/time64.h"
#include <deque>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct accum_cl {
  accum_cl(size_t idx, bool exec_on_update)
      : indices_(idx), exec_on_update_(exec_on_update), updated_(false),
        reset_(false) {}
  void init(fm_exec_ctx_t *ctx, const fm_frame_t *arg, fm_frame_t *result) {
    inp_ = arg;

    if (exec_on_update_) {
      data_ = result;
    } else {
      auto *frames = fm_exec_ctx_frames(ctx);
      auto type = fm_frame_type(result);
      data_ = fm_frame_from_type(frames, type);
      fm_frame_assign(data_, result);
    }
  }
  bool exec(fm_stream_ctx_t *ctx, fm_frame_t *result) {
    auto up = updated_;
    if (up) {
      push(ctx);
      updated_ = false;
    }
    if (exec_on_update_ && up) {
      return true;
    } else if (reset_) {
      reset(result);
      return true;
    }
    return false;
  }
  void queue(size_t idx) {
    if (idx == 0) {
      updated_ = true;
    } else if (idx == 1) {
      reset_ = true;
    }
  }
  void push(fm_stream_ctx_t *ctx) {
    auto nd = fm_frame_dim(data_, 0);
    auto nf = indices_.size();
    auto curr = fm_stream_ctx_now(ctx);

    fm_frame_reserve0(data_, nd + 1);
    memcpy(fm_frame_get_ptr1(data_, indices_[nf - 1], nd), &curr,
           sizeof(fm_time64_t));

    for (unsigned i = 0; i < nf - 1; ++i) {
      fm_frame_field_copy_from0(data_, indices_[i], inp_, i, nd);
    }
    updated_ = true;
  }
  void reset(fm_frame_t *result) {
    fm_frame_swap(result, data_);
    fm_frame_reserve0(data_, 0);
    reset_ = false;
  }
  vector<size_t> indices_;
  bool exec_on_update_;
  bool updated_;
  bool reset_;
  fm_frame_t *data_ = nullptr;
  const fm_frame_t *inp_ = nullptr;
};

bool fm_comp_accumulate_call_stream_init(fm_frame_t *result, size_t args,
                                         const fm_frame_t *const argv[],
                                         fm_call_ctx_t *ctx,
                                         fm_call_exec_cl *cl) {
  auto *exec_accum_cl = (accum_cl *)ctx->comp;
  exec_accum_cl->init(ctx->exec, argv[0], result);
  return true;
}

bool fm_comp_accumulate_stream_exec(fm_frame_t *result, size_t,
                                    const fm_frame_t *const argv[],
                                    fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &exec_accum_cl = *(accum_cl *)ctx->comp;
  return exec_accum_cl.exec((fm_stream_ctx_t *)ctx->exec, result);
}

fm_call_def *fm_comp_accumulate_stream_call(fm_comp_def_cl comp_cl,
                                            const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_accumulate_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_accumulate_stream_exec);
  return def;
}

void fm_comp_accumulate_queuer(size_t idx, fm_call_ctx_t *ctx) {
  auto &exec_accum_cl = *(accum_cl *)ctx->comp;
  exec_accum_cl.queue(idx);
}

fm_ctx_def_t *fm_comp_accumulate_gen(fm_comp_sys_t *csys,
                                     fm_comp_def_cl closure, unsigned argc,
                                     fm_type_decl_cp argv[],
                                     fm_type_decl_cp ptype,
                                     fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  if (argc != 1 && argc != 2) {
    auto *errstr = "expect a single operator argument as the input and an "
                   "optional reset operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  int nd = fm_type_frame_ndims(argv[0]);

  if (nd != 1) {
    auto *errstr = "input operator must have only one dimension";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (fm_type_frame_dim(argv[0], 0) != 1) {
    auto *errstr = "input operator dimension must be one";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect no "
                           "parameters");
    return nullptr;
  }

  int nf = fm_type_frame_nfields(argv[0]);

  vector<fm_type_decl_cp> types(nf + 1);
  types[0] = fm_base_type_get(sys, FM_TYPE_TIME64);

  vector<const char *> names(nf + 1);
  names[0] = "Timestamp";

  int dims[1];
  dims[0] = 0;

  for (int i = 0; i < nf; ++i) {
    names[i + 1] = fm_type_frame_field_name(argv[0], i);
    types[i + 1] = fm_type_frame_field_type(argv[0], i);
  }

  auto *type =
      fm_frame_type_get1(sys, nf + 1, names.data(), types.data(), 1, dims);

  auto *cl = new accum_cl(nf + 1, argc == 1);
  auto &cl_v = *cl;
  cl_v.indices_[nf] = fm_type_frame_field_idx(type, "Timestamp");
  for (int i = 1; i < nf + 1; ++i)
    cl_v.indices_[fm_type_frame_field_idx(argv[0], names[i])] =
        fm_type_frame_field_idx(type, names[i]);

  if (!type) {
    auto *errstr = "unable to create result frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_accumulate_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  fm_ctx_def_queuer_set(def, &fm_comp_accumulate_queuer);
  return def;
}

void fm_comp_accumulate_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (accum_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
