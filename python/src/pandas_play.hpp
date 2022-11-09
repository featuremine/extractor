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
 * @file pandas_play.cpp
 * @author Andres Rangel
 * @date 13 Mar 2019
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "errno.h"
#include "fmc++/counters.hpp"
#include "fmc++/mpl.hpp"
#include <cassert>
#include <functional>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include <iostream>

#include "fmc++/strings.hpp"
#include "wrapper.hpp"
#include <fmc++/python/wrapper.hpp>
#include <numpy/arrayobject.h>
#include <py_decimal128.hpp>

#include "extractor/type_sys.h"

using namespace fm;
using namespace python;
using namespace std;

struct pandas_play_info {
  pandas_play_info(fm_comp_sys_t *ctx, PyObject *d)
      : ctx(ctx), dataframe(object::from_borrowed(d)) {}
  ~pandas_play_info() {}
  fm_comp_sys_t *ctx;
  object dataframe;
};

struct pandas_play_exec_cl {
  using parser = vector<uint32_t>;
  pandas_play_exec_cl(object dataframe) { it = dataframe["itertuples"](); }
  object get_next() {
    curr = it.next();
    return curr;
  }
  object it;
  parser parsers;
  object curr;
  fm_frame_t *next;
};

// Follow
// http://pandas.pydata.org/pandas-docs/stable/getting_started/basics.html#dtypes

static bool add_column_parser(fm_exec_ctx_t *ctx, fm_frame_t *frame,
                              pandas_play_exec_cl::parser &parsers,
                              fm_field_t df_idx, const char *name,
                              object dtype) {
  auto offset = fm_frame_field(frame, name);
  auto decl = fm_frame_field_type(frame, name);

  int fdtype = -1;

  auto *np_col = (PyArray_Descr *)dtype.get_ref();

  if (!PyArray_DescrCheck(np_col)) {
    const char *errstr = "object provided as description is not valid.";
    fm_exec_ctx_error_set(ctx, errstr);
    return false;
  }

  auto error = [ctx, name, decl, dtype](const char *options) {
    auto *typestr = fm_type_to_str(decl);
    auto errstr = string("invalid object type in DataFrame in column ") + name +
                  ".\n" + "\tcannot convert type " + dtype.str() + " to " +
                  typestr + ", expecting: " + options;
    fm_exec_ctx_error_set(ctx, errstr.c_str());
    free(typestr);
    return false;
  };

  fdtype = np_col->type_num;

  if (fm_type_is_base(decl)) {
    uint32_t type = 0;
    switch (fm_type_base_enum(decl)) {
    case FM_TYPE_INT8:
      if (fdtype != NPY_INT8)
        return error("int8.");
      type = 1;
      break;
    case FM_TYPE_INT16:
      if (fdtype != NPY_INT16 && fdtype != NPY_INT8)
        return error("int16, int8.");
      type = 2;
      break;
    case FM_TYPE_INT32:
      if (fdtype != NPY_INT32 && fdtype != NPY_INT16 && fdtype != NPY_INT8)
        return error("int32, int16, int8.");
      type = 3;
      break;
    case FM_TYPE_INT64:
      if (fdtype != NPY_INT64 && fdtype != NPY_INT32 && fdtype != NPY_INT16 &&
          fdtype != NPY_INT8)
        return error("int64, int32, int16, int8.");
      type = 4;
      break;
    case FM_TYPE_UINT8:
      if (fdtype != NPY_UINT8)
        return error("uint8.");
      type = 5;
      break;
    case FM_TYPE_UINT16:
      if (fdtype != NPY_UINT16 && fdtype != NPY_UINT8)
        return error("uint16, uint8.");
      type = 6;
      break;
    case FM_TYPE_UINT32:
      if (fdtype != NPY_UINT32 && fdtype != NPY_UINT16 && fdtype != NPY_UINT8)
        return error("uint32, uint16, uint8.");
      type = 7;
      break;
    case FM_TYPE_UINT64:
      if (fdtype != NPY_UINT64 && fdtype != NPY_UINT16 &&
          fdtype != NPY_UINT32 && fdtype != NPY_UINT8)
        return error("uint64, uint32, uint16, uint8.");
      type = 8;
      break;
    case FM_TYPE_FLOAT32:
      if (fdtype != NPY_FLOAT32)
        return error("float32.");
      type = 9;
      break;
    case FM_TYPE_FLOAT64:
      if (fdtype != NPY_FLOAT64 && fdtype != NPY_FLOAT32)
        return error("float64, float32.");
      type = 10;
      break;
    case FM_TYPE_CHAR:
      if (fdtype != NPY_CHAR && fdtype != NPY_STRING && fdtype != NPY_OBJECT)
        return error("char");
      type = 15;
      break;
    case FM_TYPE_RPRICE:
      if (fdtype != NPY_FLOAT64 && fdtype != NPY_FLOAT32)
        return error("float64, float32.");
      type = 11;
      break;
    case FM_TYPE_DECIMAL128:
      if (fdtype != NPY_OBJECT)
        return error("object.");
      type = 16;
      break;
    case FM_TYPE_TIME64:
      if (fdtype != NPY_DATETIME)
        return error("datetime64[ns].");
      type = 12;
      break;
    case FM_TYPE_BOOL:
      if (fdtype != NPY_BOOL)
        return error("bool.");
      type = 13;
      break;
    case FM_TYPE_LAST:
    default:
      auto *typestr = fm_type_to_str(decl);
      auto errstr = string("unsupported type ") + typestr +
                    " in extractor frame type description for column " + name +
                    ".\n";
      fm_exec_ctx_error_set(ctx, errstr.c_str());
      free(typestr);
      return false;
      break;
    }
    parsers.push_back(type);
    parsers.push_back(offset);
    parsers.push_back(df_idx);
  } else if (fm_type_is_array(decl) &&
             fm_type_is_base(fm_type_array_of(decl)) &&
             fm_type_base_enum(fm_type_array_of(decl)) == FM_TYPE_CHAR) {
    if (fdtype == NPY_OBJECT) {
      parsers.push_back(14);
      parsers.push_back(offset);
      parsers.push_back(df_idx);
      parsers.push_back(fm_type_array_size(decl));
    } else {
      return error("object");
    }
  } else {
    auto *typestr = fm_type_to_str(decl);
    auto errstr = string("unsupported type ") + typestr +
                  " in extractor frame type description for column " + name +
                  ".\n";
    fm_exec_ctx_error_set(ctx, errstr.c_str());
    free(typestr);
    return false;
  }

  return true;
}

