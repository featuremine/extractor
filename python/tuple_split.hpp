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
 * @file tuple_split.hpp
 * @author Maxim Trokhimtchouk
 * @date 16 Jan 2020
 * @brief File contains C++ definitions of the tuple_split
 *
 * tuple_split extractor operator splits incoming tuple
 * into multiple outputs.
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include "python/py_utils.hpp"
#include "python/py_wrapper.hpp"
#include "extractor/type_sys.h"
#include "fmc++/mpl.hpp"

#include <cassert>
#include <errno.h>
#include <functional>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

using namespace fm;
using namespace python;
using namespace std;

struct tuple_split_comp_cl {
  string split_field;
  string buffer;
  unordered_map<string, unsigned> map;
};

bool fm_comp_tuple_split_call_stream_init(fm_frame_t *result, size_t args,
                                          const fm_frame_t *const argv[],
                                          fm_call_ctx_t *ctx,
                                          fm_call_exec_cl *cl) {
  auto *info = (tuple_split_comp_cl *)ctx->comp;

  if (ctx->depc != info->map.size()) {
    fm_exec_ctx_error_set(ctx->exec, "The number of outputs does not equal "
                                     "to the number of split values");
    return false;
  }

  return true;
}

void fm_comp_tuple_split_call_stream_destroy(fm_call_exec_cl cl) {}

bool fm_comp_tuple_split_stream_exec(fm_frame_t *result, size_t,
                                     const fm_frame_t *const argv[],
                                     fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *info = (tuple_split_comp_cl *)ctx->comp;

  auto *ptr = *(PyObject **)fm_frame_get_cptr1(argv[0], 0, 0);
  auto obj = object::from_borrowed(ptr);

  if (!obj)
    return false;

  auto *attr = info->split_field.c_str();
  auto field = obj[attr];
  if (!field) {
    fm_exec_ctx_error_set(ctx->exec,
                          "could not obtain attribute %s from object %s", attr,
                          obj.str().c_str());
    return false;
  }

  if (PyBytes_Check(field.get_ref())) {
    Py_buffer buff;
    if (PyObject_GetBuffer(field.get_ref(), &buff, PyBUF_C_CONTIGUOUS) != 0) {
      fm_exec_ctx_error_set(
          ctx->exec, "could not obtain buffer of attribute %s in object %s",
          attr, obj.str().c_str());
      return false;
    }
    info->buffer.assign((const char *)buff.buf, buff.len);
    PyBuffer_Release(&buff);
  } else if (PyUnicode_Check(field.get_ref())) {
    Py_ssize_t size;
    auto *data = PyUnicode_AsUTF8AndSize(field.get_ref(), &size);
    if (!data) {
      fm_exec_ctx_error_set(
          ctx->exec, "could not obtain value of attribute %s in object %s",
          attr, obj.str().c_str());
      return false;
    }
    info->buffer.assign(data, size);
  } else {
    fm_exec_ctx_error_set(
        ctx->exec, "attribute %s in object %s should be a string, instead %s",
        attr, obj.str().c_str(), field.str().c_str());
    return false;
  }

  auto where = info->map.find(info->buffer);
  if (where == info->map.end())
    return false;

  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  fm_stream_ctx_queue(exec_ctx, ctx->deps[where->second]);
  return false;
}

fm_call_def *fm_comp_tuple_split_stream_call(fm_comp_def_cl comp_cl,
                                             const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_tuple_split_call_stream_init);
  fm_call_def_destroy_set(def, fm_comp_tuple_split_call_stream_destroy);
  fm_call_def_exec_set(def, fm_comp_tuple_split_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_tuple_split_gen(fm_comp_sys_t *csys,
                                      fm_comp_def_cl closure, unsigned argc,
                                      fm_type_decl_cp argv[],
                                      fm_type_decl_cp ptype,
                                      fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  auto rec_t = fm_record_type_get(sys, "PyObject*", sizeof(PyObject *));
  auto in_type = fm_frame_type_get(sys, 1, 1, "update", rec_t, 1);
  if (!in_type) {
    return nullptr;
  }

  if (argc != 1 || !fm_type_equal(argv[0], in_type)) {
    const char *errstr = "a feature whose return is a namedtuple must be "
                         "provided";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }
  auto type = argv[0];

  auto param_error = [&]() {
    auto *errstr =
        "expect a field name, field type and a tuple of split values as "
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
      auto *errstr = "split values must be strings";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    }
  }

  auto field_name = STACK_POP(plist, const char *);

  auto *ctx_cl = new tuple_split_comp_cl();
  ctx_cl->split_field = field_name;

  for (unsigned i = 0; i < split_count; ++i) {
    auto value = STACK_POP(plist, const char *);
    string key(value);
    ctx_cl->map.emplace(key, i);
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, true);
  fm_ctx_def_volatile_set(def, split_count);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_tuple_split_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_tuple_split_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (tuple_split_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}

const fm_comp_def_t fm_comp_tuple_split = {"tuple_split",
                                           &fm_comp_tuple_split_gen,
                                           &fm_comp_tuple_split_destroy, NULL};
