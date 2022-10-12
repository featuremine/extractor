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
 * @file tuple_msg.hpp
 * @author Maxim Trokhimtchouk
 * @date 27 Mar 2019
 * @brief File contains C++ definitions of the tuple_msg
 *
 * tuple_msg extractor operator generates an update from a named tuple
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

#include "extractor/type_sys.h"
#include "fmc++/mpl.hpp"
#include "py_utils.hpp"
#include "py_wrapper.hpp"

#include <cassert>
#include <errno.h>
#include <functional>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include "fmc++/strings.hpp"
#include <numpy/arrayobject.h>

using namespace fm;
using namespace python;
using namespace std;

py_field_conv get_py_field_checked_converter(fm_type_decl_cp decl) {
  if (fm_type_is_base(decl)) {
    switch (fm_type_base_enum(decl)) {
    case FM_TYPE_INT8:
      return [](void *ptr, PyObject *obj) {
        if (!PyLong_Check(obj)) {
          return false;
        }
        *(INT8 *)ptr = PyLong_AsLong(obj);
        return true;
      };
      break;
    case FM_TYPE_INT16:
      return [](void *ptr, PyObject *obj) {
        if (!PyLong_Check(obj)) {
          return false;
        }
        *(INT16 *)ptr = PyLong_AsLong(obj);
        return true;
      };
      break;
    case FM_TYPE_INT32:
      return [](void *ptr, PyObject *obj) {
        if (!PyLong_Check(obj)) {
          return false;
        }
        *(INT32 *)ptr = PyLong_AsLong(obj);
        return true;
      };
      break;
    case FM_TYPE_INT64:
      return [](void *ptr, PyObject *obj) {
        if (!PyLong_Check(obj)) {
          return false;
        }
        *(INT64 *)ptr = PyLong_AsLongLong(obj);
        return true;
      };
      break;
    case FM_TYPE_UINT8:
      return [](void *ptr, PyObject *obj) {
        if (!PyLong_Check(obj)) {
          return false;
        }
        *(UINT8 *)ptr = PyLong_AsUnsignedLong(obj);
        return true;
      };
      break;
    case FM_TYPE_UINT16:
      return [](void *ptr, PyObject *obj) {
        if (!PyLong_Check(obj)) {
          return false;
        }
        *(UINT16 *)ptr = PyLong_AsUnsignedLong(obj);
        return true;
      };
      break;
    case FM_TYPE_UINT32:
      return [](void *ptr, PyObject *obj) {
        if (!PyLong_Check(obj)) {
          return false;
        }
        *(UINT32 *)ptr = PyLong_AsUnsignedLong(obj);
        return true;
      };
      break;
    case FM_TYPE_UINT64:
      return [](void *ptr, PyObject *obj) {
        if (!PyLong_Check(obj)) {
          return false;
        }
        *(UINT64 *)ptr = PyLong_AsUnsignedLongLong(obj);
        return true;
      };
      break;
    case FM_TYPE_FLOAT32:
      return [](void *ptr, PyObject *obj) {
        if (!PyFloat_Check(obj)) {
          return false;
        }
        *(FLOAT32 *)ptr = PyFloat_AsDouble(obj);
        return true;
      };
      break;
    case FM_TYPE_FLOAT64:
      return [](void *ptr, PyObject *obj) {
        if (!PyFloat_Check(obj)) {
          return false;
        }
        *(FLOAT64 *)ptr = PyFloat_AsDouble(obj);
        return true;
      };
      break;
    case FM_TYPE_DECIMAL64:
      return [](void *ptr, PyObject *obj) {
        if (!PyFloat_Check(obj)) {
          return false;
        }
        *(DECIMAL64 *)ptr = fm_decimal64_from_double(PyFloat_AsDouble(obj));
        return true;
      };
      break;
    case FM_TYPE_TIME64:
      return [](void *ptr, PyObject *obj) {
        if (PyLong_Check(obj)) {
          *(TIME64 *)ptr = fmc_time64_from_nanos(PyLong_AsLongLong(obj));
          if (PyErr_Occurred()) {
            return false;
          }
          return true;
        }
        auto dt_ob = object::from_new(PyObject_GetAttrString(obj, "value"));
        if (!dt_ob) {
          return false;
        }
        *(TIME64 *)ptr =
            fmc_time64_from_nanos(PyLong_AsLongLong(dt_ob.get_ref()));
        return true;
      };
      break;
    case FM_TYPE_BOOL:
      return [](void *ptr, PyObject *obj) {
        *(BOOL *)ptr = (bool)PyObject_IsTrue(obj);
        return true;
      };
      break;
    case FM_TYPE_LAST:
    default:
      return py_field_conv();
    }
  } else if (fm_type_is_array(decl) &&
             fm_type_is_base(fm_type_array_of(decl)) &&
             fm_type_base_enum(fm_type_array_of(decl)) == FM_TYPE_CHAR) {
    auto sz = fm_type_array_size(decl);
    return [sz](void *ptr, PyObject *obj) {
      if (PyBytes_Check(obj)) {
        Py_buffer buff;
        if (PyObject_GetBuffer(obj, &buff, PyBUF_C_CONTIGUOUS) != 0)
          return false;
        if (buff.len > (int)sz)
          return false;
        memset(ptr, '\0', sz);
        memcpy(ptr, buff.buf, buff.len);
        PyBuffer_Release(&buff);
      } else if (PyUnicode_Check(obj)) {
        Py_ssize_t size;
        auto *data = PyUnicode_AsUTF8AndSize(obj, &size);
        if (!data)
          return false;
        if (size > (int)sz)
          return false;
        memset(ptr, '\0', sz);
        memcpy(ptr, data, size);
      }
      return true;
    };
  }
  return py_field_conv();
}