bool pandas_parse_one(fm_exec_ctx_t *ctx, pandas_play_exec_cl *cl,
                      fm_frame_t *frame, int row) {
  auto error = [ctx](const char *msg) {
    auto errstr = string("error parsing field.\n\t") + msg + "\n";
    fm_exec_ctx_error_set(ctx, errstr.c_str());
    return false;
  };

  auto field_error = [error]() { return error("unable to obtain field data"); };

  for (uint32_t p_off = 0; p_off < cl->parsers.size();) {
    switch (cl->parsers[p_off]) {
    case 1: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(INT8 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          PyLong_AsLong(item.get_ref());
      p_off += 3;
    } break;
    case 2: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(INT16 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          PyLong_AsLong(item.get_ref());
      p_off += 3;
    } break;
    case 3: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(INT32 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          PyLong_AsLong(item.get_ref());
      p_off += 3;
    } break;
    case 4: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(INT64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          PyLong_AsLongLong(item.get_ref());
      p_off += 3;
    } break;
    case 5: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(UINT8 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          PyLong_AsUnsignedLong(item.get_ref());
      p_off += 3;
    } break;
    case 6: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(UINT16 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          PyLong_AsUnsignedLong(item.get_ref());
      p_off += 3;
    } break;
    case 7: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(UINT32 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          PyLong_AsUnsignedLong(item.get_ref());
      p_off += 3;
    } break;
    case 8: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(UINT64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          PyLong_AsUnsignedLongLong(item.get_ref());
      p_off += 3;
    } break;
    case 9: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(FLOAT32 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          PyFloat_AsDouble(item.get_ref());
      p_off += 3;
    } break;
    case 10: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(FLOAT64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          PyFloat_AsDouble(item.get_ref());
      p_off += 3;
    } break;
    case 11: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      fmc_rprice_from_double(
          (RPRICE *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row),
          PyFloat_AsDouble(item.get_ref()));
      p_off += 3;
    } break;
    case 12: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      auto val = item["value"];
      if (!bool(val))
        return error("unable to obtain timestamp value");
      *(TIME64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          fmc_time64_from_nanos(PyLong_AsLongLong(val.get_ref()));
      p_off += 3;
    } break;
    case 13: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      *(BOOL *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          (bool)PyObject_IsTrue(item.get_ref());
      p_off += 3;
    } break;
    case 14: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      if (PyBytes_Check(item.get_ref())) {
        Py_buffer buff;
        if (PyObject_GetBuffer(item.get_ref(), &buff, PyBUF_C_CONTIGUOUS) != 0)
          return error("unable to obtain temporary buffer");
        if (buff.len > cl->parsers[p_off + 3])
          return error("bytes data is larger than field size");
        auto *dest = fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row);
        memset(dest, '\0', cl->parsers[p_off + 3]);
        memcpy(dest, buff.buf, buff.len);
        PyBuffer_Release(&buff);
      } else if (PyUnicode_Check(item.get_ref())) {
        auto *dest = fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row);
        Py_ssize_t size;
        auto *data = PyUnicode_AsUTF8AndSize(item.get_ref(), &size);
        if (!data)
          return error("unable to decode string as utf-8");
        if (size > cl->parsers[p_off + 3])
          return error("string is longer than field size");
        memset(dest, '\0', cl->parsers[p_off + 3]);
        memcpy(dest, data, size);
      }
      p_off += 4;
    } break;
    case 15: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();

      if (!bool(item))
        return field_error();
      Py_ssize_t size;
      *(CHAR *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          *PyUnicode_AsUTF8AndSize(item.get_ref(), &size);
      p_off += 3;
    } break;
    case 16: {
      auto item = object::from_borrowed(
          PyTuple_GetItem(cl->curr.get_ref(), cl->parsers[p_off + 2] + 1));
      if (!bool(item))
        return field_error();
      if (!PyObject_IsInstance(item.get_ref(),
                               (PyObject *)&ExtractorBaseTypeDecimal128Type))
        return field_error();
      ExtractorBaseTypeDecimal128 *dec =
          (ExtractorBaseTypeDecimal128 *)item.get_ref();
      *(DECIMAL128 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          dec->val;
      p_off += 3;
    } break;
    default:
      break;
    }
  }
  return true;
}

