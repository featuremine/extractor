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
 * @file fields.cpp
 * @author Maxim Trokhimtchouk
 * @date Mar 28 2017
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "fields.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include <stdlib.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

bool fm_comp_fields_call_stream_init(fm_frame_t *result, size_t args,
                                     const fm_frame_t *const argv[],
                                     fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto &exec_cl = *(vector<size_t> *)ctx->comp;
  for (unsigned i = 0; i < exec_cl.size(); ++i)
    fm_frame_field_copy(result, i, argv[0], exec_cl[i]);
  return true;
}

bool fm_comp_fields_stream_exec(fm_frame_t *result, size_t,
                                const fm_frame_t *const argv[],
                                fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &exec_cl = *(vector<size_t> *)ctx->comp;
  for (unsigned i = 0; i < exec_cl.size(); ++i)
    fm_frame_field_copy(result, i, argv[0], exec_cl[i]);
  return true;
}

fm_call_def *fm_comp_fields_stream_call(fm_comp_def_cl comp_cl,
                                        const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_fields_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_fields_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_fields_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                 unsigned argc, fm_type_decl_cp argv[],
                                 fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    auto *errstr = "expects a tuple of names as argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto *names_t = fm_type_tuple_arg(ptype, 0);

  auto t_size = fm_type_tuple_size(names_t);

  if (t_size > fm_type_frame_nfields(argv[0])) {
    auto *errstr = "expecting less names than number of fields in input";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  vector<fm_type_decl_cp> types(t_size);

  vector<const char *> names(t_size);
  for (unsigned i = 0; i < t_size; ++i) {
    names[i] = fm_arg_try_cstring(fm_type_tuple_arg(names_t, i), &plist);
    if (names[i] == nullptr) {
      auto *errstr = "all arguments provided must be strings";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    }
    if (fm_type_frame_field_idx(argv[0], names[i]) == -1) {
      auto *errstr = "all provided field names must exist in input frame";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    }
  }

  for (unsigned i = 0; i < t_size; ++i) {
    types[i] = fm_type_frame_field_type(
        argv[0], fm_type_frame_field_idx(argv[0], names[i]));
  }

  auto nd = fm_type_frame_ndims(argv[0]);
  vector<int> dims(nd);

  for (unsigned i = 0; i < nd; ++i)
    dims[i] = fm_type_frame_dim(argv[0], i);

  auto *type = fm_frame_type_get1(sys, t_size, names.data(), types.data(), nd,
                                  dims.data());

  if (!type) {
    auto *errstr = "unable to create result frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto *cl = new vector<size_t>(t_size);
  auto &cl_r = *cl;

  for (unsigned i = 0; i < t_size; ++i)
    cl_r[fm_type_frame_field_idx(type, names[i])] =
        fm_type_frame_field_idx(argv[0], names[i]);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_closure_set(def, (void *)cl);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_stream_call_set(def, &fm_comp_fields_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_fields_destroy(fm_comp_def_cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (vector<size_t> *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
