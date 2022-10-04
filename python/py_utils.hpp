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
 * @file py_graph.hpp
 * @author Maxim Trokhimtchouk
 * @date 5 Oct 2017
 * @brief Python extension for extractor library
 *
 * This file contains Python C extention for extractor library
 */

#pragma once

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/module.h"
}

#include "py_types.hpp"

#include "python/py_wrapper.hpp"
#include "extractor/type_sys.h"
#include <fmc++/mpl.hpp>

#include <cassert>
#include <errno.h>
#include <functional>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include <fmc++/memory.hpp>
#include <fmc++/strings.hpp>
#include <numpy/arrayobject.h>

#include "extractor/time64.hpp"
#include <Python.h>
#include <datetime.h>
#include <variant>

char *strclone(const char *s) {
  auto len = strlen(s) + 1;
  auto *n = malloc(len);
  memcpy(n, s, len);
  return (char *)n;
}

// mapping to the stack
#define MAX_EXTRACTOR_INPUT 1024
constexpr size_t MAX_EXTRACTOR_STACK_SIZE = 1024;

using comp_array =
    std::variant<std::vector<fm_comp_t *>, std::vector<fm_module_comp_t *>>;

static int python_to_stack_arg(fm_type_sys_t *tsys, PyObject *obj,
                               comp_array &inputs, fm_arg_stack_t *&s,
                               fm_type_decl_cp *type);