bool fm_comp_pandas_play_call_init(fm_frame_t *result, size_t args,
                                   const fm_frame_t *const argv[],
                                   fm_call_ctx_t *call_ctx,
                                   fm_call_exec_cl *cl) {
  using namespace std;
  auto *info = (pandas_play_info *)call_ctx->comp;
  auto *ctx = call_ctx->exec;

  auto *exec_cl = new pandas_play_exec_cl(info->dataframe);

  auto error = [&]() {
    delete exec_cl;
    return false;
  };

  auto column_names = info->dataframe["columns"];

  if (!bool(column_names)) {
    fm_exec_ctx_error_set(ctx, "unable to obtain columns from dataframe");
    return error();
  }

  auto column_types = info->dataframe["dtypes"];

  if (!bool(column_types)) {
    fm_exec_ctx_error_set(ctx, "unable to obtain dtypes from dataframe");
    return error();
  }

  auto col_name_iter = column_names.iter();
  auto col_type_iter = column_types.iter();

  auto col_name = col_name_iter.next();
  auto col_type = col_type_iter.next();

  unsigned count = 0;
  unsigned success = 0U;
  while (bool(col_type) && bool(col_name)) {

    auto *c_col_name = PyUnicode_AsUTF8(col_name.get_ref());

    if (!c_col_name) {
      fm_exec_ctx_error_set(ctx, "unable to decode field name as utf-8");
      return error();
    }

    auto offset = fm_frame_field(result, c_col_name);
    ++count;
    if (!fm_field_valid(offset)) {
      col_name = col_name_iter.next();
      col_type = col_type_iter.next();
      continue;
    }

    if (add_column_parser(ctx, result, exec_cl->parsers, count - 1, c_col_name,
                          col_type)) {
      ++success;
    }

    col_name = col_name_iter.next();
    col_type = col_type_iter.next();
  };

  // check the length
  if (success != fm_type_frame_nfields(fm_frame_type(result))) {
    const char *errstr = "unable to find all the described fields in the "
                         "dataframe";
    fm_exec_ctx_error_set(ctx, errstr);
    return error();
  }

  auto *index_type =
      (PyArray_Descr *)info->dataframe["index"]["dtype"].get_ref();

  auto datetime = datetime::get_pandas_dttz_type();
  if (!datetime) {
    const char *errstr =
        "cannot create pandas.core.dtypes.dtypes.DatetimeTZDtype python object";
    fm_exec_ctx_error_set(ctx, errstr);
    return error();
  }

  if (PyArray_DescrCheck(index_type)) {
    if (index_type->type_num != NPY_DATETIME) {
      const char *errstr = "provided type for index is not valid, expecting "
                           "datetime64[ns]";
      fm_exec_ctx_error_set(ctx, errstr);
      return error();
    }
  } else if (!PyObject_TypeCheck(index_type,
                                 (PyTypeObject *)datetime.get_ref())) {
    const char *errstr = "invalid index type description";
    fm_exec_ctx_error_set(ctx, errstr);
    return error();
  }

  *cl = exec_cl;
  return true;
}

