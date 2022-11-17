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
 * @file comp.hpp
 * @author Maxim Trokhimtchouk
 * @date 5 Oct 2017
 * @brief Python extension for extractor library
 *
 * This file contains Python C extention for extractor library
 */

#pragma once

#include "comp.h"
#include "extractor/comp_sys.h"

#include "comp_base.hpp"
#include "types.hpp"
#include "wrapper.hpp"

#include <Python.h>

// Field description iterator

typedef struct {
  PyObject_HEAD ExtractorComputation *comp_;
  unsigned iter_;
} ExtractorComputationDescriptionIter;

static void ExtractorComputationDescriptionIter_dealloc(
    ExtractorComputationDescriptionIter *self) {
  Py_XDECREF(self->comp_);
  self->~ExtractorComputationDescriptionIter();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorComputationDescriptionIter_iter(
    ExtractorComputationDescriptionIter *self) {
  Py_XINCREF(self);
  return (PyObject *)self;
}

static PyObject *ExtractorComputationDescriptionIter_iternext(
    ExtractorComputationDescriptionIter *self) {
  auto type = fm_comp_result_type(self->comp_->comp_);

  auto nfields = fm_type_frame_nfields(type);

  if (self->iter_ != nfields) {

    PyObject *ret = PyTuple_New(2);

    auto fieldtype = fm_type_frame_field_type(type, self->iter_);
    auto fieldname = fm_type_frame_field_name(type, self->iter_);

    PyTuple_SetItem(ret, 0, PyUnicode_FromString(fieldname));
    PyTuple_SetItem(ret, 1, (PyObject *)py_type_from_fm_type(fieldtype));

    self->iter_++;
    return ret;
  }

  PyErr_SetNone(PyExc_StopIteration);
  return NULL;
}

static PyTypeObject ExtractorComputationDescriptionIterType = {
    PyVarObject_HEAD_INIT(
        NULL, 0) "extractor.ComputationDescriptionIter",     /* tp_name */
    sizeof(ExtractorComputationDescriptionIter),             /* tp_basicsize */
    0,                                                       /* tp_itemsize */
    (destructor)ExtractorComputationDescriptionIter_dealloc, /* tp_dealloc */
    0,                                                       /* tp_print */
    0,                                                       /* tp_getattr */
    0,                                                       /* tp_setattr */
    0,                                                       /* tp_reserved */
    0,                                                       /* tp_repr */
    0,                                                       /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "Extractor features iterator",            /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    (getiterfunc)ExtractorComputationDescriptionIter_iter, /* tp_iter */
    (iternextfunc)
        ExtractorComputationDescriptionIter_iternext, /* tp_iternext */
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
    0,                                                /* tp_new */
};

ExtractorComputationDescriptionIter *
ExtractorComputationDescriptionIter_new(ExtractorComputation *comp) {
  PyTypeObject *subtype = &ExtractorComputationDescriptionIterType;
  auto *self =
      (ExtractorComputationDescriptionIter *)subtype->tp_alloc(subtype, 0);
  Py_INCREF(comp);
  self->comp_ = comp;
  self->iter_ = 0;
  return self;
}

// Extractor Computation

static void ExtractorComputation_dealloc(ExtractorComputation *self) {
  Py_TYPE(self)->tp_free((PyObject *)self);
}

PyObject *BinaryCompGen_NoArgs(PyObject *s, PyObject *arg, const char *name);

static PyObject *ExtractorComputation_add(PyObject *s, PyObject *arg) {
  return BinaryCompGen_NoArgs(s, arg, "add");
}

static PyObject *ExtractorComputation_substract(PyObject *s, PyObject *arg) {
  return BinaryCompGen_NoArgs(s, arg, "diff");
}

static PyObject *ExtractorComputation_multiply(PyObject *s, PyObject *arg) {
  return BinaryCompGen_NoArgs(s, arg, "mult");
}

static PyObject *ExtractorComputation_div(PyObject *s, PyObject *arg) {
  return BinaryCompGen_NoArgs(s, arg, "divide");
}

static PyObject *ExtractorComputation_pow(PyObject *first, PyObject *second,
                                          PyObject *third) {
  // Third argument is ignored
  // Third argument should not be ignored once module
  // operation is included in Extractor
  return BinaryCompGen_NoArgs(first, second, "pow");
}

static PyObject *ExtractorComputation_and(PyObject *s, PyObject *arg) {
  return BinaryCompGen_NoArgs(s, arg, "logical_and");
}

static PyObject *ExtractorComputation_or(PyObject *s, PyObject *arg) {
  return BinaryCompGen_NoArgs(s, arg, "logical_or");
}

PyObject *FieldGen_NoArgs(PyObject *s, char *name);

static PyObject *ExtractorComputation_getattr(PyObject *obj, char *name) {
  if (!ExtractorComputation_type_check(obj)) {
    PyErr_SetString(PyExc_RuntimeError, "Argument is not an extractor"
                                        " computation");
    return nullptr;
  }

  // NOTE: Safe to use strcmp because name is guaranteed to be null terminated:
  // PyUnicode_AsUTF8: "The returned buffer always has an extra null byte
  // appended"
  bool fields = strcmp("_fields", name) == 0;
  bool shape = strcmp("_shape", name) == 0;
  if (fields || shape) {
    ExtractorComputation *self = (ExtractorComputation *)obj;
    fm_comp_t *comp = self->comp_;
    auto type = fm_comp_result_type(comp);
    if (fields) {
      auto nfields = fm_type_frame_nfields(type);
      auto ret = fm::python::object::from_new(PyDict_New());
      for (auto i = 0U; i < nfields; ++i) {
        auto fieldtype = fm_type_frame_field_type(type, i);
        auto fieldname = fm_type_frame_field_name(type, i);
        auto pytype = fm::python::object::from_new(
            (PyObject *)py_type_from_fm_type(fieldtype));
        PyDict_SetItemString(ret.get_ref(), fieldname, pytype.get_ref());
      }
      return ret.steal_ref();
    } else if (shape) {
      auto dims = fm_type_frame_ndims(type);
      auto ret = fm::python::object::from_new(PyTuple_New(dims));
      for (auto i = 0U; i < dims; ++i) {
        auto dim = fm_type_frame_dim(type, i);
        PyTuple_SetItem(ret.get_ref(), i, PyLong_FromLong(dim));
      }
      return ret.steal_ref();
    }
  }

  return FieldGen_NoArgs(obj, name);
}

static PyObject *ExtractorComputation_richcompare(PyObject *obj1,
                                                  PyObject *obj2, int op);

static PyNumberMethods ExtractorComputation_numerical_methods[] = {
    ExtractorComputation_add,       /*nb_add*/
    ExtractorComputation_substract, /*nb_substract*/
    ExtractorComputation_multiply,  /*nb_multiply*/
    NULL,                           /*nb_remainder*/
    NULL,                           /*nb_divmod*/
    ExtractorComputation_pow,       /*nb_power*/
    NULL,                           /*nb_negative*/
    NULL,                           /*nb_positive*/
    NULL,                           /*nb_absolute*/
    NULL,                           /*nb_bool*/
    NULL,                           /*nb_invert*/
    NULL,                           /*nb_lshift*/
    NULL,                           /*nb_rshift*/
    ExtractorComputation_and,       /*nb_and*/
    NULL,                           /*nb_xor*/
    ExtractorComputation_or,        /*nb_or*/
    NULL,                           /*nb_int*/
    NULL,                           /*nb_reserved*/
    NULL,                           /*nb_float*/
    NULL,                           /*nb_inplace_add*/
    NULL,                           /*nb_inplace_substract*/
    NULL,                           /*nb_inplace_multiply*/
    NULL,                           /*nb_inplace_remainder*/
    NULL,                           /*nb_inplace_power*/
    NULL,                           /*nb_inplace_lshift*/
    NULL,                           /*nb_inplace_rshift*/
    NULL,                           /*nb_inplace_and*/
    NULL,                           /*nb_inplace_xor*/
    NULL,                           /*nb_inplace_or*/
    NULL,                           /*nb_floor_divide*/
    ExtractorComputation_div,       /*nb_true_divide*/
    NULL,                           /*nb_inplace_floor_divide*/
    NULL,                           /*nb_inplace_true_divide*/
    NULL,                           /*nb_index*/
    NULL,                           /*nb_matrix_multiply*/
    NULL                            /*nb_inplace_matrix_multiply*/
};

static PyObject *ExtractorComputation_iter(ExtractorComputation *self) {
  auto ret = ExtractorComputationDescriptionIter_new(self);
  return (PyObject *)ret;
}

static PyTypeObject ExtractorComputationType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Computation", /* tp_name */
    sizeof(ExtractorComputation),                           /* tp_basicsize */
    0,                                                      /* tp_itemsize */
    (destructor)ExtractorComputation_dealloc,               /* tp_dealloc */
    0,                                                      /* tp_print */
    ExtractorComputation_getattr,                           /* tp_getattr */
    0,                                                      /* tp_setattr */
    0,                                                      /* tp_reserved */
    0,                                                      /* tp_repr */
    ExtractorComputation_numerical_methods,                 /* tp_as_number */
    0,                                                      /* tp_as_sequence */
    0,                                                      /* tp_as_mapping */
    0,                                                      /* tp_hash  */
    0,                                                      /* tp_call */
    0,                                                      /* tp_str */
    0,                                                      /* tp_getattro */
    0,                                                      /* tp_setattro */
    0,                                                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,               /* tp_flags */
    "Computation objects",                                  /* tp_doc */
    0,                                                      /* tp_traverse */
    0,                                                      /* tp_clear */
    ExtractorComputation_richcompare,                       /* tp_richcompare */
    0,                                      /* tp_weaklistoffset */
    (getiterfunc)ExtractorComputation_iter, /* tp_iter */
    0,                                      /* tp_iternext */
    0,                                      /* tp_methods */
    0,                                      /* tp_members */
    0,                                      /* tp_getset */
    0,                                      /* tp_base */
    0,                                      /* tp_dict */
    0,                                      /* tp_descr_get */
    0,                                      /* tp_descr_set */
    0,                                      /* tp_dictoffset */
    0,                                      /* tp_init */
    0,                                      /* tp_alloc */
    0,                                      /* tp_new */
};

