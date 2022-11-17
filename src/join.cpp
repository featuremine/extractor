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
 * @file join.cpp
 * @author Maxim Trokhimtchouk
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "join.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"

#include "fmc++/time.hpp"
#include "frame_pool.hpp"

#include "fmc/time.h"
#include <deque>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct join_comp_cl {
  deque<int> inputs;
  frame_pool pool_;
  deque<fm_frame_t *> queue_;
  vector<string> labels;
  vector<pair<fm_field_t, fm_field_t>> fields;
  fm_field_t label_idx;
};

bool fm_comp_join_call_stream_init(fm_frame_t *result, size_t args,
                                   const fm_frame_t *const argv[],
                                   fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  return true;
}

bool fm_comp_join_stream_exec(fm_frame_t *result, size_t,
                              const fm_frame_t *const argv[],
                              fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *ctx_cl = (join_comp_cl *)ctx->comp;
  while (!ctx_cl->inputs.empty()) {
    auto *frame = ctx_cl->pool_.get(result);
    auto idx = ctx_cl->inputs.front();
    ctx_cl->inputs.pop_front();
    auto *inp = argv[idx];
    auto nd_from = fm_frame_dim(inp, 0);
    auto nd_to = fm_frame_dim(frame, 0);
    if (nd_from != nd_to)
      fm_frame_reserve0(frame, nd_from);
    for (auto &&[from, to] : ctx_cl->fields) {
      fm_frame_field_copy(frame, to, inp, from);
    }
    auto &label = ctx_cl->labels[idx];
    auto *where = fm_frame_get_ptr1(frame, ctx_cl->label_idx, 0);
    memcpy(where, label.data(), label.size());

    for (int i = 1; i < nd_from; ++i) {
      where = fm_frame_get_ptr1(frame, ctx_cl->label_idx, i);
      memset(where, 0, label.size());
    }
    ctx_cl->queue_.push_back(frame);
  }

  if (!ctx_cl->queue_.empty()) {
    auto *frame = ctx_cl->queue_.front();
    ctx_cl->queue_.pop_front();
    fm_frame_swap(result, frame);
    ctx_cl->pool_.release(frame);
  }

  if (!ctx_cl->queue_.empty()) {
    auto *stream_ctx = (fm_stream_ctx_t *)ctx->exec;
    auto now = fm_stream_ctx_now(stream_ctx);
    auto handle = ctx->handle;
    fm_stream_ctx_schedule(stream_ctx, handle, now);
  }
  return true;
}

fm_call_def *fm_comp_join_stream_call(fm_comp_def_cl comp_cl,
                                      const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_join_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_join_stream_exec);
  return def;
}

void fm_comp_join_queuer(size_t idx, fm_call_ctx_t *ctx) {
  auto *ctx_cl = (join_comp_cl *)ctx->comp;
  ctx_cl->inputs.push_back(idx);
}

fm_ctx_def_t *fm_comp_join_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                               unsigned argc, fm_type_decl_cp argv[],
                               fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc < 1) {
    auto *errstr = "expect two operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto *label = fm_arg_try_cstring(fm_type_tuple_arg(ptype, 0), &plist);
  auto *label_type = fm_arg_try_type_decl(fm_type_tuple_arg(ptype, 1), &plist);
  auto *tup_t = fm_type_tuple_arg(ptype, 2);
  if (!label || !label_type || !tup_t || fm_type_tuple_size(tup_t) != argc) {
    auto *error = "expecting a label name, "
                  "label type and a tuple of labels, one for each input";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, error);
    return nullptr;
  }

  if (fm_type_base_enum(fm_type_array_of(label_type)) != FM_TYPE_CHAR) {
    auto *error = "only string labels are supported at this point";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, error);
    return nullptr;
  }

  auto in_t = argv[0];
  for (unsigned argi = 1; argi < argc; ++argi) {
    if (!fm_type_equal(in_t, argv[argi])) {
      auto *error = "all inputs must have the same type";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, error);
      return nullptr;
    }
  }

  int nd = fm_type_frame_ndims(in_t);
  vector<int> dims(nd);
  for (int i = 0; i < nd; ++i) {
    dims[i] = fm_type_frame_dim(in_t, i);
  }

  int nf = fm_type_frame_nfields(in_t);
  vector<const char *> names(nf + 1);
  vector<fm_type_decl_cp> types(nf + 1);
  for (int idx = 0; idx < nf; ++idx) {
    names[idx] = fm_type_frame_field_name(in_t, idx);
    types[idx] = fm_type_frame_field_type(in_t, idx);
    if (strcmp(names[idx], label) == 0) {
      auto *error = "join label cannot be the same one of the fields";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, error);
      return nullptr;
    }
  }
  names[nf] = label;
  types[nf] = label_type;

  size_t size = fm_type_array_size(label_type);
  vector<string> labels;
  for (unsigned argi = 0; argi < argc; ++argi) {
    labels.emplace_back(size, 0);
    auto *item = fm_arg_try_cstring(fm_type_tuple_arg(tup_t, argi), &plist);
    if (!item || strlen(item) > size) {
      ostringstream os;
      os << "expecting a tuple of strings each of "
         << "size less than or equal to " << size;
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, os.str().c_str());
      return nullptr;
    }
    strncpy(labels[argi].data(), item, size);
  }

  auto type = fm_frame_type_get1(sys, nf + 1, names.data(), types.data(), nd,
                                 dims.data());
  if (!type) {
    return nullptr;
  }

  auto *ctx_cl = new join_comp_cl();

  for (int idx = 0; idx < nf; ++idx) {
    auto to_idx = fm_type_frame_field_idx(type, names[idx]);
    ctx_cl->fields.emplace_back(idx, to_idx);
  }
  ctx_cl->label_idx = fm_type_frame_field_idx(type, names[nf]);
  ctx_cl->labels = labels;

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_queuer_set(def, &fm_comp_join_queuer);
  fm_ctx_def_stream_call_set(def, &fm_comp_join_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_join_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (join_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