bool fm_comp_pandas_play_call_stream_init(fm_frame_t *result, size_t args,
                                          const fm_frame_t *const argv[],
                                          fm_call_ctx_t *ctx,
                                          fm_call_exec_cl *cl) {
  if (!fm_comp_pandas_play_call_init(result, args, argv, ctx, cl))
    return false;

  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exec_cl = (pandas_play_exec_cl *)*cl;

  auto *frames = fm_exec_ctx_frames((fm_exec_ctx *)ctx->exec);
  auto type = fm_frame_type(result);
  exec_cl->next = fm_frame_from_type(frames, type);
  fm_frame_reserve(exec_cl->next, 1);

  if (!bool(exec_cl->get_next())) {
    if (PyErr_Occurred()) {
      fm_exec_ctx_error_set(ctx->exec, "unable to obtain first entry of "
                                       "dataframe");
      return false;
    }
    return true;
  }

  if (!pandas_parse_one(ctx->exec, exec_cl, exec_cl->next, 0))
    return false;

  auto item =
      object::from_borrowed(PyTuple_GetItem(exec_cl->curr.get_ref(), 0));
  if (!bool(item)) {
    fm_exec_ctx_error_set(ctx->exec, "unable to obtain index data");
    return false;
  }

  auto val = item["value"];
  if (!bool(val)) {
    fm_exec_ctx_error_set(ctx->exec, "unable to obtain index value");
    return false;
  }

  auto next = fmc_time64_from_nanos(PyLong_AsLongLong(val.get_ref()));
  fm_stream_ctx_schedule(exec_ctx, ctx->handle, next);

  return true;
}

void fm_comp_pandas_play_call_stream_destroy(fm_call_exec_cl cl) {
  auto *exec_cl = (pandas_play_exec_cl *)cl;
  if (exec_cl)
    delete exec_cl;
}

