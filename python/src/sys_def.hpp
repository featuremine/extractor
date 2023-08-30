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
 * @file py_comp_sys.hpp
 * @author Maxim Trokhimtchouk
 * @date 5 Oct 2017
 * @brief Python extension for extractor library
 *
 * This file contains Python C extention for extractor library
 */

#pragma once

#include "extractor/comp_def.h"
#include "extractor/python/extractor.h"
#include "extractor/python/system.hpp"
#include "extractor/std_comp.h"

#include "custom.hpp"
#include "module.hpp"
#include "pandas_play.hpp"
#include <Python.h>
#include <memory>
#include <string>

ExtractorSystem *ExtractorSystem_lazy(ExtractorSystem *obj) {
  auto *self = (ExtractorSystem *)obj;
  if (self->sys == nullptr) {
    char *errmsg;
    self->sys = fm_comp_sys_new(&errmsg);
    if (!self->sys) {
      PyErr_SetString(PyExc_RuntimeError, errmsg);
      free(errmsg);
      return nullptr;
    }
    fm_comp_sys_std_comp(self->sys);
    fm_comp_sys_py_comp(self->sys);
    self->to_delete = true;
    for (auto &&comp : self->custom) {
      if (!fm_comp_type_add(obj->sys, &(obj->custom.back()))) {
        PyErr_SetString(PyExc_TypeError, "Unable to add custom operator");
        return nullptr;
      }
    }
  }

  return self;
}

static PyObject *ExtractorSystem_extend(PyObject *self, PyObject *args,
                                        PyObject *keywds) {
  PyObject *py_class = nullptr;
  char *comp_name = nullptr;

  static char *kwlist[] = {(char *)"class", (char *)"name", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "Os", kwlist, &py_class,
                                   &comp_name)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse keywords");
    return nullptr;
  }

  if (!PyType_Check(py_class)) {
    PyErr_SetString(PyExc_TypeError, "Argument provided must be a "
                                     "class type");
    return nullptr;
  }

  auto obj = ExtractorSystem_lazy((ExtractorSystem *)self);
  Py_INCREF(py_class);
  obj->custom.emplace_back(fm_comp_def_t{strclone(comp_name),
                                         &fm_comp_custom_gen,
                                         &fm_comp_custom_destroy, py_class});

  if (obj->sys) {
    if (!fm_comp_type_add(obj->sys, &(obj->custom.back()))) {
      PyErr_SetString(PyExc_TypeError, "Unable to add custom operator");
      return nullptr;
    }
  }
  Py_RETURN_NONE;
}

static void ExtractorSystem_dealloc(ExtractorSystem *self) {
  if (self->sys != nullptr && self->to_delete) {
    fm_comp_sys_del(self->sys);
    self->sys = nullptr;
  }
  for (auto &&comp : self->custom) {
    free((void *)comp.name);
    Py_DECREF(comp.closure);
  }
  self->~ExtractorSystem();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorSystem_load_ext(ExtractorSystem *obj,
                                          PyObject *args) {
  const char *name;
  const char *path;
  if (!PyArg_ParseTuple(args, "ss", &name, &path)) {
    PyErr_SetString(PyExc_RuntimeError, "expecting module name and path");
    return nullptr;
  }
  auto *self = ExtractorSystem_lazy(obj);
  if (!self)
    return nullptr;
  auto *sys = self->sys;
  if (!fm_comp_sys_ext_load(sys, name, path)) {
    PyErr_SetString(PyExc_RuntimeError, fm_comp_sys_error_msg(sys));
    return nullptr;
  }
  Py_RETURN_NONE;
}

static PyObject *ExtractorSystem_comp_graph(ExtractorSystem *obj) {
  auto *self = ExtractorSystem_lazy(obj);
  if (!self)
    return nullptr;
  auto *sys = self->sys;
  if (auto *graph = fm_comp_graph_get(sys); graph) {
    if (auto *py_g = ExtractorGraph_py_new((PyObject *)self, sys, graph, true);
        py_g) {
      return py_g;
    } else {
      return nullptr;
    }
  } else {
    PyErr_SetString(PyExc_RuntimeError, fm_comp_sys_error_msg(sys));
    return nullptr;
  }
}

static PyObject *ExtractorSystem_sample_value(ExtractorSystem *obj,
                                              PyObject *args) {
  const char *name;
  if (!PyArg_ParseTuple(args, "s", &name)) {
    PyErr_SetString(PyExc_RuntimeError, "expecting sample name");
    return nullptr;
  }
  double val;
  if (!fm_comp_sys_sample_value(obj->sys, name, &val)) {
    PyErr_SetString(PyExc_RuntimeError, "unable to find sample with the "
                                        "given name");
    return nullptr;
  }
  return PyFloat_FromDouble(val);
}

static PyObject *ExtractorSystem_module(ExtractorSystem *obj, PyObject *args,
                                        PyObject *keywds) {
  int ninps;
  const char *name = nullptr;
  static char *kwlist[] = {(char *)"inputs", (char *)"name", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "i|s", kwlist, &ninps,
                                   &name)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse keywords");
    return nullptr;
  }

  auto *self = ExtractorSystem_lazy(obj);
  if (!self)
    return nullptr;
  auto *sys = self->sys;
  vector<fm_module_comp_t *> inputs(ninps);
  if (auto *m = fm_module_new(name, ninps, inputs.data()); m) {
    PyObject *input_placeholders = PyList_New(ninps);
    for (int i = 0; i < ninps; ++i) {
      PyObject *identity_obj =
          ExtractorModuleComputation_new(fm_type_sys_get(sys), m, inputs[i]);
      PyList_SetItem(input_placeholders, i, identity_obj);
    }
    // @todo return a tuple of input placeholders and the actual module
    return Py_BuildValue("OO", ExtractorModule_new(sys, m), input_placeholders);
    // return ExtractorModule_new(sys, m);
  } else {
    PyErr_SetString(PyExc_RuntimeError, fm_comp_sys_error_msg(sys));
    return nullptr;
  }
}

