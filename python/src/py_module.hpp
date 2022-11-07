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

extern "C" {
#include "extractor/module.h"
}

#include "py_utils.hpp"
#include <Python.h>

typedef struct {
  PyObject_HEAD fm_type_sys_t *tsys_;
  fm_module_t *m_;
  fm_module_comp_t *comp_;
} ExtractorModuleComputation;

static void
ExtractorModuleComputation_dealloc(ExtractorModuleComputation *self) {
  Py_TYPE(self)->tp_free((PyObject *)self);
}

PyObject *BinaryModuleCompGen_NoArgs(PyObject *s, PyObject *arg,
                                     const char *name);

static PyObject *ExtractorModuleComputation_add(PyObject *s, PyObject *arg) {
  return BinaryModuleCompGen_NoArgs(s, arg, "add");
}

static PyObject *ExtractorModuleComputation_substract(PyObject *s,
                                                      PyObject *arg) {
  return BinaryModuleCompGen_NoArgs(s, arg, "diff");
}

static PyObject *ExtractorModuleComputation_multiply(PyObject *s,
                                                     PyObject *arg) {
  return BinaryModuleCompGen_NoArgs(s, arg, "mult");
}

static PyObject *ExtractorModuleComputation_div(PyObject *s, PyObject *arg) {
  return BinaryModuleCompGen_NoArgs(s, arg, "divide");
}

static PyObject *ExtractorModuleComputation_pow(PyObject *first,
                                                PyObject *second,
                                                PyObject *third) {
  // Third argument is ignored
  // Third argument should not be ignored once module
  // operation is included in Extractor
  return BinaryModuleCompGen_NoArgs(first, second, "pow");
}

static PyObject *ExtractorModuleComputation_and(PyObject *s, PyObject *arg) {
  return BinaryModuleCompGen_NoArgs(s, arg, "logical_and");
}

static PyObject *ExtractorModuleComputation_or(PyObject *s, PyObject *arg) {
  return BinaryModuleCompGen_NoArgs(s, arg, "logical_or");
}

PyObject *ModuleFieldGen_NoArgs(PyObject *s, char *name);

static PyObject *ExtractorModuleComputation_getattr(PyObject *obj, char *name) {
  return ModuleFieldGen_NoArgs(obj, name);
}

static PyNumberMethods ExtractorModuleComputation_numerical_methods[] = {
    ExtractorModuleComputation_add,       /*nb_add*/
    ExtractorModuleComputation_substract, /*nb_substract*/
    ExtractorModuleComputation_multiply,  /*nb_multiply*/
    NULL,                                 /*nb_remainder*/
    NULL,                                 /*nb_divmod*/
    ExtractorModuleComputation_pow,       /*nb_power*/
    NULL,                                 /*nb_negative*/
    NULL,                                 /*nb_positive*/
    NULL,                                 /*nb_absolute*/
    NULL,                                 /*nb_bool*/
    NULL,                                 /*nb_invert*/
    NULL,                                 /*nb_lshift*/
    NULL,                                 /*nb_rshift*/
    ExtractorModuleComputation_and,       /*nb_and*/
    NULL,                                 /*nb_xor*/
    ExtractorModuleComputation_or,        /*nb_or*/
    NULL,                                 /*nb_int*/
    NULL,                                 /*nb_reserved*/
    NULL,                                 /*nb_float*/
    NULL,                                 /*nb_inplace_add*/
    NULL,                                 /*nb_inplace_substract*/
    NULL,                                 /*nb_inplace_multiply*/
    NULL,                                 /*nb_inplace_remainder*/
    NULL,                                 /*nb_inplace_power*/
    NULL,                                 /*nb_inplace_lshift*/
    NULL,                                 /*nb_inplace_rshift*/
    NULL,                                 /*nb_inplace_and*/
    NULL,                                 /*nb_inplace_xor*/
    NULL,                                 /*nb_inplace_or*/
    NULL,                                 /*nb_floor_divide*/
    ExtractorModuleComputation_div,       /*nb_true_divide*/
    NULL,                                 /*nb_inplace_floor_divide*/
    NULL,                                 /*nb_inplace_true_divide*/
    NULL,                                 /*nb_index*/
    NULL,                                 /*nb_matrix_multiply*/
    NULL                                  /*nb_inplace_matrix_multiply*/
};

