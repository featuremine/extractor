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
 * @file split.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "split.h"
#include "arg_stack.h"
#include "comp_def.h"
#include "comp_sys.h"
#include "stream_ctx.h"
#include "time64.h"
}

#include <stdlib.h>
#include <string.h>
#include <string>
#include <unordered_map>

using namespace std;

struct split_comp_cl {
  fm_field_t split_field = -1;
  string buffer;
  unordered_map<string, unsigned> map;
};

bool fm_comp_split_call_stream_init(fm_frame_t *result, size_t args,
                                    const fm_frame_t *const argv[],
                                    fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  using namespace std;
  auto *info = (split_comp_cl *)ctx->comp;

  if (ctx->depc != info->map.size()) {
    fm_exec_ctx_error_set(ctx->exec, "The number of outputs does not equal "
                                     "to the number of split values");
    return false;
  }

  return true;
}

void fm_comp_split_call_stream_destroy(fm_call_exec_cl cl) {}

bool fm_comp_split_stream_exec(fm_frame_t *result, size_t,
                               const fm_frame_t *const argv[],
                               fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *info = (split_comp_cl *)ctx->comp;
  auto *x = (const char *)fm_frame_get_cptr1(argv[0], info->split_field, 0);
  auto size = info->buffer.size();
  memcpy(info->buffer.data(), x, size);
  auto where = info->map.find(info->buffer);
  if (where == info->map.end())
    return false;
  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  fm_stream_ctx_queue(exec_ctx, ctx->deps[where->second]);
  return false;
}

fm_call_def *fm_comp_split_stream_call(fm_comp_def_cl comp_cl,
                                       const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_split_call_stream_init);
  fm_call_def_destroy_set(def, fm_comp_split_call_stream_destroy);
  fm_call_def_exec_set(def, fm_comp_split_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_split_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                unsigned argc, fm_type_decl_cp argv[],
                                fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }
  auto type = argv[0];
  if (!fm_type_is_frame(type) || fm_type_frame_ndims(type) != 1) {
    auto *errstr = "the result of the input operator is expected to be "
                   "one-dimensional frame";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto param_error = [&]() {
    auto *errstr = "expect a field name and a tuple of split values as "
                   "parameters";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 2) {
    return param_error();
  }

  if (!fm_type_is_cstring(fm_type_tuple_arg(ptype, 0))) {
    return param_error();
  }

  auto split_param = fm_type_tuple_arg(ptype, 1);
  if (!fm_type_is_tuple(split_param)) {
    string errstr = "expect second parameter to be a tuple of split "
                    "values, instead got ";
    char *type_str = fm_type_to_str(split_param);
    if (!type_str) {
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                             "could not get "
                             "type string");
    }
    errstr.append(type_str);
    free(type_str);
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr.c_str());
    return nullptr;
  }

  unsigned split_count = fm_type_tuple_size(split_param);
  for (unsigned i = 0; i < split_count; ++i) {
    if (!fm_type_is_cstring(fm_type_tuple_arg(split_param, i))) {
      auto *errstr = "split values must have same type as a split field";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    }
  }

  auto field_name = STACK_POP(plist, const char *);
  auto field_idx = fm_type_frame_field_idx(type, field_name);
  if (field_idx < 0) {
    auto *errstr = "an input result frame must have the field specified";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto type_error = [&]() {
    auto *errstr = "split value type must be an array of char";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  auto field_type = fm_type_frame_field_type(type, field_idx);
  if (!fm_type_is_array(field_type)) {
    return type_error();
  }
  if (!fm_type_is_base(fm_type_array_of(field_type))) {
    return type_error();
  }
  if (fm_type_base_enum(fm_type_array_of(field_type)) != FM_TYPE_CHAR) {
    return type_error();
  }

  auto *ctx_cl = new split_comp_cl();

  ctx_cl->split_field = field_idx;
  auto field_len = fm_type_array_size(field_type);
  ctx_cl->buffer.resize(field_len);

  for (unsigned i = 0; i < split_count; ++i) {
    auto value = STACK_POP(plist, const char *);
    string key(value);
    if (key.size() > field_len) {
      string errstr = "a split value ";
      errstr.append(value);
      errstr.append(" is longer than the field length ");
      errstr.append(to_string(field_len));
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr.c_str());
      delete ctx_cl;
      return nullptr;
    }
    key.resize(field_len);
    ctx_cl->map.emplace(key, i);
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, true);
  fm_ctx_def_volatile_set(def, split_count);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_split_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_split_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (split_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
