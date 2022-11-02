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
 * @file py_types.hpp
 * @author Maxim Trokhimtchouk
 * @date 5 Oct 2017
 * @brief Python wrappers for extractor base types
 * */

#pragma once

extern "C" {
#include "extractor/type_decl.h"
}
#include "extractor/comp_def.hpp"
#include "extractor/decimal64.hpp"
#include "extractor/rational64.hpp"
#include "extractor/rprice.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/time.hpp"

#include <Python.h>
#include <datetime.h>
#include <limits>
#include <py_comp_base.hpp>
#include <py_wrapper.hpp>
#include <type_traits>

#include <py_decimal128.hpp>

fm_type_decl_cp fm_type_from_py_type(fm_type_sys_t *tsys, PyObject *obj);

static PyObject *create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyObject *input = NULL;
  static char *kwlist[] = {(char *)"input", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &input)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse keywords");
    return nullptr;
  }
  if (!ExtractorComputation_type_check(input)) {
    PyErr_SetString(PyExc_RuntimeError, "Argument is not an extractor"
                                        " computation");
    return nullptr;
  }
  ExtractorComputation *i = (ExtractorComputation *)input;
  fm_comp_t *i_comp = i->comp_;
  fm_comp_sys_t *sys = i->sys_;
  fm_type_sys_t *tsys = fm_type_sys_get(sys);
  fm_comp_graph *graph = i->graph_;
  auto *comp =
      fm_comp_decl(sys, graph, "convert", 1,
                   fm_tuple_type_get(tsys, 1, fm_type_type_get(tsys)), i_comp,
                   fm_type_from_py_type(tsys, (PyObject *)type));
  if (!comp) {
    if (fm_type_sys_errno(tsys) != FM_TYPE_ERROR_OK) {
      PyErr_SetString(PyExc_RuntimeError, fm_type_sys_errmsg(tsys));
    } else if (fm_comp_sys_is_error(sys)) {
      PyErr_SetString(PyExc_RuntimeError, fm_comp_sys_error_msg(sys));
    }
    return nullptr;
  };
  return (PyObject *)ExtractorComputation_new(comp, sys, graph);
}

template <bool B> struct integral_value { typedef long long type; };
template <> struct integral_value<true> { typedef unsigned long long type; };

template <class T> struct py_type_convert {
  static bool convert(T &val, PyObject *args) {
    using namespace std;
    if constexpr (is_same_v<bool, T>) {
      bool temp;
      if (!PyArg_ParseTuple(args, "p", &temp)) {
        PyErr_SetString(PyExc_TypeError, "expecting an integer value");
        return false;
      }
      val = temp;
      return true;
    } else if constexpr (is_integral_v<T>) {
      typename integral_value<is_unsigned_v<T>>::type temp;
      if (!PyArg_ParseTuple(args, "L", &temp) ||
          temp > numeric_limits<T>::max() || temp < numeric_limits<T>::min()) {
        PyErr_SetString(PyExc_TypeError, "expecting an integer value");
        return false;
      }
      val = temp;
      return true;
    } else if constexpr (is_floating_point<T>::value) {
      double temp;
      if (!PyArg_ParseTuple(args, "d", &temp) ||
          temp > numeric_limits<T>::max() || temp < numeric_limits<T>::min()) {
        PyErr_SetString(PyExc_TypeError, "expecting an float value");
        return false;
      }
      val = temp;
      return true;
    } else if constexpr (is_same_v<T, DECIMAL128>) {
      PyObject *temp;
      if (!PyArg_ParseTuple(args, "O", &temp)) {
        PyErr_SetString(PyExc_TypeError, "Expect single argument");
        return false;
      }
      if (PyUnicode_Check(temp)) {
        Py_ssize_t sz = 0;
        const char *str = PyUnicode_AsUTF8AndSize(temp, &sz);
        if (sz > FMC_DECIMAL128_STR_SIZE) {
          PyErr_SetString(PyExc_TypeError, "expecting a valid string value");
          return false;
        }
        fmc_decimal128_from_str(&val, str);
        return true;
      } else if (PyLong_Check(temp)) {
        uint64_t u = PyLong_AsUnsignedLongLong(temp);
        if (PyErr_Occurred()) {
          PyErr_Clear();
          int64_t i = PyLong_AsLongLong(temp);
          if (PyErr_Occurred()) {
            return false;
          } else {
            fmc_decimal128_from_int(&val, i);
            return true;
          }
        } else {
          fmc_decimal128_from_uint(&val, u);
          return true;
        }
      }
    }
    PyErr_SetString(PyExc_TypeError, "unknown type");
    return false;
  }
};