static PyTypeObject ExtractorModuleComputationType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Module.Computation", /* tp_name */
    sizeof(ExtractorModuleComputation),             /* tp_basicsize */
    0,                                              /* tp_itemsize */
    (destructor)ExtractorModuleComputation_dealloc, /* tp_dealloc */
    0,                                              /* tp_print */
    ExtractorModuleComputation_getattr,             /* tp_getattr */
    0,                                              /* tp_setattr */
    0,                                              /* tp_reserved */
    0,                                              /* tp_repr */
    ExtractorModuleComputation_numerical_methods,   /* tp_as_number */
    0,                                              /* tp_as_sequence */
    0,                                              /* tp_as_mapping */
    0,                                              /* tp_hash  */
    0,                                              /* tp_call */
    0,                                              /* tp_str */
    0,                                              /* tp_getattro */
    0,                                              /* tp_setattro */
    0,                                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,       /* tp_flags */
    "Module computation objects",                   /* tp_doc */
    0,                                              /* tp_traverse */
    0,                                              /* tp_clear */
    0,                                              /* tp_richcompare */
    0,                                              /* tp_weaklistoffset */
    0,                                              /* tp_iter */
    0,                                              /* tp_iternext */
    0,                                              /* tp_methods */
    0,                                              /* tp_members */
    0,                                              /* tp_getset */
    0,                                              /* tp_base */
    0,                                              /* tp_dict */
    0,                                              /* tp_descr_get */
    0,                                              /* tp_descr_set */
    0,                                              /* tp_dictoffset */
    0,                                              /* tp_init */
    0,                                              /* tp_alloc */
    0,                                              /* tp_new */
};

static PyObject *ExtractorModuleComputation_new(fm_type_sys_t *tsys,
                                                fm_module_t *m,
                                                fm_module_comp_t *comp) {
  PyTypeObject *type = &ExtractorModuleComputationType;
  ExtractorModuleComputation *self;

  self = (ExtractorModuleComputation *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->tsys_ = tsys;
  self->m_ = m;
  self->comp_ = comp;
  return (PyObject *)self;
}

PyObject *BinaryModuleCompGen_NoArgs(PyObject *s, PyObject *arg,
                                     const char *name) {
  if (!PyObject_TypeCheck(s, &ExtractorModuleComputationType)) {
    PyErr_SetString(PyExc_TypeError, "First value provided is not an "
                                     "Extractor Module Computation");
    return nullptr;
  }

  if (!PyObject_TypeCheck(arg, &ExtractorModuleComputationType)) {
    PyErr_SetString(PyExc_TypeError, "Second value provided is not an "
                                     "Extractor Module Computation");
    return nullptr;
  }

  ExtractorModuleComputation *self = (ExtractorModuleComputation *)s;
  ExtractorModuleComputation *other = (ExtractorModuleComputation *)arg;

  if (self->m_ != other->m_) {
    PyErr_SetString(PyExc_RuntimeError, "Module Computations must belong "
                                        "to the same module");
    return nullptr;
  }

  fm_module_comp_t *self_comp = self->comp_;
  fm_module_comp_t *other_comp = other->comp_;

  fm_type_sys_t *tsys = self->tsys_;
  fm_module_t *m = self->m_;
  fm_module_comp_t *inputs[2] = {self_comp, other_comp};

  auto *comp = fm_module_comp_add(m, name, nullptr, 2, inputs, nullptr);

  if (!comp) {
    if (fm_type_sys_errno(tsys) != FM_TYPE_ERROR_OK) {
      PyErr_SetString(PyExc_RuntimeError, fm_type_sys_errmsg(tsys));
    }
    return nullptr;
  };

  return (PyObject *)ExtractorModuleComputation_new(tsys, m, comp);
}

PyObject *ModuleFieldGen_NoArgs(PyObject *s, char *name) {
  if (!PyObject_TypeCheck(s, &ExtractorModuleComputationType)) {
    PyErr_SetString(PyExc_TypeError, "First value provided is not an "
                                     "Extractor Module Computation");
    return nullptr;
  }

  ExtractorModuleComputation *self = (ExtractorModuleComputation *)s;

  fm_module_comp_t *self_comp = self->comp_;

  fm_type_sys_t *tsys = self->tsys_;
  fm_module_t *m = self->m_;
  fm_module_comp_t *inputs[1] = {self_comp};

  auto *comp = fm_module_comp_add(
      m, "field", nullptr, 1, inputs,
      fm_tuple_type_get(tsys, 1, fm_cstring_type_get(tsys)), name);

  if (!comp) {
    if (fm_type_sys_errno(tsys) != FM_TYPE_ERROR_OK) {
      PyErr_SetString(PyExc_RuntimeError, fm_type_sys_errmsg(tsys));
    }
    return nullptr;
  };

  return (PyObject *)ExtractorModuleComputation_new(tsys, m, comp);
}

typedef struct {
  PyObject_HEAD fm_comp_sys_t *sys_;
  fm_module_t *m_;
  char *typename_;
} ExtractorModuleFeature;

static void ExtractorModuleFeature_dealloc(ExtractorModuleFeature *self) {
  free(self->typename_);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorModuleFeature_call(ExtractorModuleFeature *obj,
                                             PyObject *args, PyObject *kwds) {
  auto *name = [&]() -> const char * {
    if (!kwds)
      return nullptr;

    PyObject *pyname = PyDict_GetItemString(kwds, "name");
    if (!pyname) {
      PyErr_SetString(PyExc_TypeError, "need to specify name as a keyword "
                                       "argument");
      return nullptr;
    }

    if (!PyUnicode_Check(pyname)) {
      PyErr_SetString(PyExc_TypeError, "keyword argument 'name' must be a "
                                       "string");
      return nullptr;
    }
    return PyUnicode_AsUTF8(pyname);
  }();

  auto *stack = fm_arg_stack_alloc(MAX_EXTRACTOR_STACK_SIZE);
  fmc::scope_end_call free_stack([&stack]() { fm_arg_stack_free(stack); });

  comp_array inputs = std::vector<fm_module_comp_t *>();
  fm_type_decl_cp type = nullptr;
  auto *tsys = fm_type_sys_get(obj->sys_);
  int res = python_to_stack_arg(tsys, args, inputs, stack, &type);

  auto error = [&]() {
    if (fm_type_sys_errno(tsys) != FM_TYPE_ERROR_OK) {
      PyErr_SetString(PyExc_RuntimeError, fm_type_sys_errmsg(tsys));
    } else if (fm_comp_sys_is_error(obj->sys_)) {
      PyErr_SetString(PyExc_RuntimeError, fm_comp_sys_error_msg(obj->sys_));
    }
    return nullptr;
  };

  if (res == 1) {
    PyErr_SetString(PyExc_TypeError, "stack overflow");
    return nullptr;
  } else if (res == -1) {
    PyErr_SetString(PyExc_TypeError, "incorrect parameters");
    return nullptr;
  }
  auto &arr = std::get<std::vector<fm_module_comp_t *>>(inputs);
  auto *comp = fm_module_comp_add1(obj->m_, obj->typename_, name, arr.size(),
                                   arr.data(), type, fm_arg_stack_args(stack));

  if (!comp)
    return error();

  PyObject *result = nullptr;

  result = ExtractorModuleComputation_new(tsys, obj->m_, comp);

  //@act accordingly when volatile

  return (PyObject *)result;
}

static PyTypeObject ExtractorModuleFeatureType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Module.Feature", /* tp_name */
    sizeof(ExtractorModuleFeature),             /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)ExtractorModuleFeature_dealloc, /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash  */
    (ternaryfunc)ExtractorModuleFeature_call,   /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "Module Feature objects",                   /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    0,                                          /* tp_iter */
    0,                                          /* tp_iternext */
    0,                                          /* tp_methods */
    0,                                          /* tp_members */
    0,                                          /* tp_getset */
    0,                                          /* tp_base */
    0,                                          /* tp_dict */
    0,                                          /* tp_descr_get */
    0,                                          /* tp_descr_set */
    0,                                          /* tp_dictoffset */
    0,                                          /* tp_init */
    0,                                          /* tp_alloc */
    0,                                          /* tp_new */
};