bool ExtractorComputation_type_check(PyObject *obj) {
  return PyObject_TypeCheck(obj, &ExtractorComputationType);
}

PyObject *ExtractorComputation_new(fm_comp_t *comp, fm_comp_sys_t *sys,
                                   fm_comp_graph_t *graph) {
  PyTypeObject *type = &ExtractorComputationType;
  ExtractorComputation *self;

  self = (ExtractorComputation *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->comp_ = comp;
  self->sys_ = sys;
  self->graph_ = graph;

  return (PyObject *)self;
}

PyObject *ConstGen(PyObject *obj, fm_comp_sys_t *sys, fm_comp_graph *graph) {
  fm_type_sys_t *tsys = fm_type_sys_get(sys);

  fm_comp_t *comp = nullptr;

  if (PyUnicode_Check(obj)) {
    const char *val = PyUnicode_AsUTF8(obj);
    auto *type = fm_cstring_type_get(tsys);
    auto *constant_param_t =
        fm_tuple_type_get(tsys, 1,
                          fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                                            fm_type_type_get(tsys), type));

    comp = fm_comp_decl(sys, graph, "constant", 0, constant_param_t, "const",
                        type, val);
  } else if (PyBool_Check(obj)) {
    bool val = obj == Py_True;
    auto *type = fm_base_type_get(tsys, FM_TYPE_BOOL);
    auto *constant_param_t =
        fm_tuple_type_get(tsys, 1,
                          fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                                            fm_type_type_get(tsys), type));

    comp = fm_comp_decl(sys, graph, "constant", 0, constant_param_t, "const",
                        type, val);
  } else if (PyLong_Check(obj)) {
    int64_t val = PyLong_AsLongLong(obj);
    auto *type = fm_base_type_get(tsys, FM_TYPE_INT64);
    auto *constant_param_t =
        fm_tuple_type_get(tsys, 1,
                          fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                                            fm_type_type_get(tsys), type));

    comp = fm_comp_decl(sys, graph, "constant", 0, constant_param_t, "const",
                        type, val);
  } else if (PyFloat_Check(obj)) {
    double val = PyFloat_AsDouble(obj);
    auto *type = fm_base_type_get(tsys, FM_TYPE_FLOAT64);
    auto *constant_param_t =
        fm_tuple_type_get(tsys, 1,
                          fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                                            fm_type_type_get(tsys), type));

    comp = fm_comp_decl(sys, graph, "constant", 0, constant_param_t, "const",
                        type, val);
  } else if (PyDelta_Check(obj)) {
    auto h = 24 * PyLong_AsLong(PyObject_GetAttrString(obj, "days"));
    auto sec = PyLong_AsLong(PyObject_GetAttrString(obj, "seconds"));
    auto us = PyLong_AsLong(PyObject_GetAttrString(obj, "microseconds"));
    auto val = fmc_time64_from_nanos(us * 1000) +
               fmc_time64_from_seconds(h * 3600 + sec);
    auto *type = fm_base_type_get(tsys, FM_TYPE_TIME64);
    auto *constant_param_t =
        fm_tuple_type_get(tsys, 1,
                          fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                                            fm_type_type_get(tsys), type));

    comp = fm_comp_decl(sys, graph, "constant", 0, constant_param_t, "const",
                        type, val);
  } else {
    return nullptr;
  }

  if (!comp) {
    if (fm_type_sys_errno(tsys) != FM_TYPE_ERROR_OK) {
      PyErr_SetString(PyExc_RuntimeError, fm_type_sys_errmsg(tsys));
    } else if (fm_comp_sys_is_error(sys)) {
      PyErr_SetString(PyExc_RuntimeError, fm_comp_sys_error_msg(sys));
    }
    return nullptr;
  };

  return ExtractorComputation_new(comp, sys, graph);
}

