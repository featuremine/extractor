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
 * @file substr.cpp
 * @authors Andres Rangel
 * @date 23 Aug 2018
 * @brief File contains C++ definitions for the "substr" operator
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "substr.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
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

struct substr_comp_cl {
  substr_comp_cl(size_t fields, uint64_t substr_start, uint64_t substr_sz)
      : fields_(fields), substr_start_(substr_start), substr_sz_(substr_sz) {
    // @note should we copy the string in substr to substr_ using
    // strncpy to ensure that the substring will not be deleted
    // while we are still using it?
    // Maybe we could use a string instead of a const char*
  }

  size_t fields_;
  uint64_t substr_start_;
  uint64_t substr_sz_;
};

bool fm_comp_substr_call_stream_init(fm_frame_t *result, size_t args,
                                     const fm_frame_t *const argv[],
                                     fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {

  auto *closure = (substr_comp_cl *)ctx->comp;

  for (size_t i = 0; i < closure->fields_; ++i) {
    memcpy(fm_frame_get_ptr1(result, i, 0),
           (const char *)fm_frame_get_cptr1(argv[0], i, 0) +
               closure->substr_start_,
           closure->substr_sz_);
  }

  return true;
}

bool fm_comp_substr_stream_exec(fm_frame_t *result, size_t args,
                                const fm_frame_t *const argv[],
                                fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *closure = (substr_comp_cl *)ctx->comp;

  for (size_t i = 0; i < closure->fields_; ++i) {
    memcpy(fm_frame_get_ptr1(result, i, 0),
           (const char *)fm_frame_get_cptr1(argv[0], i, 0) +
               closure->substr_start_,
           closure->substr_sz_);
  }

  return true;
}

fm_call_def *fm_comp_substr_stream_call(fm_comp_def_cl comp_cl,
                                        const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_substr_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_substr_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_substr_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                 unsigned argc, fm_type_decl_cp argv[],
                                 fm_type_decl_cp ptype, fm_arg_stack_t plist) {

  auto *sys = fm_type_sys_get(csys);

  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype)) {
    auto *errstr = "expect the indices for begin and end of substring. End "
                   "index is optional.";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto argsz = fm_type_tuple_size(ptype);

  if ((argsz != 1 && argsz != 2)) {
    auto *errstr = "expect the indices for begin and end of substring. End "
                   "index is optional.";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  int64_t start, end;
  if (!fm_arg_try_integer(fm_type_tuple_arg(ptype, 0), &plist, &start)) {
    auto *errstr = "unable to obtain start index";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  if (argsz == 2) {
    if (!fm_arg_try_integer(fm_type_tuple_arg(ptype, 1), &plist, &end)) {
      auto *errstr = "unable to obtain end index";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    }
  } else {
    end = -1;
  }

  auto inp = argv[0];
  int nf = fm_type_frame_nfields(inp);

  if (fm_type_frame_ndims(inp) != 1 || fm_type_frame_dim(inp, 0) != 1) {
    auto *errstr = "invalid dimensions in input frame";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto arr_sz = std::numeric_limits<size_t>::max();

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
    if (farr_sz < arr_sz)
      arr_sz = farr_sz;
  }

  if (start < 0)
    start = arr_sz + start;

  if (start < 0 || (size_t)start > arr_sz) {
    auto *errstr = "invalid start index of substring";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (end < 0)
    end = arr_sz + end;

  if (end < 0 || (size_t)end > arr_sz) {
    auto *errstr = "invalid end index of substring";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (start > end) {
    auto *errstr =
        "invalid range, start index is higher than end index of substring";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto substr_sz = end - start + 1;

  auto ctx_cl = make_unique<substr_comp_cl>(nf, start, substr_sz);

  vector<const char *> names(nf);
  vector<fm_type_decl_cp> types(nf);
  int dims[1] = {1};
  for (int idx = 0; idx < nf; ++idx) {
    names[idx] = fm_type_frame_field_name(inp, idx);
    types[idx] =
        fm_array_type_get(sys, fm_base_type_get(sys, FM_TYPE_CHAR), substr_sz);
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
  fm_ctx_def_stream_call_set(def, &fm_comp_substr_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_substr_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (substr_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