#define BASE_TYPE_WRAPPER(name, T)                                             \
  struct ExtractorBaseType##name {                                             \
    PyObject_HEAD;                                                             \
    T val;                                                                     \
    static void py_dealloc(ExtractorBaseType##name *self) {                    \
      Py_TYPE(self)->tp_free((PyObject *)self);                                \
    }                                                                          \
    static PyObject *py_richcmp(PyObject *obj1, PyObject *obj2, int op);       \
    static PyObject *tp_new(PyTypeObject *subtype, PyObject *args,             \
                            PyObject *kwds);                                   \
    static PyObject *py_new(T t);                                              \
    static PyObject *tp_str(PyObject *self);                                   \
    static bool init(PyObject *m);                                             \
  };                                                                           \
  static PyTypeObject ExtractorBaseType##name##Type = {                        \
      PyVarObject_HEAD_INIT(NULL, 0) "extractor." #name, /* tp_name */         \
      sizeof(ExtractorBaseType##name),                   /* tp_basicsize */    \
      0,                                                 /* tp_itemsize */     \
      (destructor)ExtractorBaseType##name::py_dealloc,   /* tp_dealloc */      \
      0,                                                 /* tp_print */        \
      0,                                                 /* tp_getattr */      \
      0,                                                 /* tp_setattr */      \
      0,                                                 /* tp_reserved */     \
      0,                                                 /* tp_repr */         \
      0,                                                 /* tp_as_number */    \
      0,                                                 /* tp_as_sequence */  \
      0,                                                 /* tp_as_mapping */   \
      0,                                                 /* tp_hash  */        \
      0,                                                 /* tp_call */         \
      (reprfunc)ExtractorBaseType##name::tp_str,         /* tp_str */          \
      0,                                                 /* tp_getattro */     \
      0,                                                 /* tp_setattro */     \
      0,                                                 /* tp_as_buffer */    \
      Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,          /* tp_flags */        \
      "Extractor system base type object",               /* tp_doc */          \
      0,                                                 /* tp_traverse */     \
      0,                                                 /* tp_clear */        \
      (richcmpfunc)ExtractorBaseType##name::py_richcmp,  /* tp_richcompare */  \
      0,                               /* tp_weaklistoffset */                 \
      0,                               /* tp_iter */                           \
      0,                               /* tp_iternext */                       \
      0,                               /* tp_methods */                        \
      0,                               /* tp_members */                        \
      0,                               /* tp_getset */                         \
      0,                               /* tp_base */                           \
      0,                               /* tp_dict */                           \
      0,                               /* tp_descr_get */                      \
      0,                               /* tp_descr_set */                      \
      0,                               /* tp_dictoffset */                     \
      0,                               /* tp_init */                           \
      0,                               /* tp_alloc */                          \
      ExtractorBaseType##name::tp_new, /* tp_new */                            \
  };                                                                           \
  PyObject *ExtractorBaseType##name::py_new(T t) {                             \
    PyTypeObject *type = (PyTypeObject *)&ExtractorBaseType##name##Type;       \
    ExtractorBaseType##name *self;                                             \
                                                                               \
    self = (ExtractorBaseType##name *)type->tp_alloc(type, 0);                 \
    if (self == nullptr)                                                       \
      return nullptr;                                                          \
                                                                               \
    self->val = t;                                                             \
    return (PyObject *)self;                                                   \
  }                                                                            \
  PyObject *ExtractorBaseType##name::tp_new(PyTypeObject *subtype,             \
                                            PyObject *args, PyObject *kwds) {  \
    PyObject *input = NULL;                                                    \
    if (PyArg_ParseTuple(args, "O", &input) &&                                 \
        ExtractorComputation_type_check(input))                                \
      return create(subtype, args, kwds);                                      \
    T val;                                                                     \
    if (py_type_convert<T>::convert(val, args)) {                              \
      return py_new(val);                                                      \
    }                                                                          \
    PyErr_SetString(PyExc_RuntimeError, "Could not convert to type " /*##T*/); \
    return nullptr;                                                            \
  }                                                                            \
  PyObject *ExtractorBaseType##name::tp_str(PyObject *self) {                  \
    std::string str = std::to_string(((ExtractorBaseType##name *)self)->val);  \
    return PyUnicode_FromString(str.c_str());                                  \
  }                                                                            \
                                                                               \
  bool ExtractorBaseType##name::init(PyObject *m) {                            \
    if (PyType_Ready(&ExtractorBaseType##name##Type) < 0)                      \
      return false;                                                            \
    Py_INCREF(&ExtractorBaseType##name##Type);                                 \
    PyModule_AddObject(m, #name, (PyObject *)&ExtractorBaseType##name##Type);  \
    return true;                                                               \
  }                                                                            \
                                                                               \
  PyObject *ExtractorBaseType##name::py_richcmp(PyObject *obj1,                \
                                                PyObject *obj2, int op) {      \
    auto type = &ExtractorBaseType##name##Type;                                \
    if (!PyObject_TypeCheck(obj1, type) || !PyObject_TypeCheck(obj2, type)) {  \
      if (op == Py_NE) {                                                       \
        Py_RETURN_TRUE;                                                        \
      }                                                                        \
      Py_RETURN_FALSE;                                                         \
    }                                                                          \
    PyObject *result;                                                          \
    int c = 0;                                                                 \
    T t1;                                                                      \
    T t2;                                                                      \
    t1 = ((ExtractorBaseType##name *)obj1)->val;                               \
    t2 = ((ExtractorBaseType##name *)obj2)->val;                               \
    switch (op) {                                                              \
    case Py_LT:                                                                \
      c = t1 < t2;                                                             \
      break;                                                                   \
    case Py_LE:                                                                \
      c = t1 <= t2;                                                            \
      break;                                                                   \
    case Py_EQ:                                                                \
      c = t1 == t2;                                                            \
      break;                                                                   \
    case Py_NE:                                                                \
      c = t1 != t2;                                                            \
      break;                                                                   \
    case Py_GT:                                                                \
      c = t1 > t2;                                                             \
      break;                                                                   \
    case Py_GE:                                                                \
      c = t1 >= t2;                                                            \
      break;                                                                   \
    }                                                                          \
    result = c ? Py_True : Py_False;                                           \
    Py_INCREF(result);                                                         \
    return result;                                                             \
  }

// for now we need to declare the type separately
struct ExtractorBaseTypeTime64 {
  PyObject_HEAD;
  fmc_time64_t val;
  static void py_dealloc(ExtractorBaseTypeTime64 *self) {
    Py_TYPE(self)->tp_free((PyObject *)self);
  }
  static PyObject *py_richcmp(PyObject *obj1, PyObject *obj2, int op);
  static PyObject *py_new(fmc_time64_t t);
  static bool init(PyObject *m);
  static PyObject *tp_new(PyTypeObject *type, PyObject *args, PyObject *kwds);
};

static PyObject *ExtractorBaseTypeTime64_from_seconds(PyObject *type,
                                                      PyObject *sec_obj) {
  int sec;
  if (!PyArg_ParseTuple(sec_obj, "i", &sec)) {
    PyErr_SetString(PyExc_TypeError, "expecting an integer number of "
                                     "seconds");
    return nullptr;
  }
  return ExtractorBaseTypeTime64::py_new(fmc_time64_from_seconds(sec));
}

static PyObject *ExtractorBaseTypeTime64_from_timedelta(PyObject *self,
                                                        PyObject *args) {
  using namespace std::chrono;
  PyObject *delta;
  if (!PyArg_ParseTuple(args, "O", &delta)) {
    return nullptr;
  }
  if (!PyDelta_Check(delta)) {
    PyErr_SetString(PyExc_TypeError, "expecting timedelta object");
    return nullptr;
  };
  int64_t days = PyDateTime_DELTA_GET_DAYS(delta);
  int64_t secs = days * 24 * 3600 + PyDateTime_DELTA_GET_SECONDS(delta);
  int64_t mics = PyDateTime_DELTA_GET_MICROSECONDS(delta);
  int64_t total_nanos = secs * 1000000000 + mics * 1000;
  auto t = fmc_time64_from_nanos(total_nanos);
  return ExtractorBaseTypeTime64::py_new(t);
}

static PyObject *ExtractorBaseTypeTime64_end_time(PyObject *type,
                                                  PyObject *sec_obj) {
  return ExtractorBaseTypeTime64::py_new(fmc_time64_end());
}

static PyObject *ExtractorBaseTypeTime64_as_timedelta(PyObject *self) {
  int64_t ns = fmc_time64_to_nanos(((ExtractorBaseTypeTime64 *)self)->val);
  int64_t us = ns / 1000;
  int64_t sec = us / 1000000;
  us = us - sec * 1000000;
  return PyDelta_FromDSU(0, sec, us);
}

static PyObject *ExtractorBaseTypeTime64_from_nanos(PyObject *self,
                                                    PyObject *args) {
  using namespace std::chrono;
  int64_t nanos;

  if (!PyArg_ParseTuple(args, "l", &nanos)) {
    return nullptr;
  }
  auto t = fmc_time64_from_nanos(nanos);
  return ExtractorBaseTypeTime64::py_new(t);
}

static PyObject *ExtractorBaseTypeTime64_as_nanos(PyObject *self) {
  int64_t ns = fmc_time64_to_nanos(((ExtractorBaseTypeTime64 *)self)->val);
  return PyLong_FromLongLong(ns);
}

static PyMethodDef ExtractorBaseTypeTime64_methods[] = {
    {"from_seconds", (PyCFunction)ExtractorBaseTypeTime64_from_seconds,
     METH_VARARGS | METH_CLASS,
     "Returns an Extractor Time64 object from the specified time.\n"
     "This method expects as a single argument the desired time in seconds as "
     "an integer."},
    {"from_timedelta", (PyCFunction)ExtractorBaseTypeTime64_from_timedelta,
     METH_VARARGS | METH_CLASS,
     "Returns an Extractor Time64 object from the specified timedelta object.\n"
     "This method expects as a single argument the desired timedelta object"},
    {"as_timedelta", (PyCFunction)ExtractorBaseTypeTime64_as_timedelta,
     METH_NOARGS, "Returns the time as a timedelta "},
    {"from_nanos", (PyCFunction)ExtractorBaseTypeTime64_from_nanos,
     METH_VARARGS | METH_CLASS,
     "Returns an Extractor Time64 object from the specified nanos object.\n"
     "This method expects as a single argument the desired nanos object"},
    {"as_nanos", (PyCFunction)ExtractorBaseTypeTime64_as_nanos, METH_NOARGS,
     "Returns the time as a nanos "},
    {"end_time", (PyCFunction)ExtractorBaseTypeTime64_end_time,
     METH_VARARGS | METH_CLASS, "Returns the end of time value as Time64"},
    {NULL} /* Sentinel */
};

static PyTypeObject ExtractorBaseTypeTime64Type = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Time64", /* tp_name */
    sizeof(ExtractorBaseTypeTime64),                   /* tp_basicsize */
    0,                                                 /* tp_itemsize */
    (destructor)ExtractorBaseTypeTime64::py_dealloc,   /* tp_dealloc */
    0,                                                 /* tp_print */
    0,                                                 /* tp_getattr */
    0,                                                 /* tp_setattr */
    0,                                                 /* tp_reserved */
    0,                                                 /* tp_repr */
    0,                                                 /* tp_as_number */
    0,                                                 /* tp_as_sequence */
    0,                                                 /* tp_as_mapping */
    0,                                                 /* tp_hash  */
    0,                                                 /* tp_call */
    0,                                                 /* tp_str */
    0,                                                 /* tp_getattro */
    0,                                                 /* tp_setattro */
    0,                                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,          /* tp_flags */
    "Extractor system base type object",               /* tp_doc */
    0,                                                 /* tp_traverse */
    0,                                                 /* tp_clear */
    (richcmpfunc)ExtractorBaseTypeTime64::py_richcmp,  /* tp_richcompare */
    0,                                                 /* tp_weaklistoffset */
    0,                                                 /* tp_iter */
    0,                                                 /* tp_iternext */
    ExtractorBaseTypeTime64_methods,                   /* tp_methods */
    0,                                                 /* tp_members */
    0,                                                 /* tp_getset */
    0,                                                 /* tp_base */
    0,                                                 /* tp_dict */
    0,                                                 /* tp_descr_get */
    0,                                                 /* tp_descr_set */
    0,                                                 /* tp_dictoffset */
    0,                                                 /* tp_init */
    0,                                                 /* tp_alloc */
    ExtractorBaseTypeTime64::tp_new,                   /* tp_new */
};

#include <iostream>
using namespace std;

PyObject *ExtractorBaseTypeTime64::tp_new(PyTypeObject *type, PyObject *args,
                                          PyObject *kwds) {
  auto error = [](const char *err) {
    PyErr_SetString(PyExc_TypeError, err);
    return nullptr;
  };

  PyObject *input = NULL;
  if (!PyArg_ParseTuple(args, "O", &input))
    error("cannot parse tuple");

  fm::python::datetime dt(fm::python::object::from_borrowed(input));
  auto time64 = static_cast<fmc_time64_t>(dt);
  auto *err = PyErr_Occurred();
  if (err == nullptr)
    return py_new(time64);
  else
    PyErr_Clear();
  return create(type, args, kwds);
}

PyObject *ExtractorBaseTypeTime64::py_new(fmc_time64_t t) {
  PyTypeObject *type = (PyTypeObject *)&ExtractorBaseTypeTime64Type;
  ExtractorBaseTypeTime64 *self;

  self = (ExtractorBaseTypeTime64 *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->val = t;
  return (PyObject *)self;
}

bool ExtractorBaseTypeTime64::init(PyObject *m) {
  if (PyType_Ready(&ExtractorBaseTypeTime64Type) < 0)
    return false;
  Py_INCREF(&ExtractorBaseTypeTime64Type);
  PyModule_AddObject(m, "Time64", (PyObject *)&ExtractorBaseTypeTime64Type);
  return true;
}

PyObject *ExtractorBaseTypeTime64::py_richcmp(PyObject *obj1, PyObject *obj2,
                                              int op) {
  if (!PyObject_TypeCheck(obj1, &ExtractorBaseTypeTime64Type) ||
      !PyObject_TypeCheck(obj2, &ExtractorBaseTypeTime64Type)) {
    if (op == Py_NE) {
      Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
  }
  PyObject *result;
  int c = 0;
  fmc_time64_t t1;
  fmc_time64_t t2;
  t1 = ((ExtractorBaseTypeTime64 *)obj1)->val;
  t2 = ((ExtractorBaseTypeTime64 *)obj2)->val;
  switch (op) {
  case Py_LT:
    c = t1 < t2;
    break;
  case Py_LE:
    c = t1 <= t2;
    break;
  case Py_EQ:
    c = t1 == t2;
    break;
  case Py_NE:
    c = t1 != t2;
    break;
  case Py_GT:
    c = t1 > t2;
    break;
  case Py_GE:
    c = t1 >= t2;
    break;
  }
  result = c ? Py_True : Py_False;
  Py_INCREF(result);
  return result;
}

struct ExtractorArrayType {
  PyObject_HEAD;
  PyObject *type;
  unsigned size;
  static void py_dealloc(ExtractorArrayType *self) {
    Py_XDECREF(self->type);
    Py_TYPE(self)->tp_free((PyObject *)self);
  }
  static PyObject *py_richcmp(PyObject *obj1, PyObject *obj2, int op);
  static PyObject *py_new(PyTypeObject *subtype, PyObject *args,
                          PyObject *kwds);
  static bool init(PyObject *m);
};
static PyTypeObject ExtractorArrayTypeType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Array", /* tp_name */
    sizeof(ExtractorArrayType),                       /* tp_basicsize */
    0,                                                /* tp_itemsize */
    (destructor)ExtractorArrayType::py_dealloc,       /* tp_dealloc */
    0,                                                /* tp_print */
    0,                                                /* tp_getattr */
    0,                                                /* tp_setattr */
    0,                                                /* tp_reserved */
    0,                                                /* tp_repr */
    0,                                                /* tp_as_number */
    0,                                                /* tp_as_sequence */
    0,                                                /* tp_as_mapping */
    0,                                                /* tp_hash  */
    0,                                                /* tp_call */
    0,                                                /* tp_str */
    0,                                                /* tp_getattro */
    0,                                                /* tp_setattro */
    0,                                                /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,         /* tp_flags */
    "Extractor system base type object",              /* tp_doc */
    0,                                                /* tp_traverse */
    0,                                                /* tp_clear */
    (richcmpfunc)ExtractorArrayType::py_richcmp,      /* tp_richcompare */
    0,                                                /* tp_weaklistoffset */
    0,                                                /* tp_iter */
    0,                                                /* tp_iternext */
    0,                                                /* tp_methods */
    0,                                                /* tp_members */
    0,                                                /* tp_getset */
    0,                                                /* tp_base */
    0,                                                /* tp_dict */
    0,                                                /* tp_descr_get */
    0,                                                /* tp_descr_set */
    0,                                                /* tp_dictoffset */
    0,                                                /* tp_init */
    0,                                                /* tp_alloc */
    (newfunc)ExtractorArrayType::py_new,              /* tp_new */
};

inline PyObject *ArrayTypeGen(PyTypeObject *subtype, PyObject *obj,
                              unsigned size) {
  auto *self = (ExtractorArrayType *)subtype->tp_alloc(subtype, 0);
  if (self == nullptr)
    return nullptr;

  Py_XINCREF(obj);
  self->type = obj;
  self->size = size;

  return (PyObject *)self;
}

PyObject *ExtractorArrayType::py_new(PyTypeObject *subtype, PyObject *args,
                                     PyObject *kwds) {
  PyObject *obj;
  unsigned size;
  if (!PyArg_ParseTuple(args, "OI", &obj, &size)) {
    PyErr_SetString(PyExc_TypeError, "expecting an Extractor type object "
                                     "and an unsigned int");
    return nullptr;
  }
  return ArrayTypeGen(subtype, obj, size);
}

bool ExtractorArrayType::init(PyObject *m) {
  if (PyType_Ready(&ExtractorArrayTypeType) < 0)
    return false;
  Py_INCREF(&ExtractorArrayTypeType);
  PyModule_AddObject(m, "Array", (PyObject *)&ExtractorArrayTypeType);
  return true;
}

PyObject *ExtractorArrayType::py_richcmp(PyObject *obj1, PyObject *obj2,
                                         int op) {

  if (!PyObject_TypeCheck(obj1, &ExtractorArrayTypeType) ||
      !PyObject_TypeCheck(obj2, &ExtractorArrayTypeType)) {
    if (op == Py_NE) {
      Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
  }

  auto self = (ExtractorArrayType *)obj1;
  auto other = (ExtractorArrayType *)obj2;

  if (self->size != other->size) {
    if (op == Py_NE) {
      Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
  }

  if (PyObject_TypeCheck(self->type, &ExtractorArrayTypeType)) {
    return ExtractorArrayType::py_richcmp(self->type, other->type, op);
  }

  switch (op) {
  case Py_LT:
    break;
  case Py_LE:
    break;
  case Py_EQ:
    if (self->type == other->type)
      Py_RETURN_TRUE;
    break;
  case Py_NE:
    if (self->type != other->type)
      Py_RETURN_TRUE;
    break;
  case Py_GT:
    break;
  case Py_GE:
    break;
  }

  Py_RETURN_FALSE;
}

BASE_TYPE_WRAPPER(Int8, INT8);
BASE_TYPE_WRAPPER(Int16, INT16);
BASE_TYPE_WRAPPER(Int32, INT32);
BASE_TYPE_WRAPPER(Int64, INT64);
BASE_TYPE_WRAPPER(Uint8, UINT8);
BASE_TYPE_WRAPPER(Uint16, UINT16);
BASE_TYPE_WRAPPER(Uint32, UINT32);
BASE_TYPE_WRAPPER(Uint64, UINT64);
BASE_TYPE_WRAPPER(Float32, FLOAT32);
BASE_TYPE_WRAPPER(Float64, FLOAT64);
BASE_TYPE_WRAPPER(Rational64, RATIONAL64);
BASE_TYPE_WRAPPER(Decimal64, DECIMAL64);
BASE_TYPE_WRAPPER(Char, CHAR);
BASE_TYPE_WRAPPER(Wchar, WCHAR);
BASE_TYPE_WRAPPER(Bool, bool);

fm_type_decl_cp fm_type_from_py_type(fm_type_sys_t *tsys, PyObject *obj) {
  if (PyObject_TypeCheck(obj, &ExtractorArrayTypeType)) {
    auto *py_obj = (ExtractorArrayType *)obj;
    auto subtype = fm_type_from_py_type(tsys, py_obj->type);
    if (subtype == nullptr)
      return nullptr;
    return fm_array_type_get(tsys, subtype, py_obj->size);
  }
  if (!PyType_CheckExact(obj))
    return nullptr;
  if (PyType_IsSubtype((PyTypeObject *)obj, &ExtractorBaseTypeInt8Type)) {
    return fm_base_type_get(tsys, FM_TYPE_INT8);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeInt16Type)) {
    return fm_base_type_get(tsys, FM_TYPE_INT16);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeInt32Type)) {
    return fm_base_type_get(tsys, FM_TYPE_INT32);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeInt64Type)) {
    return fm_base_type_get(tsys, FM_TYPE_INT64);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeUint8Type)) {
    return fm_base_type_get(tsys, FM_TYPE_UINT8);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeUint16Type)) {
    return fm_base_type_get(tsys, FM_TYPE_UINT16);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeUint32Type)) {
    return fm_base_type_get(tsys, FM_TYPE_UINT32);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeUint64Type)) {
    return fm_base_type_get(tsys, FM_TYPE_UINT64);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeFloat32Type)) {
    return fm_base_type_get(tsys, FM_TYPE_FLOAT32);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeFloat64Type)) {
    return fm_base_type_get(tsys, FM_TYPE_FLOAT64);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeRational64Type)) {
    return fm_base_type_get(tsys, FM_TYPE_RATIONAL64);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeDecimal64Type)) {
    return fm_base_type_get(tsys, FM_TYPE_DECIMAL64);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeDecimal128Type)) {
    return fm_base_type_get(tsys, FM_TYPE_DECIMAL128);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeTime64Type)) {
    return fm_base_type_get(tsys, FM_TYPE_TIME64);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeCharType)) {
    return fm_base_type_get(tsys, FM_TYPE_CHAR);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeWcharType)) {
    return fm_base_type_get(tsys, FM_TYPE_WCHAR);
  } else if (PyType_IsSubtype((PyTypeObject *)obj,
                              &ExtractorBaseTypeBoolType)) {
    return fm_base_type_get(tsys, FM_TYPE_BOOL);
  }
  return nullptr;
}
#include <iostream>
fm_type_decl_cp fm_type_from_py_obj(fm_type_sys_t *tsys, PyObject *o,
                                    fm_arg_stack_t *&s) {
  if (PyObject_TypeCheck(o, &ExtractorBaseTypeInt8Type)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeInt8 *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_INT8);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeInt16Type)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeInt16 *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_INT16);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeInt32Type)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeInt32 *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_INT32);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeInt64Type)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeInt64 *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_INT64);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeUint8Type)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeUint8 *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_UINT8);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeUint16Type)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeUint16 *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_UINT16);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeUint32Type)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeUint32 *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_UINT32);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeUint64Type)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeUint64 *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_UINT64);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeFloat32Type)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeFloat32 *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_FLOAT32);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeFloat64Type)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeFloat64 *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_FLOAT64);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeCharType)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeChar *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_CHAR);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeWcharType)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeWchar *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_WCHAR);
  } else if (PyObject_TypeCheck(o, &ExtractorBaseTypeBoolType)) {
    HEAP_STACK_PUSH(s, ((ExtractorBaseTypeBool *)o)->val);
    return fm_base_type_get(tsys, FM_TYPE_BOOL);
  }
  return nullptr;
}