py_field_parse get_tuple_parse(const string &col, fm_type_decl_cp decl,
                               fm_field_t idx) {
  auto convert = get_py_field_checked_converter(decl);
  if (idx == -1 || !convert) {
    return py_field_parse();
  }
  auto tp_name = fmc::autofree<char>{fm_type_to_str(decl)};
  return [col, idx, convert, tp_name = std::string(&*tp_name)](
             object row, fm_frame_t *result, fm_call_ctx_t *ctx) {
    auto *name = col.c_str();
    auto obj = row[name];
    if (!obj) {
      fm_exec_ctx_error_set(ctx->exec, "could not obtain column %s from row %s",
                            name, row.str().c_str());
      return false;
    }
    if (obj.get_ref() == Py_None)
      return true;
    if (!convert(fm_frame_get_ptr1(result, idx, 0), obj.get_ref())) {
      auto py_tp = obj.str();
      auto py_type = obj.type();
      fm_exec_ctx_error_set(
          ctx->exec, "could not convert %s of type %s to %s for attribute %s",
          py_tp.c_str(), py_type.str().c_str(), tp_name.c_str(), name);
      return false;
    }
    return true;
  };
}

struct namedtuple_parser {
  namedtuple_parser(string class_name, fm_type_decl_cp decl)
      : class_name_(class_name) {
    assert(fm_type_is_frame(decl));
    auto nfields = fm_type_frame_nfields(decl);
    for (auto i = 0u; i < nfields; ++i) {
      auto type = fm_type_frame_field_type(decl, i);
      auto name = fm_type_frame_field_name(decl, i);
      auto idx = fm_type_frame_field_idx(decl, name);
      auto parse = get_tuple_parse(name, type, idx);
      if (!parse) {
        auto tp_name = fmc::autofree<char>{fm_type_to_str(decl)};
        fmc_runtime_error_unless(false) << "could not obtain parser for field "
                                        << name << " of type " << &*tp_name;
      }
      parses_.push_back(parse);
    }
  }
  bool required(object obj, fm_call_ctx_t *ctx) {
    if (!obj)
      return false;
    if (class_name_ != Py_TYPE(obj.get_ref())->tp_name)
      return false;

    return true;
  }
  bool parse(object row, fm_frame_t *result, fm_call_ctx_t *ctx) {
    for (auto &parser : parses_) {
      if (!parser(row, result, ctx)) {
        return false;
      }
    }
    return true;
  }
  string class_name_;
  vector<py_field_parse> parses_;
};

bool fm_comp_tuple_msg_stream_init(fm_frame_t *result, size_t args,
                                   const fm_frame_t *const argv[],
                                   fm_call_ctx_t *call_ctx,
                                   fm_call_exec_cl *cl) {
  auto *parser = (namedtuple_parser *)call_ctx->comp;
  auto obj =
      object::from_borrowed(*(PyObject **)fm_frame_get_cptr1(argv[0], 0, 0));
  if (parser->required(obj, call_ctx)) {
    if (!parser->parse(obj, result, call_ctx)) {
      return false;
    }
  }
  return true;
}

bool fm_comp_tuple_msg_stream_exec(fm_frame_t *result, size_t,
                                   const fm_frame_t *const argv[],
                                   fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *parser = (namedtuple_parser *)ctx->comp;
  auto obj =
      object::from_borrowed(*(PyObject **)fm_frame_get_cptr1(argv[0], 0, 0));
  if (parser->required(obj, ctx)) {
    if (!parser->parse(obj, result, ctx)) {
      return false;
    }
    return true;
  }
  return false;
}

fm_call_def *fm_comp_tuple_msg_stream_call(fm_comp_def_cl comp_cl,
                                           const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_tuple_msg_stream_init);
  fm_call_def_exec_set(def, fm_comp_tuple_msg_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_tuple_msg_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                    unsigned argc, fm_type_decl_cp argv[],
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

  auto error = [&]() {
    const char *errstr = "a class name of an namedtuple to process and a "
                         "tuple describing result frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 2)
    return error();

  auto name = fm_arg_try_cstring(fm_type_tuple_arg(ptype, 0), &plist);
  if (!name) {
    return error();
  }

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

  try {
    auto *cl = new namedtuple_parser(name, type);
    auto *def = fm_ctx_def_new();
    fm_ctx_def_inplace_set(def, false);
    fm_ctx_def_type_set(def, type);
    fm_ctx_def_closure_set(def, cl);
    fm_ctx_def_stream_call_set(def, &fm_comp_tuple_msg_stream_call);
    fm_ctx_def_query_call_set(def, nullptr);
    return def;
  } catch (std::exception &e) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, e.what());
    return nullptr;
  }
}

void fm_comp_tuple_msg_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (namedtuple_parser *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}

const fm_comp_def_t fm_comp_tuple_msg = {"tuple_msg", &fm_comp_tuple_msg_gen,
                                         &fm_comp_tuple_msg_destroy, NULL};