PyObject *BinaryCompGen_NoArgs(PyObject *s, PyObject *arg, const char *name) {
  ExtractorComputation *self = (ExtractorComputation *)s;
  ExtractorComputation *other = (ExtractorComputation *)arg;

  bool a_comp = PyObject_TypeCheck(s, &ExtractorComputationType);
  bool b_comp = PyObject_TypeCheck(arg, &ExtractorComputationType);

  if (!a_comp && !b_comp) {
    PyErr_SetString(PyExc_TypeError, "None of the objects provided is"
                                     " an Extractor Computation");
    return nullptr;
  } else if (a_comp && !b_comp) {
    other = (ExtractorComputation *)ConstGen(arg, self->sys_, self->graph_);
    if (!other) {
      PyErr_SetString(PyExc_TypeError, "Second value provided could not "
                                       "be processed as an Extractor "
                                       "Computation");
      return nullptr;
    }
  } else if (!a_comp && b_comp) {
    self = (ExtractorComputation *)ConstGen(s, other->sys_, other->graph_);
    if (!self) {
      PyErr_SetString(PyExc_TypeError, "First value provided could not "
                                       "be processed as an Extractor "
                                       "Computation");
      return nullptr;
    }
  }

  if (self->graph_ != other->graph_) {
    PyErr_SetString(PyExc_RuntimeError, "Computations must belong to the "
                                        "same graph");
    return nullptr;
  }

  fm_comp_t *self_comp = self->comp_;
  fm_comp_t *other_comp = other->comp_;

  fm_comp_sys_t *sys = self->sys_;
  fm_type_sys_t *tsys = fm_type_sys_get(sys);
  fm_comp_graph *graph = self->graph_;

  auto *comp =
      fm_comp_decl(sys, graph, name, 2, nullptr, self_comp, other_comp);

  if (!comp) {
    if (fm_type_sys_errno(tsys) != FM_TYPE_ERROR_OK) {
      PyErr_SetString(PyExc_RuntimeError, fm_type_sys_errmsg(tsys));
    } else if (fm_comp_sys_is_error(sys)) {
      PyErr_SetString(PyExc_RuntimeError, fm_comp_sys_error_msg(sys));
    }
    return nullptr;
  };

  return ExtractorComputation_new(comp, sys, graph);
}