PyTypeObject *py_type_from_fm_type(fm_type_decl_cp decl) {
  if (fm_type_is_base(decl)) {
    switch (fm_type_base_enum(decl)) {
    case FM_TYPE_CHAR:
      Py_INCREF(&ExtractorBaseTypeCharType);
      return &ExtractorBaseTypeCharType;
    case FM_TYPE_INT8:
      Py_INCREF(&ExtractorBaseTypeInt8Type);
      return &ExtractorBaseTypeInt8Type;
      break;
    case FM_TYPE_INT16:
      Py_INCREF(&ExtractorBaseTypeInt16Type);
      return &ExtractorBaseTypeInt16Type;
      break;
    case FM_TYPE_INT32:
      Py_INCREF(&ExtractorBaseTypeInt32Type);
      return &ExtractorBaseTypeInt32Type;
      break;
    case FM_TYPE_INT64:
      Py_INCREF(&ExtractorBaseTypeInt64Type);
      return &ExtractorBaseTypeInt64Type;
      break;
    case FM_TYPE_UINT8:
      Py_INCREF(&ExtractorBaseTypeUint8Type);
      return &ExtractorBaseTypeUint8Type;
      break;
    case FM_TYPE_UINT16:
      Py_INCREF(&ExtractorBaseTypeUint16Type);
      return &ExtractorBaseTypeUint16Type;
      break;
    case FM_TYPE_UINT32:
      Py_INCREF(&ExtractorBaseTypeUint32Type);
      return &ExtractorBaseTypeUint32Type;
      break;
    case FM_TYPE_UINT64:
      Py_INCREF(&ExtractorBaseTypeUint64Type);
      return &ExtractorBaseTypeUint64Type;
      break;
    case FM_TYPE_FLOAT32:
      Py_INCREF(&ExtractorBaseTypeFloat32Type);
      return &ExtractorBaseTypeFloat32Type;
      break;
    case FM_TYPE_FLOAT64:
      Py_INCREF(&ExtractorBaseTypeFloat64Type);
      return &ExtractorBaseTypeFloat64Type;
      break;
    case FM_TYPE_DECIMAL64:
      Py_INCREF(&ExtractorBaseTypeDecimal64Type);
      return &ExtractorBaseTypeDecimal64Type;
      break;
    case FM_TYPE_DECIMAL128:
      Py_INCREF(&ExtractorBaseTypeDecimal128Type);
      return &ExtractorBaseTypeDecimal128Type;
      break;
    case FM_TYPE_RATIONAL64:
      Py_INCREF(&ExtractorBaseTypeRational64Type);
      return &ExtractorBaseTypeRational64Type;
      break;
    case FM_TYPE_TIME64: {
      Py_INCREF(&ExtractorBaseTypeTime64Type);
      return &ExtractorBaseTypeTime64Type;
    } break;
    case FM_TYPE_BOOL:
      Py_INCREF(&ExtractorBaseTypeBoolType);
      return &ExtractorBaseTypeBoolType;
      break;
    case FM_TYPE_LAST:
    default:
      break;
    }
  } else if (fm_type_is_array(decl) &&
             fm_type_is_base(fm_type_array_of(decl)) &&
             fm_type_base_enum(fm_type_array_of(decl)) == FM_TYPE_CHAR) {
    auto sz = fm_type_array_size(decl);
    return (PyTypeObject *)ArrayTypeGen(
        &ExtractorArrayTypeType, (PyObject *)&ExtractorBaseTypeCharType, sz);
  }
  return nullptr;
}

bool init_type_wrappers(PyObject *m) {
  return ExtractorBaseTypeInt8::init(m) && ExtractorBaseTypeInt16::init(m) &&
         ExtractorBaseTypeInt32::init(m) && ExtractorBaseTypeInt64::init(m) &&
         ExtractorBaseTypeUint8::init(m) && ExtractorBaseTypeUint16::init(m) &&
         ExtractorBaseTypeUint32::init(m) && ExtractorBaseTypeUint64::init(m) &&
         ExtractorBaseTypeFloat32::init(m) &&
         ExtractorBaseTypeFloat64::init(m) &&
         ExtractorBaseTypeTime64::init(m) &&
         ExtractorBaseTypeRational64::init(m) &&
         ExtractorBaseTypeDecimal64::init(m) &&
         ExtractorBaseTypeDecimal128::init(m) &&
         ExtractorBaseTypeChar::init(m) && ExtractorBaseTypeWchar::init(m) &&
         ExtractorArrayType::init(m) && ExtractorBaseTypeBool::init(m);
  return false;
}
