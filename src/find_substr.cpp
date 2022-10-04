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
 * @file find.cpp
 * @authors Andres Rangel
 * @date 23 Aug 2018
 * @brief File contains C++ definitions for the "find" logical operator
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "substr.h"
#include "extractor/time64.h"
}

#include "extractor/decimal64.hpp"
#include "extractor/frame.hpp"
#include "op_util.hpp"
#include "extractor/time64.hpp"
#include <fmc++/mpl.hpp>

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

struct find_field_exec {
  find_field_exec(fm_field_t field, const char *substr, size_t substrsz)
      : field_(field), substr_(substr, substrsz) {}
  void exec(fm_frame_t *result, const fm_frame_t *const argv) {
    bool res = strstr((const char *)fm_frame_get_cptr1(argv, field_, 0),
                      substr_.data()) != nullptr;

    *(bool *)fm_frame_get_ptr1(result, field_, 0) = res;
  }
  fm_field_t field_;
  string substr_;
};

struct find_comp_cl {
  vector<find_field_exec> calls;
};

bool fm_comp_find_call_stream_init(fm_frame_t *result, size_t args,
                                   const fm_frame_t *const argv[],
                                   fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {

  auto *closure = (find_comp_cl *)ctx->comp;

  for (auto &&call : closure->calls) {
    call.exec(result, argv[0]);
  }

  return true;
}

bool fm_comp_find_stream_exec(fm_frame_t *result, size_t args,
                              const fm_frame_t *const argv[],
                              fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *closure = (find_comp_cl *)ctx->comp;

  for (auto &&call : closure->calls) {
    call.exec(result, argv[0]);
  }

  return true;
}

fm_call_def *fm_comp_find_stream_call(fm_comp_def_cl comp_cl,
                                      const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_find_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_find_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_find_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                               unsigned argc, fm_type_decl_cp argv[],
                               fm_type_decl_cp ptype, fm_arg_stack_t plist) {

  auto *sys = fm_type_sys_get(csys);

  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    auto *errstr = "expect a substring value as parameter";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto *substr = fm_arg_try_cstring(fm_type_tuple_arg(ptype, 0), &plist);

  if (!substr) {
    auto *errstr = "unable to obtain substring from args";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto substrsz = strlen(substr);

  auto ctx_cl = make_unique<find_comp_cl>();

  auto inp = argv[0];
  int nf = fm_type_frame_nfields(inp);

  vector<const char *> names(nf);
  vector<fm_type_decl_cp> types(nf);
  int dims[1] = {1};
  for (int idx = 0; idx < nf; ++idx) {
    auto f_type = fm_type_frame_field_type(inp, idx);
    if (!fm_type_is_array(f_type)) {
      auto *errstr = "field type must be string array";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
      return nullptr;
    }

    auto af_type = fm_type_array_of(f_type);
    if (!fm_type_is_base(af_type) ||
        fm_type_base_enum(af_type) != FM_TYPE_CHAR) {
      auto *errstr = "field array type must be string";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
      return nullptr;
    }

    auto farr_sz = fm_type_array_size(f_type);
    if (farr_sz < substrsz) {
      auto *errstr = "substring is larger than field length";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
      return nullptr;
    }

    names[idx] = fm_type_frame_field_name(inp, idx);
    types[idx] = fm_base_type_get(sys, FM_TYPE_BOOL);
    ctx_cl->calls.emplace_back(idx, substr, substrsz);
  }

  auto type = fm_frame_type_get1(sys, nf, names.data(), types.data(), 1, dims);
  if (!type) {
    auto *errstr = "unable to create result frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_find_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_find_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (find_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