PyObject *FieldGen_NoArgs(PyObject *s, char *name) {
  if (!PyObject_TypeCheck(s, &ExtractorComputationType)) {
    PyErr_SetString(PyExc_TypeError, "First value provided is not an "
                                     "Extractor Computation");
    return nullptr;
  }

  ExtractorComputation *self = (ExtractorComputation *)s;

  fm_comp_t *self_comp = self->comp_;

  fm_comp_sys_t *sys = self->sys_;
  fm_type_sys_t *tsys = fm_type_sys_get(sys);
  fm_comp_graph *graph = self->graph_;

  auto *comp = fm_comp_decl(
      sys, graph, "field", 1,
      fm_tuple_type_get(tsys, 1, fm_cstring_type_get(tsys)), self_comp, name);

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

static PyObject *ExtractorComputation_richcompare(PyObject *obj1,
                                                  PyObject *obj2, int op) {
  PyObject *result = nullptr;

  switch (op) {
  case Py_LT:
    result = BinaryCompGen_NoArgs(obj1, obj2, "less");
    break;
  case Py_LE:
    result = BinaryCompGen_NoArgs(obj1, obj2, "less_equal");
    break;
  case Py_EQ:
    if (!PyObject_TypeCheck(obj1, &ExtractorComputationType) ||
        !PyObject_TypeCheck(obj2, &ExtractorComputationType)) {
      result = Py_False;
      Py_INCREF(result);
    } else {
      result = BinaryCompGen_NoArgs(obj1, obj2, "equal");
    }
    break;
  case Py_NE:
    if (!PyObject_TypeCheck(obj1, &ExtractorComputationType) ||
        !PyObject_TypeCheck(obj2, &ExtractorComputationType)) {
      result = Py_True;
      Py_INCREF(result);
    } else {
      result = BinaryCompGen_NoArgs(obj1, obj2, "not_equal");
    }
    break;
  case Py_GT:
    result = BinaryCompGen_NoArgs(obj1, obj2, "greater");
    break;
  case Py_GE:
    result = BinaryCompGen_NoArgs(obj1, obj2, "greater_equal");
    break;
  }
  return result;
}