bool fm_comp_pandas_play_stream_exec(fm_frame_t *result, size_t,
                                     const fm_frame_t *const argv[],
                                     fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exec_cl = (pandas_play_exec_cl *)cl;

  fm_frame_swap(result, exec_cl->next);

  if (exec_cl->get_next()) {
    if (pandas_parse_one(ctx->exec, exec_cl, exec_cl->next, 0)) {
      auto item =
          object::from_borrowed(PyTuple_GetItem(exec_cl->curr.get_ref(), 0));
      if (!bool(item)) {
        fm_exec_ctx_error_set(ctx->exec, "unable to obtain index data");
        return false;
      }

      auto val = item["value"];
      if (!bool(val)) {
        fm_exec_ctx_error_set(ctx->exec, "unable to obtain index "
                                         "value");
        return false;
      }

      auto next = fmc_time64_from_nanos(PyLong_AsLongLong(val.get_ref()));
      fm_stream_ctx_schedule(exec_ctx, ctx->handle, next);
    } else {
      return false;
    }
  } else {
    if (PyErr_Occurred()) {
      fm_exec_ctx_error_set(ctx->exec, "unable to obtain next row of "
                                       "DataFrame");
      return false;
    }
  }
  return true;
}

fm_call_def *fm_comp_pandas_play_stream_call(fm_comp_def_cl comp_cl,
                                             const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_pandas_play_call_stream_init);
  fm_call_def_destroy_set(def, fm_comp_pandas_play_call_stream_destroy);
  fm_call_def_exec_set(def, fm_comp_pandas_play_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_pandas_play_gen(fm_comp_sys_t *csys,
                                      fm_comp_def_cl closure, unsigned argc,
                                      fm_type_decl_cp argv[],
                                      fm_type_decl_cp ptype,
                                      fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 0) {
    const char *errstr = "no input features should be provided.";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto error = [&]() {
    const char *errstr = "expect a pandas dataframe and result frame type "
                         "tuple";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!ptype)
    return error();

  if (!fm_type_is_tuple(ptype))
    return error();
  if (fm_type_tuple_size(ptype) != 2)
    return error();

  auto rec_t = fm_record_type_get(sys, "PyObject*", sizeof(PyObject *));

  auto *param1 = fm_type_tuple_arg(ptype, 0);
  if (!fm_type_is_record(param1) || !fm_type_equal(rec_t, param1))
    return error();

  PyObject *pd_df = STACK_POP(plist, PyObject *);
  auto *row_descs = fm_type_tuple_arg(ptype, 1);
  if (!fm_type_is_tuple(row_descs))
    return error();
  auto size = fm_type_tuple_size(row_descs);

  vector<const char *> names(size);
  vector<fm_type_decl_cp> types(size);
  int dims[1] = {1};

  auto field_error = [sys](size_t field_idx, const char *str) {
    string errstr = str;
    errstr.append(" for field ");
    errstr.append(to_string(field_idx));
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr.c_str());
    return nullptr;
  };

  for (unsigned i = 0; i < size; ++i) {
    auto *row_desc = fm_type_tuple_arg(row_descs, i);
    auto field_tuple_size = fm_type_tuple_size(row_desc);
    if (field_tuple_size != 2) {
      string errstr = "invalid field description size ";
      errstr.append(to_string(field_tuple_size));
      errstr.append("; expected 2");
      return field_error(i, errstr.c_str());
    };

    if (!fm_type_is_cstring(fm_type_tuple_arg(row_desc, 0))) {
      return field_error(i, "first element of field description tuple "
                            "must be the field name");
    };
    names[i] = STACK_POP(plist, const char *);

    if (!fm_type_is_type(fm_type_tuple_arg(row_desc, 1))) {
      return field_error(i, "second element of field description tuple "
                            "must be of type type");
    };
    types[i] = STACK_POP(plist, fm_type_decl_cp);

    if (!fm_type_is_simple(types[i])) {
      auto *typestr = fm_type_to_str(types[i]);
      auto errstr = string("expect simple type, got: ") + typestr;
      free(typestr);
      return field_error(i, errstr.c_str());
    }
  }

  auto *type =
      fm_frame_type_get1(sys, size, names.data(), types.data(), 1, dims);
  if (!type) {
    const char *errstr = "unable to generate type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto *ctx_cl = new pandas_play_info(csys, pd_df);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_pandas_play_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_pandas_play_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (pandas_play_info *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}

const fm_comp_def_t fm_comp_pandas_play = {"pandas_play",
                                           &fm_comp_pandas_play_gen,
                                           &fm_comp_pandas_play_destroy, NULL};