static PyObject *ExtractorModuleFeature_new(const char *t, fm_comp_sys_t *s,
                                            fm_module_t *m) {
  PyTypeObject *type = &ExtractorModuleFeatureType;
  ExtractorModuleFeature *self;

  self = (ExtractorModuleFeature *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->sys_ = s;
  self->m_ = m;

  self->typename_ = strclone(t);

  return (PyObject *)self;
}

typedef struct {
  PyObject_HEAD fm_comp_sys_t *sys_;
  fm_module_t *m_;
} ExtractorModuleFeatures;

static void ExtractorModuleFeatures_dealloc(ExtractorModuleFeatures *self) {
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorModuleFeatures_getattr(ExtractorModuleFeatures *obj,
                                                 char *name) {
  return ExtractorModuleFeature_new(name, obj->sys_, obj->m_);
}

static PyTypeObject ExtractorModuleFeaturesType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Module.Features", /* tp_name */
    sizeof(ExtractorModuleFeatures),              /* tp_basicsize */
    0,                                            /* tp_itemsize */
    (destructor)ExtractorModuleFeatures_dealloc,  /* tp_dealloc */
    0,                                            /* tp_print */
    (getattrfunc)ExtractorModuleFeatures_getattr, /* tp_getattr */
    0,                                            /* tp_setattr */
    0,                                            /* tp_reserved */
    0,                                            /* tp_repr */
    0,                                            /* tp_as_number */
    0,                                            /* tp_as_sequence */
    0,                                            /* tp_as_mapping */
    0,                                            /* tp_hash  */
    0,                                            /* tp_call */
    0,                                            /* tp_str */
    0,                                            /* tp_getattro */
    0,                                            /* tp_setattro */
    0,                                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,     /* tp_flags */
    "Module Features objects",                    /* tp_doc */
    0,                                            /* tp_traverse */
    0,                                            /* tp_clear */
    0,                                            /* tp_richcompare */
    0,                                            /* tp_weaklistoffset */
    0,                                            /* tp_iter */
    0,                                            /* tp_iternext */
    0,                                            /* tp_methods */
    0,                                            /* tp_members */
    0,                                            /* tp_getset */
    0,                                            /* tp_base */
    0,                                            /* tp_dict */
    0,                                            /* tp_descr_get */
    0,                                            /* tp_descr_set */
    0,                                            /* tp_dictoffset */
    0,                                            /* tp_init */
    0,                                            /* tp_alloc */
    0,                                            /* tp_new */
};

static PyObject *ExtractorModuleFeatures_new(fm_comp_sys_t *sys,
                                             fm_module_t *m) {
  PyTypeObject *type = &ExtractorModuleFeaturesType;
  ExtractorModuleFeatures *self;

  self = (ExtractorModuleFeatures *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->sys_ = sys;
  self->m_ = m;

  return (PyObject *)self;
}

typedef struct {
  PyObject_HEAD ExtractorModuleFeatures *features;
} ExtractorModule;

static void ExtractorModule_dealloc(ExtractorModule *self) {
  Py_XDECREF(self->features);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorModule_getfeatures(ExtractorModule *self,
                                             void *closure) {
  Py_INCREF(self->features);
  return (PyObject *)self->features;
}

static int ExtractorModule_setfeatures(ExtractorModule *self, PyObject *value,
                                       void *closure) {
  PyErr_SetString(PyExc_TypeError, "cannot set the features object");
  return -1;
}

static PyGetSetDef ExtractorModule_getseters[] = {
    {(char *)"features", (getter)ExtractorModule_getfeatures,
     (setter)ExtractorModule_setfeatures,
     (char *)"Returns The Extractor feature generator object for the module.\n"
             "The returned object can be used to add features to the module.",
     NULL},
    {NULL, NULL, NULL, NULL, NULL} /* Sentinel */
};

static PyObject *ExtractorModule_declare_outputs(ExtractorModule *module_obj,
                                                 PyObject *args) {
  int n = PyTuple_Size(args);

  if (!n) {
    PyErr_SetString(PyExc_RuntimeError, "at least one output must be "
                                        "provided");
    return nullptr;
  }

  vector<fm_module_comp_t *> outputs(n);
  for (int i = 0; i < n; ++i) {
    outputs[i] =
        ((ExtractorModuleComputation *)PyTuple_GetItem(args, i))->comp_;
  }

  bool success =
      fm_module_outs_set(module_obj->features->m_, n, outputs.data());

  if (!success) {
    PyErr_SetString(PyExc_RuntimeError, "unable to set outputs in module");
    return nullptr;
  }

  Py_RETURN_NONE;
}

static PyMethodDef ExtractorModule_methods[] = {
    {"declare_outputs", (PyCFunction)ExtractorModule_declare_outputs,
     METH_VARARGS,
     "Sets the desired outputs of module to the specified features.\n"
     "The specified output features must be created using the feature "
     "generator from the Extractor module where they are set as outputs."},
    {NULL} /* Sentinel */
};

static PyTypeObject ExtractorModuleType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Module", /* tp_name */
    sizeof(ExtractorModule),                           /* tp_basicsize */
    0,                                                 /* tp_itemsize */
    (destructor)ExtractorModule_dealloc,               /* tp_dealloc */
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
    "Module objects",                                  /* tp_doc */
    0,                                                 /* tp_traverse */
    0,                                                 /* tp_clear */
    0,                                                 /* tp_richcompare */
    0,                                                 /* tp_weaklistoffset */
    0,                                                 /* tp_iter */
    0,                                                 /* tp_iternext */
    ExtractorModule_methods,                           /* tp_methods */
    0,                                                 /* tp_members */
    ExtractorModule_getseters,                         /* tp_getset */
    0,                                                 /* tp_base */
    0,                                                 /* tp_dict */
    0,                                                 /* tp_descr_get */
    0,                                                 /* tp_descr_set */
    0,                                                 /* tp_dictoffset */
    0,                                                 /* tp_init */
    0,                                                 /* tp_alloc */
    0,                                                 /* tp_new */
};

PyObject *ExtractorModule_new(fm_comp_sys_t *sys, fm_module_t *module) {
  PyTypeObject *type = &ExtractorModuleType;
  ExtractorModule *self;

  self = (ExtractorModule *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->features =
      (ExtractorModuleFeatures *)ExtractorModuleFeatures_new(sys, module);

  return (PyObject *)self;
}