namespace fm {
using namespace std;
using namespace python;
using namespace chrono;

using df_column_check = function<bool(object, fm_call_ctx_t *)>;
using py_field_parse = function<bool(object, fm_frame_t *, fm_call_ctx_t *)>;
using df_type_check = function<bool(int)>;
using py_field_conv = function<bool(void *, PyObject *)>;

df_type_check get_df_type_checker(fm_type_decl_cp decl) {
  if (fm_type_is_base(decl)) {
    auto integer = [](int fdtype) {
      return fdtype == NPY_UINT64 || fdtype == NPY_UINT16 ||
             fdtype == NPY_UINT32 || fdtype == NPY_UINT8 ||
             fdtype == NPY_INT64 || fdtype == NPY_INT16 ||
             fdtype == NPY_INT32 || fdtype == NPY_INT8;
    };
    auto en = fm_type_base_enum(decl);
    switch (en) {
    case FM_TYPE_INT8:
    case FM_TYPE_INT16:
    case FM_TYPE_INT32:
    case FM_TYPE_INT64:
    case FM_TYPE_UINT8:
    case FM_TYPE_UINT16:
    case FM_TYPE_UINT32:
    case FM_TYPE_UINT64:
      return integer;
      break;
    case FM_TYPE_FLOAT32:
      return [](int fdtype) { return fdtype == NPY_FLOAT32; };
      break;
    case FM_TYPE_FLOAT64:
      return [](int fdtype) {
        return fdtype == NPY_FLOAT64 || fdtype == NPY_FLOAT32;
      };
      break;
    case FM_TYPE_DECIMAL64:
      return [](int fdtype) {
        return fdtype == NPY_FLOAT64 || fdtype == NPY_FLOAT32;
      };
      break;
    case FM_TYPE_TIME64:
      return [](int fdtype) { return fdtype == NPY_DATETIME; };
      break;
    case FM_TYPE_BOOL:
      return [](int fdtype) { return fdtype == NPY_BOOL; };
      break;
    case FM_TYPE_CHAR:
      return [](int fdtype) { return fdtype == NPY_BYTE; };
      break;
    case FM_TYPE_WCHAR:
      return [](int fdtype) { return fdtype == NPY_UNICODE; };
      break;
    case FM_TYPE_LAST:
    default:
      return df_type_check();
    }
  } else if (fm_type_is_array(decl) &&
             fm_type_is_base(fm_type_array_of(decl)) &&
             fm_type_base_enum(fm_type_array_of(decl)) == FM_TYPE_CHAR) {
    return [](int fdtype) { return fdtype == NPY_OBJECT; };
  }
  return df_type_check();
}

df_column_check get_df_column_check(string col, fm_type_decl_cp decl) {
  auto checker = get_df_type_checker(decl);
  if (!checker) {
    return df_column_check();
  }
  auto tp_name = fmc::autofree<char>{fm_type_to_str(decl)};
  return [col, tp_name = std::string(&*tp_name), checker](object dtypes,
                                                          fm_call_ctx_t *ctx) {
    auto name = col.c_str();
    auto dt = dtypes[col];
    auto py_obj = (PyArray_Descr *)dt.get_ref();
    if (!py_obj) {
      fm_exec_ctx_error_set(ctx->exec, "DataFrame does not have column %s",
                            name);
      return false;
    }
    if (!PyArray_DescrCheck(py_obj)) {
      fm_exec_ctx_error_set(
          ctx->exec, "something is wrong with dtype for column %s", name);
      return false;
    }
    if (!checker(py_obj->type_num)) {
      fm_exec_ctx_error_set(ctx->exec, "cannot convert %s to %s for column %s",
                            py_obj->typeobj->tp_name, tp_name.c_str(), name);
      return false;
    }
    return true;
  };
}

py_field_conv get_py_field_converter(fm_type_decl_cp decl) {
  if (fm_type_is_base(decl)) {
    switch (fm_type_base_enum(decl)) {
    case FM_TYPE_INT8:
      return [](void *ptr, PyObject *obj) {
        *(INT8 *)ptr = PyLong_AsLong(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_INT16:
      return [](void *ptr, PyObject *obj) {
        *(INT16 *)ptr = PyLong_AsLong(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_INT32:
      return [](void *ptr, PyObject *obj) {
        *(INT32 *)ptr = PyLong_AsLong(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_INT64:
      return [](void *ptr, PyObject *obj) {
        *(INT64 *)ptr = PyLong_AsLong(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_UINT8:
      return [](void *ptr, PyObject *obj) {
        *(UINT8 *)ptr = PyLong_AsUnsignedLong(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_UINT16:
      return [](void *ptr, PyObject *obj) {
        *(UINT16 *)ptr = PyLong_AsUnsignedLong(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_UINT32:
      return [](void *ptr, PyObject *obj) {
        *(UINT32 *)ptr = PyLong_AsUnsignedLong(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_UINT64:
      return [](void *ptr, PyObject *obj) {
        *(UINT64 *)ptr = PyLong_AsUnsignedLong(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_FLOAT32:
      return [](void *ptr, PyObject *obj) {
        *(FLOAT32 *)ptr = PyFloat_AsDouble(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_FLOAT64:
      return [](void *ptr, PyObject *obj) {
        *(FLOAT64 *)ptr = PyFloat_AsDouble(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_DECIMAL64:
      return [](void *ptr, PyObject *obj) {
        *(DECIMAL64 *)ptr = fm_decimal64_from_double(PyFloat_AsDouble(obj));
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;

    case FM_TYPE_CHAR:
      return [](void *ptr, PyObject *obj) {
        Py_ssize_t size;
        *(CHAR *)ptr = *PyUnicode_AsUTF8AndSize(obj, &size);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_WCHAR:
      return [](void *ptr, PyObject *obj) {
        Py_ssize_t size;
        *(WCHAR *)ptr = *PyUnicode_AsWideCharString(obj, &size);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;

    case FM_TYPE_TIME64:
      return [](void *ptr, PyObject *obj) {
        if (!PyDelta_Check(obj))
          return false;
        auto h = duration_cast<nanoseconds>(
            hours(24 * PyLong_AsLong(PyObject_GetAttrString(obj, "days"))));
        auto sec = duration_cast<nanoseconds>(
            (seconds(PyLong_AsLong(PyObject_GetAttrString(obj, "seconds")))));
        auto us = duration_cast<nanoseconds>((microseconds(
            PyLong_AsLong(PyObject_GetAttrString(obj, "microseconds")))));
        auto tm = fm_time64_from_nanos((h + sec + us).count());
        if (PyErr_Occurred())
          return false;
        *(TIME64 *)ptr = tm;
        return true;
      };
      break;
    case FM_TYPE_BOOL:
      return [](void *ptr, PyObject *obj) {
        if (!PyBool_Check(obj))
          return false;
        *(BOOL *)ptr = (BOOL)PyObject_IsTrue(obj);
        if (PyErr_Occurred())
          return false;
        return true;
      };
      break;
    case FM_TYPE_LAST:
    default:
      return py_field_conv();
    }
  } else if (fm_type_is_array(decl)) {
    if (!fm_type_is_base(fm_type_array_of(decl))) {
      PyErr_SetString(PyExc_RuntimeError, "Type of array is not base type");
      return [](void *ptr, PyObject *obj) { return false; };
    }
    if (fm_type_base_enum(fm_type_array_of(decl)) != FM_TYPE_CHAR) {
      string mesage = string("Unsupported base type of array: ") +
                      to_string(fm_type_base_enum(fm_type_array_of(decl)));
      PyErr_SetString(PyExc_RuntimeError, mesage.data());
      return [](void *ptr, PyObject *obj) { return false; };
    }
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
        return true;
      } else if (PyUnicode_Check(obj)) {
        Py_ssize_t size;
        auto *data = PyUnicode_AsUTF8AndSize(obj, &size);
        if (!data)
          return false;
        if (size > (int)sz)
          return false;
        memset(ptr, '\0', sz);
        memcpy(ptr, data, size);
        return true;
      }
      return false;
    };
  }
  return py_field_conv();
}

PyObject *get_py_obj_from_ptr(fm_type_decl_cp decl, const void *ptr) {
  if (fm_type_is_base(decl)) {
    switch (fm_type_base_enum(decl)) {
    case FM_TYPE_INT8:
      return PyLong_FromLong(*(INT8 *)ptr);
    case FM_TYPE_INT16:
      return PyLong_FromLong(*(INT16 *)ptr);
      break;
    case FM_TYPE_INT32:
      return PyLong_FromLong(*(INT32 *)ptr);
      break;
    case FM_TYPE_INT64:
      return PyLong_FromLongLong(*(INT64 *)ptr);
      break;
    case FM_TYPE_UINT8:
      return PyLong_FromUnsignedLong(*(INT8 *)ptr);
      break;
    case FM_TYPE_UINT16:
      return PyLong_FromUnsignedLong(*(INT16 *)ptr);
      break;
    case FM_TYPE_UINT32:
      return PyLong_FromUnsignedLong(*(INT32 *)ptr);
      break;
    case FM_TYPE_UINT64:
      return PyLong_FromUnsignedLongLong(*(INT64 *)ptr);
      break;
    case FM_TYPE_FLOAT32:
      return PyFloat_FromDouble(*(FLOAT32 *)ptr);
      break;
    case FM_TYPE_FLOAT64:
      return PyFloat_FromDouble(*(FLOAT64 *)ptr);
      break;
    case FM_TYPE_DECIMAL64:
      return PyFloat_FromDouble(fm_decimal64_to_double(*(DECIMAL64 *)ptr));
      break;
    case FM_TYPE_CHAR:
      return PyUnicode_FromStringAndSize((const char *)ptr, 1);
      break;
    case FM_TYPE_WCHAR:
      return PyUnicode_FromWideChar((const wchar_t *)ptr, 1);
      break;
    case FM_TYPE_TIME64: {
      using days = typename chrono::duration<long int, std::ratio<86400>>;
      auto t = nanoseconds(fm_time64_to_nanos(*(TIME64 *)ptr));
      auto d = duration_cast<days>(t);
      auto us = duration_cast<microseconds>(t - d);
      auto sec = duration_cast<seconds>(us);
      auto tmp = duration_cast<microseconds>(sec);
      auto rem = us - tmp;
      return PyDelta_FromDSU(d.count(), sec.count(), rem.count());
    } break;
    case FM_TYPE_BOOL:
      if (*(BOOL *)ptr)
        Py_RETURN_TRUE;
      Py_RETURN_FALSE;
      break;
    case FM_TYPE_LAST:
    default:
      string mesage = string("Unsupported base type: ") +
                      to_string(fm_type_base_enum(decl));
      PyErr_SetString(PyExc_RuntimeError, mesage.data());
      return nullptr;
    }
  } else if (fm_type_is_array(decl)) {
    if (!fm_type_is_base(fm_type_array_of(decl))) {
      PyErr_SetString(PyExc_RuntimeError, "Type of array is not base type");
      return nullptr;
    }
    if (fm_type_base_enum(fm_type_array_of(decl)) != FM_TYPE_CHAR) {
      string mesage = string("Unsupported base type of array: ") +
                      to_string(fm_type_base_enum(fm_type_array_of(decl)));
      PyErr_SetString(PyExc_RuntimeError, mesage.data());
      return nullptr;
    }
    auto sz = fm_type_array_size(decl);
    auto str_size = strnlen((const char *)ptr, sz);
    return PyUnicode_FromStringAndSize((const char *)ptr, str_size);
  }
  PyErr_SetString(PyExc_RuntimeError, "Unsupported object type");
  return nullptr;
}

PyObject *get_py_obj_from_arg_stack(fm_type_decl_cp decl,
                                    fm_arg_stack_t &plist) {
  if (fm_type_is_base(decl)) {
    switch (fm_type_base_enum(decl)) {
    case FM_TYPE_INT8:
      return PyLong_FromLongLong(STACK_POP(plist, INT8));
    case FM_TYPE_INT16:
      return PyLong_FromLongLong(STACK_POP(plist, INT16));
      break;
    case FM_TYPE_INT32:
      return PyLong_FromLongLong(STACK_POP(plist, INT32));
      break;
    case FM_TYPE_INT64:
      return PyLong_FromLongLong(STACK_POP(plist, INT64));
      break;
    case FM_TYPE_UINT8:
      return PyLong_FromUnsignedLongLong(STACK_POP(plist, UINT8));
      break;
    case FM_TYPE_UINT16:
      return PyLong_FromUnsignedLongLong(STACK_POP(plist, UINT16));
      break;
    case FM_TYPE_UINT32:
      return PyLong_FromUnsignedLongLong(STACK_POP(plist, UINT32));
      break;
    case FM_TYPE_UINT64:
      return PyLong_FromUnsignedLongLong(STACK_POP(plist, UINT64));
      break;
    case FM_TYPE_FLOAT32:
      return PyFloat_FromDouble(STACK_POP(plist, FLOAT32));
      break;
    case FM_TYPE_FLOAT64:
      return PyFloat_FromDouble(STACK_POP(plist, FLOAT64));
      break;
    case FM_TYPE_CHAR:
      return PyUnicode_FromStringAndSize(&STACK_POP(plist, CHAR), 1);
      break;
    case FM_TYPE_WCHAR:
      return PyUnicode_FromWideChar(&STACK_POP(plist, WCHAR), 1);
      break;
    case FM_TYPE_DECIMAL64:
      return PyFloat_FromDouble(
          fm_decimal64_to_double(STACK_POP(plist, DECIMAL64)));
      break;
    case FM_TYPE_TIME64: {
      using days = typename chrono::duration<long int, std::ratio<86400>>;
      auto t = nanoseconds(fm_time64_to_nanos(STACK_POP(plist, TIME64)));
      auto d = duration_cast<days>(t);
      auto us = duration_cast<microseconds>(t - d);
      auto sec = duration_cast<seconds>(us);
      auto tmp = duration_cast<microseconds>(sec);
      auto rem = us - tmp;
      return PyDelta_FromDSU(d.count(), sec.count(), rem.count());
    } break;
    case FM_TYPE_BOOL:
      if (STACK_POP(plist, BOOL))
        Py_RETURN_TRUE;
      Py_RETURN_FALSE;
      break;
    case FM_TYPE_LAST:
    default:
      string mesage = string("Unsupported base type: ") +
                      to_string(fm_type_base_enum(decl));
      PyErr_SetString(PyExc_RuntimeError, mesage.data());
      return nullptr;
    }
  } else if (fm_type_is_array(decl)) {
    if (!fm_type_is_base(fm_type_array_of(decl))) {
      PyErr_SetString(PyExc_RuntimeError, "Type of array is not base type");
      return nullptr;
    }
    if (fm_type_base_enum(fm_type_array_of(decl)) != FM_TYPE_CHAR) {
      string mesage = string("Unsupported base type of array: ") +
                      to_string(fm_type_base_enum(fm_type_array_of(decl)));
      PyErr_SetString(PyExc_RuntimeError, mesage.data());
      return nullptr;
    }
    auto sz = fm_type_array_size(decl);
    return PyUnicode_FromStringAndSize(STACK_POP(plist, const char *), sz);
  } else if (fm_type_is_cstring(decl)) {
    return PyUnicode_FromString(STACK_POP(plist, const char *));
  } else if (fm_type_is_tuple(decl)) {
    auto args_sz = fm_type_tuple_size(decl);
    auto args = PyTuple_New(args_sz);
    for (unsigned i = 0; i < args_sz; ++i) {
      auto sz = fm_type_array_size(decl);
      auto arg_type = fm_type_tuple_arg(decl, i);
      PyTuple_SET_ITEM(args, i, get_py_obj_from_arg_stack(arg_type, plist));
    }
    return args;
  }
  PyErr_SetString(PyExc_RuntimeError, "Unsupported object type");
  return nullptr;
}

py_field_parse get_df_column_parse(string col, fm_type_decl_cp decl,
                                   fm_field_t idx) {
  auto convert = get_py_field_converter(decl);
  if (idx == -1 || !convert) {
    return py_field_parse();
  }

  auto tp_name = fmc::autofree<char>{fm_type_to_str(decl)};
  return [col, idx, convert, tp_name = std::string(&*tp_name)](
             object row, fm_frame_t *result, fm_call_ctx_t *ctx) {
    auto *name = col.c_str();
    auto obj = row[name];
    if (!obj) {
      fm_exec_ctx_error_set(ctx->exec, "could not obtain column %s from row",
                            name);
      return false;
    }
    if (!convert(fm_frame_get_ptr1(result, idx, 0), obj.get_ref())) {
      auto *py_tp = obj.str().c_str();
      fm_exec_ctx_error_set(ctx->exec,
                            "could not convert %s to %s for column %s", py_tp,
                            tp_name.c_str(), name);
      return false;
    }
    return true;
  };
}

inline int dim_from_key(const fm_frame_t *frame, PyObject *key) {
  auto key_field = -1;

  if (fm_frame_ndims(frame) != 1) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Access is only supported for frames with one dimension.");
    return key_field;
  }

  auto key_from_long = [&key_field, &frame](PyObject *k) {
    key_field = PyLong_AsLong(k);
    if (!PyErr_Occurred()) {
      auto frame_dim = fm_frame_dim(frame, 0);
      if (key_field >= frame_dim || key_field < -frame_dim) {
        PyErr_SetString(PyExc_RuntimeError, "Invalid dimension");
      } else if (key_field < 0) {
        key_field = frame_dim + key_field;
      }
    }
  };

  if (PyLong_Check(key)) {
    key_from_long(key);
  } else if (PyTuple_Check(key)) {
    if (PyTuple_Size(key) != 1) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Invalid tuple size, access "
                      "is only supported for frames with one dimension.");
    } else {
      auto *data = PyTuple_GetItem(key, 0);
      if (!PyLong_Check(data)) {
        PyErr_SetString(PyExc_RuntimeError,
                        "Invalid index type. Expected long");
      } else {
        key_from_long(data);
      }
    }
  } else {
    PyErr_SetString(PyExc_RuntimeError, "Invalid key type");
  }

  return key_field;
}

template <typename S, typename T> void set_python_error(S obj, T clbl) {
  if (PyErr_Occurred() != nullptr) {
    PyObject *ptype;
    PyObject *pvalue;
    PyObject *ptraceback;
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    object type = object::from_new(ptype);
    object value = object::from_new(pvalue);
    object traceback = object::from_new(ptraceback);
    std::string errstr;

    if (bool(traceback)) {
      auto strobj = object::from_new(PyUnicode_FromString("traceback"));
      if (auto mod = object::from_new(PyImport_Import(strobj.get_ref())); mod) {
        if (auto list = mod["format_tb"](traceback); list) {
          if (auto iter = list.iter(); iter) {
            while (true) {
              auto obj = iter.next();
              if (!obj)
                break;
              errstr += PyUnicode_AsUTF8(obj.get_ref());
            }
          }
        }
      }
    }
    auto stack_str =
        errstr.empty() ? "could not obtain the stack" : errstr.c_str();
    clbl(obj, "Python error:\n%s\n%s", value.str().c_str(), stack_str);
    PyErr_Restore(type.steal_ref(), value.steal_ref(), traceback.steal_ref());
  }
}

void set_python_error(fm_exec_ctx_t *exec) {
  set_python_error(exec, fm_exec_ctx_error_set);
}

void set_python_error(fm_comp_sys_t *sys) {
  set_python_error(sys, fm_comp_sys_error_set);
}

PyObject *gen_array(FM_BASE_TYPE fm_type, int type, int ndims, npy_intp *f_dims,
                    unsigned elem_size) {
  PyObject *array;
  if (fm_type == FM_TYPE_TIME64) {
    PyObject *date_type = Py_BuildValue("s", "M8[ns]");
    PyArray_Descr *descr;
    PyArray_DescrConverter(date_type, &descr);
    Py_XDECREF(date_type);
    array = PyArray_SimpleNewFromDescr(ndims, f_dims, descr);
  } else if (fm_type == FM_TYPE_CHAR) {
    auto np = fm::python::object::from_new(PyImport_ImportModule("numpy"));
    if (!np) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to import numpy");
      return nullptr;
    }
    auto type = np["dtype"](fm::python::object::from_new(PyUnicode_FromString(
        (string("S") + to_string(elem_size / sizeof(char))).c_str())));
    PyArray_Descr *descr;
    PyArray_DescrConverter(type.steal_ref(), &descr);
    array = PyArray_SimpleNewFromDescr(ndims, f_dims, descr);
  } else
    array = PyArray_SimpleNew(ndims, f_dims, type);

  PyArray_ENABLEFLAGS((PyArrayObject *)array, NPY_ARRAY_OWNDATA);
  return array;
}

PyObject *result_as_pandas(const fm_frame_t *frame,
                           const char *index = nullptr) {
  auto f_decl = fm_frame_type(frame);

  int nf = fm_type_frame_nfields(f_decl);

  if (fm_frame_ndims(frame) != 1) {
    PyErr_SetString(PyExc_RuntimeError, "Extractor frame to pandas conversion "
                                        "is only supported for one dimensional"
                                        " frames");
    return nullptr;
  }

  PyObject *df_arg = nullptr;
  PyObject *ret = nullptr;

  auto *dict = PyDict_New();

  npy_intp f_dims[1] = {fm_frame_dim(frame, 0)};

  auto cleanup = [dict, df_arg, ret]() {
    Py_XDECREF(dict);
    if (df_arg)
      Py_XDECREF(df_arg);
    if (ret)
      Py_XDECREF(ret);
  };

  for (int i = 0; i < nf; ++i) {
    auto type = NPY_NOTYPE;
    auto fm_type = FM_TYPE_INT8;
    auto elem_size = 0;
    auto decl = fm_type_frame_field_type(f_decl, i);
    if (fm_type_is_base(decl)) {
      fm_type = fm_type_base_enum(decl);
      switch (fm_type) {
      case FM_TYPE_INT8:
        type = NPY_INT8;
        elem_size = sizeof(int8_t);
        break;
      case FM_TYPE_INT16:
        type = NPY_INT16;
        elem_size = sizeof(int16_t);
        break;
      case FM_TYPE_INT32:
        type = NPY_INT32;
        elem_size = sizeof(int32_t);
        break;
      case FM_TYPE_INT64:
        type = NPY_INT64;
        elem_size = sizeof(int64_t);
        break;
      case FM_TYPE_UINT8:
        type = NPY_UINT8;
        elem_size = sizeof(uint8_t);
        break;
      case FM_TYPE_UINT16:
        type = NPY_UINT16;
        elem_size = sizeof(uint16_t);
        break;
      case FM_TYPE_UINT32:
        type = NPY_UINT32;
        elem_size = sizeof(uint32_t);
        break;
      case FM_TYPE_UINT64:
        type = NPY_UINT64;
        elem_size = sizeof(uint64_t);
        break;
      case FM_TYPE_FLOAT32:
        type = NPY_FLOAT32;
        elem_size = sizeof(float);
        break;
      case FM_TYPE_FLOAT64:
        type = NPY_FLOAT64;
        elem_size = sizeof(double);
        break;
      case FM_TYPE_CHAR:
        type = NPY_BYTE;
        elem_size = sizeof(char);
        break;
      case FM_TYPE_WCHAR:
        type = NPY_UNICODE;
        elem_size = sizeof(WCHAR);
        break;
      case FM_TYPE_BOOL:
        type = NPY_BOOL;
        elem_size = sizeof(bool);
        break;
      case FM_TYPE_RATIONAL64:
        type = NPY_FLOAT64;
        elem_size = sizeof(double);
        break;
      case FM_TYPE_DECIMAL64:
        type = NPY_INT64;
        elem_size = sizeof(int64_t);
        break;
      case FM_TYPE_TIME64:
        type = NPY_DATETIME;
        elem_size = sizeof(int64_t);
      case FM_TYPE_LAST:
        break;
      }
    } else if (fm_type_is_array(decl)) {
      auto f_type = fm_type_array_of(decl);
      auto size = fm_type_array_size(decl);
      if (fm_type_is_base(f_type)) {
        fm_type = fm_type_base_enum(f_type);
        switch (fm_type) {
        case FM_TYPE_CHAR:
          type = NPY_BYTE;
          elem_size = size * sizeof(char);
          break;
        case FM_TYPE_WCHAR:
          type = NPY_UNICODE;
          elem_size = size * sizeof(WCHAR);
          break;
        default:
          break;
        }
      }
    }

    if (type == NPY_NOTYPE) {
      PyErr_SetString(PyExc_TypeError, "Invalid data type in frame");
      cleanup();
      return nullptr;
    }

    PyObject *array = gen_array(fm_type, type, 1, f_dims, elem_size);

    if (!array) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to create array");
      cleanup();
      return nullptr;
    }

    if (fm_type_base_enum(decl) == FM_TYPE_RATIONAL64) {
      for (int item = 0; item < f_dims[0]; ++item) {
        *(double *)PyArray_GETPTR1((PyArrayObject *)array, item) =
            fm_rational64_to_double(
                *(fm_rational64_t *)fm_frame_get_cptr1(frame, i, item));
      }
    } else if (fm_type_base_enum(decl) == FM_TYPE_BOOL) {
      for (int item = 0; item < f_dims[0]; ++item) {
        *(char *)PyArray_GETPTR1((PyArrayObject *)array, item) =
            char(*(BOOL *)fm_frame_get_cptr1(frame, i, item));
      }
    } else {
      memcpy(PyArray_GETPTR1((PyArrayObject *)array, 0),
             fm_frame_get_cptr1(frame, i, 0), elem_size * f_dims[0]);
    }

    auto np = fm::python::object::from_new(PyImport_ImportModule("numpy"));
    if (!np) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to import numpy");
      cleanup();
      return nullptr;
    }

    if (fm_type_base_enum(decl) == FM_TYPE_DECIMAL64) {
      // @note Module is imported directly because numpy division
      // is not available in C_API

      auto tmp_array = array;
      array = PyObject_CallMethod(tmp_array, "astype", "s", "float");
      Py_XDECREF(tmp_array);
      if (!array) {
        PyErr_SetString(PyExc_RuntimeError, "Unable to change type of "
                                            "array to float");
        cleanup();
        return nullptr;
      }
      tmp_array = array;
      array = PyObject_CallMethod(np.get_ref(), "divide", "Od", tmp_array,
                                  double(DECIMAL64_FRACTION));
      Py_XDECREF(tmp_array);
      if (!array) {
        PyErr_SetString(PyExc_RuntimeError, "Unable to divide by "
                                            "conversion factor");
        cleanup();
        return nullptr;
      }
    }

    if (fm_type == FM_TYPE_CHAR) {
      auto tmp_array = np["char"]["decode"](
          fm::python::object::from_borrowed(array),
          fm::python::object::from_new(PyUnicode_FromString("UTF-8")));
      if (!tmp_array) {
        if (!PyErr_Occurred()) {
          PyErr_SetString(PyExc_RuntimeError, "Unable to decode the "
                                              "string array");
        }
        cleanup();
        return nullptr;
      }
      Py_XDECREF(array);
      array = tmp_array.steal_ref();
    }

    auto dict_set =
        PyDict_SetItemString(dict, fm_type_frame_field_name(f_decl, i), array);
    Py_XDECREF(array);

    if (dict_set) {
      PyErr_SetString(PyExc_RuntimeError, "Error setting value in "
                                          "dictionary");
      cleanup();
      return nullptr;
    }
  }

  df_arg = PyTuple_New(1);
  if (!df_arg) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to create tuple for pandas "
                                        "argument");
    cleanup();
    return nullptr;
  }

  PyTuple_SET_ITEM(df_arg, 0, dict);

  PyObject *pandas = PyImport_ImportModule("pandas");

  if (!pandas) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to import pandas");
    cleanup();
    return nullptr;
  }

  PyObject *df = PyObject_GetAttrString(pandas, "DataFrame");

  Py_XDECREF(pandas);

  if (!df) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to get DataFrame");
    cleanup();
    return nullptr;
  }

  ret = PyObject_CallObject(df, df_arg);

  Py_XDECREF(df);
  Py_XDECREF(df_arg);

  if (!ret) {
    if (!PyErr_Occurred()) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to create pandas "
                                          "dataframe");
    }
    cleanup();
    return nullptr;
  }

  if (!index)
    return ret;

  auto tmp_ret = ret;

  ret = PyObject_CallMethod(tmp_ret, "set_index", "s", index);
  Py_XDECREF(tmp_ret);

  if (!ret) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to set index");
    cleanup();
    return nullptr;
  }

  return ret;
}

inline short type_size(fm_type_decl_cp decl) {
  if (fm_type_is_base(decl)) {
    switch (fm_type_base_enum(decl)) {
    case FM_TYPE_INT8:
      return 4;
    case FM_TYPE_INT16:
      return 6;
      break;
    case FM_TYPE_INT32:
      return 14;
      break;
    case FM_TYPE_INT64:
      return 21;
      break;
    case FM_TYPE_UINT8:
      return 4;
      break;
    case FM_TYPE_UINT16:
      return 6;
      break;
    case FM_TYPE_UINT32:
      return 14;
      break;
    case FM_TYPE_UINT64:
      return 21;
      break;
    case FM_TYPE_FLOAT32:
      return 20;
      break;
    case FM_TYPE_FLOAT64:
      return 20;
      break;
    case FM_TYPE_DECIMAL64:
      return 20;
      break;
    case FM_TYPE_CHAR:
      return 1;
      break;
    case FM_TYPE_TIME64: {
      return 30;
    } break;
    case FM_TYPE_BOOL:
      return 5;
      break;
    case FM_TYPE_LAST:
    default:
      return 4;
    }
  } else if (fm_type_is_array(decl)) {
    if (!fm_type_is_base(fm_type_array_of(decl))) {
      return 4;
    }
    if (fm_type_base_enum(fm_type_array_of(decl)) != FM_TYPE_CHAR) {
      return 4;
    }

    return 10;
  }
  return 4;
}

string ptr_to_str(fm_type_decl_cp decl, const void *ptr) {
  if (fm_type_is_base(decl)) {
    switch (fm_type_base_enum(decl)) {
    case FM_TYPE_INT8:
      return to_string(*(INT8 *)ptr);
    case FM_TYPE_INT16:
      return to_string(*(INT16 *)ptr);
      break;
    case FM_TYPE_INT32:
      return to_string(*(INT32 *)ptr);
      break;
    case FM_TYPE_INT64:
      return to_string(*(INT64 *)ptr);
      break;
    case FM_TYPE_UINT8:
      return to_string(*(UINT8 *)ptr);
      break;
    case FM_TYPE_UINT16:
      return to_string(*(UINT16 *)ptr);
      break;
    case FM_TYPE_UINT32:
      return to_string(*(UINT32 *)ptr);
      break;
    case FM_TYPE_UINT64:
      return to_string(*(UINT64 *)ptr);
      break;
    case FM_TYPE_FLOAT32: {
      char buf[20];
      auto view = fmc::to_string_view_double(buf, *(FLOAT32 *)ptr, 9);
      return string(view.data(), view.size());
    } break;
    case FM_TYPE_FLOAT64: {
      char buf[20];
      auto view = fmc::to_string_view_double(buf, *(FLOAT64 *)ptr, 9);
      return string(view.data(), view.size());
    } break;
    case FM_TYPE_DECIMAL64: {
      char buf[20];
      auto view = fmc::to_string_view_double(
          buf, fm_decimal64_to_double(*(DECIMAL64 *)ptr), 9);
      return string(view.data(), view.size());
    } break;
    case FM_TYPE_CHAR:
      return string((const char *)ptr, 1);
      break;
    case FM_TYPE_TIME64: {
      using namespace std;
      using namespace chrono;

      stringstream s;
      auto x = std::chrono::nanoseconds(fm_time64_to_nanos(*(TIME64 *)ptr));
      auto epoch = time_point<system_clock>(
          duration_cast<time_point<system_clock>::duration>(x));
      auto t = system_clock::to_time_t(epoch);
      auto tm = *gmtime(&t);
      s << put_time(&tm, "%F %T") << '.' << setw(9) << setfill('0')
        << (x % seconds(1)).count();
      return s.str();
    } break;
    case FM_TYPE_BOOL:
      if (*(BOOL *)ptr) {
        return "True";
      } else {
        return "False";
      }
      break;
    case FM_TYPE_LAST:
    default:
      return "";
    }
  } else if (fm_type_is_array(decl)) {
    if (!fm_type_is_base(fm_type_array_of(decl))) {
      return "";
    }
    if (fm_type_base_enum(fm_type_array_of(decl)) != FM_TYPE_CHAR) {
      return "";
    }
    auto sz = fm_type_array_size(decl);
    auto str_size = strnlen((const char *)ptr, sz);
    return string((const char *)ptr, str_size);
  }
  return "";
}

} // namespace fm