static PyMethodDef ExtractorSystem_methods[] = {
    {"load_ext", (PyCFunction)ExtractorSystem_load_ext, METH_VARARGS,
     "Loads Extractor extension module into the system.\n"
     "Receives as arguments the name and path of the extension module to "
     "load."},
    {"comp_graph", (PyCFunction)ExtractorSystem_comp_graph, METH_NOARGS,
     "Creates a new computational graph.\n"
     "Does not require any arguments."},
    {"sample_value", (PyCFunction)ExtractorSystem_sample_value, METH_VARARGS,
     "Returns the current value of the desired sample.\n"
     "Receives as the only argument the desired sample name."},
    {"module", (PyCFunction)ExtractorSystem_module,
     METH_VARARGS | METH_KEYWORDS,
     "Creates a new module.\n"
     "Receives as the first argument the number of inputs the module will be "
     "required to use.\n"
     "Receives as an optional second argument the name associated with the "
     "module."},
    {"extend", (PyCFunction)ExtractorSystem_extend,
     METH_VARARGS | METH_KEYWORDS,
     "Extends computations with specified custom computation.\n"
     "Receives as the first argument the custom computation class type.\n"
     "Receives as the second argument the desired name for the custom "
     "computation.\n"
     "The computation names must be unique."},
    {NULL} /* Sentinel */
};

static PyTypeObject ExtractorSystemType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.System", /* tp_name */
    sizeof(ExtractorSystem),                           /* tp_basicsize */
    0,                                                 /* tp_itemsize */
    (destructor)ExtractorSystem_dealloc,               /* tp_dealloc */
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
    "System objects",                                  /* tp_doc */
    0,                                                 /* tp_traverse */
    0,                                                 /* tp_clear */
    0,                                                 /* tp_richcompare */
    0,                                                 /* tp_weaklistoffset */
    0,                                                 /* tp_iter */
    0,                                                 /* tp_iternext */
    ExtractorSystem_methods,                           /* tp_methods */
    0,                                                 /* tp_members */
    0,                                                 /* tp_getset */
    0,                                                 /* tp_base */
    0,                                                 /* tp_dict */
    0,                                                 /* tp_descr_get */
    0,                                                 /* tp_descr_set */
    0,                                                 /* tp_dictoffset */
    0,                                                 /* tp_init */
    0,                                                 /* tp_alloc */
    0,                                                 /* tp_new */
};

PyObject *ExtractorSystem_new() {
  PyTypeObject *type = &ExtractorSystemType;
  ExtractorSystem *self;

  self = (ExtractorSystem *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  return (PyObject *)ExtractorSystem_lazy(self);
}

bool ExtractorSystem_Check(PyObject *obj) {
  return PyObject_TypeCheck(obj, &ExtractorSystemType);
}

fm_comp_sys_t *ExtractorSystem_get(PyObject *obj) {
  if (!ExtractorSystem_Check(obj))
    return nullptr;
  auto *py_sys = ExtractorSystem_lazy((ExtractorSystem *)obj);
  if (!py_sys)
    return nullptr;
  return py_sys->sys;
}
