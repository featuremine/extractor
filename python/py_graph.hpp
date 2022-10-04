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
#include "py_context.h"
#include "comp_graph.h"
}

#include "py_frame.hpp"
#include "py_types.hpp"
#include "py_utils.hpp"
#include "fmc++/mpl.hpp"

#include <Python.h>
#include <datetime.h>
#include <vector>

typedef struct {
  PyObject_HEAD fm_comp_sys_t *sys_;
  fm_comp_graph_t *graph_;
  char *typename_;
} ExtractorFeature;

static void ExtractorFeature_dealloc(ExtractorFeature *self) {
  free(self->typename_);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorFeature_call(ExtractorFeature *obj, PyObject *args,
                                       PyObject *kwds) {
  auto *name = [&]() -> const char * {
    if (!kwds)
      return nullptr;

    PyObject *pyname = PyDict_GetItemString(kwds, "name");
    if (!pyname) {
      PyErr_SetString(PyExc_TypeError, "need to specify name as a keyword "
                                       "argument");
      return nullptr;
    }
    if (pyname == Py_None) {
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

  comp_array inputs = std::vector<fm_comp_t *>();
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
  auto inps = std::get<std::vector<fm_comp_t *>>(inputs);
  auto *comp =
      fm_comp_decl4(obj->sys_, obj->graph_, obj->typename_, name, inps.size(),
                    inps.data(), type, fm_arg_stack_args(stack));

  if (!comp)
    return error();

  PyObject *result = nullptr;

  auto vol_val = fm_ctx_def_volatile_get(fm_comp_ctx_def(comp));

  if (vol_val == 0)
    result = ExtractorComputation_new(comp, obj->sys_, obj->graph_);
  else {
    result = PyList_New(vol_val);
    for (size_t i = 0; i < vol_val; ++i) {
      auto *temp_comp = fm_comp_decl(obj->sys_, obj->graph_, "identity", 1,
                                     fm_tuple_type_get(tsys, 0), comp);
      if (!temp_comp) {
        Py_XDECREF(result);
        return error();
      }

      PyObject *identity_obj =
          ExtractorComputation_new(temp_comp, obj->sys_, obj->graph_);
      PyList_SetItem(result, i, identity_obj);
    }
  }
  return (PyObject *)result;
}

static PyTypeObject ExtractorFeatureType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Feature", /* tp_name */
    sizeof(ExtractorFeature),                           /* tp_basicsize */
    0,                                                  /* tp_itemsize */
    (destructor)ExtractorFeature_dealloc,               /* tp_dealloc */
    0,                                                  /* tp_print */
    0,                                                  /* tp_getattr */
    0,                                                  /* tp_setattr */
    0,                                                  /* tp_reserved */
    0,                                                  /* tp_repr */
    0,                                                  /* tp_as_number */
    0,                                                  /* tp_as_sequence */
    0,                                                  /* tp_as_mapping */
    0,                                                  /* tp_hash  */
    (ternaryfunc)ExtractorFeature_call,                 /* tp_call */
    0,                                                  /* tp_str */
    0,                                                  /* tp_getattro */
    0,                                                  /* tp_setattro */
    0,                                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,           /* tp_flags */
    "Feature objects",                                  /* tp_doc */
    0,                                                  /* tp_traverse */
    0,                                                  /* tp_clear */
    0,                                                  /* tp_richcompare */
    0,                                                  /* tp_weaklistoffset */
    0,                                                  /* tp_iter */
    0,                                                  /* tp_iternext */
    0,                                                  /* tp_methods */
    0,                                                  /* tp_members */
    0,                                                  /* tp_getset */
    0,                                                  /* tp_base */
    0,                                                  /* tp_dict */
    0,                                                  /* tp_descr_get */
    0,                                                  /* tp_descr_set */
    0,                                                  /* tp_dictoffset */
    0,                                                  /* tp_init */
    0,                                                  /* tp_alloc */
    0,                                                  /* tp_new */
};

static PyObject *ExtractorFeature_new(const char *t, fm_comp_sys_t *s,
                                      fm_comp_graph_t *g) {
  PyTypeObject *type = &ExtractorFeatureType;
  ExtractorFeature *self;

  self = (ExtractorFeature *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->sys_ = s;
  self->graph_ = g;

  self->typename_ = strclone(t);

  return (PyObject *)self;
}

// here we define Extractor Features interface

static void ExtractorFeatures_dealloc(ExtractorFeatures *self) {
  if (self->to_delete) {
    fm_comp_graph_remove(self->sys_, self->graph_);
  }
  Py_XDECREF(self->py_sys_);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorFeatures_getattr(ExtractorFeatures *obj, char *name) {
  return ExtractorFeature_new(name, obj->sys_, obj->graph_);
}

static PyTypeObject ExtractorFeaturesType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Features", /* tp_name */
    sizeof(ExtractorFeatures),                           /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    (destructor)ExtractorFeatures_dealloc,               /* tp_dealloc */
    0,                                                   /* tp_print */
    (getattrfunc)ExtractorFeatures_getattr,              /* tp_getattr */
    0,                                                   /* tp_setattr */
    0,                                                   /* tp_reserved */
    0,                                                   /* tp_repr */
    0,                                                   /* tp_as_number */
    0,                                                   /* tp_as_sequence */
    0,                                                   /* tp_as_mapping */
    0,                                                   /* tp_hash  */
    0,                                                   /* tp_call */
    0,                                                   /* tp_str */
    0,                                                   /* tp_getattro */
    0,                                                   /* tp_setattro */
    0,                                                   /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,            /* tp_flags */
    "Features objects",                                  /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    0,                                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    0,                                                   /* tp_iter */
    0,                                                   /* tp_iternext */
    0,                                                   /* tp_methods */
    0,                                                   /* tp_members */
    0,                                                   /* tp_getset */
    0,                                                   /* tp_base */
    0,                                                   /* tp_dict */
    0,                                                   /* tp_descr_get */
    0,                                                   /* tp_descr_set */
    0,                                                   /* tp_dictoffset */
    0,                                                   /* tp_init */
    0,                                                   /* tp_alloc */
    0,                                                   /* tp_new */
};

static PyObject *ExtractorFeatures_new(PyObject *py_sys, fm_comp_sys_t *sys,
                                       fm_comp_graph_t *graph, bool to_delete) {
  PyTypeObject *type = &ExtractorFeaturesType;
  ExtractorFeatures *self;

  self = (ExtractorFeatures *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->sys_ = sys;
  self->py_sys_ = py_sys;
  self->graph_ = graph;
  self->to_delete = to_delete;

  Py_XINCREF(self->py_sys_);

  return (PyObject *)self;
}

static void ExtractorFeatureIter_dealloc(ExtractorFeatureIter *self) {
  Py_XDECREF(self->graph_);
  self->~ExtractorFeatureIter();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorFeatureIter_iter(ExtractorFeatureIter *self) {
  Py_XINCREF(self);
  return (PyObject *)self;
}

static PyObject *ExtractorFeatureIter_iternext(ExtractorFeatureIter *self) {
  if (self->iter_ != self->nodes_.size()) {

    auto comp_node = fm_comp_node_obj(self->nodes_[self->iter_]);

    PyObject *ptr =
        ExtractorComputation_new(comp_node, self->graph_->features->sys_,
                                 self->graph_->features->graph_);

    PyObject *ret = PyTuple_New(2);
    PyTuple_SetItem(ret, 0, PyUnicode_FromString(fm_comp_name(comp_node)));
    PyTuple_SetItem(ret, 1, ptr);

    self->iter_++;
    return ret;
  }

  PyErr_SetNone(PyExc_StopIteration);
  return NULL;
}

static PyTypeObject ExtractorFeatureIterType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.FeatureIter", /* tp_name */
    sizeof(ExtractorFeatureIter),                           /* tp_basicsize */
    0,                                                      /* tp_itemsize */
    (destructor)ExtractorFeatureIter_dealloc,               /* tp_dealloc */
    0,                                                      /* tp_print */
    0,                                                      /* tp_getattr */
    0,                                                      /* tp_setattr */
    0,                                                      /* tp_reserved */
    0,                                                      /* tp_repr */
    0,                                                      /* tp_as_number */
    0,                                                      /* tp_as_sequence */
    0,                                                      /* tp_as_mapping */
    0,                                                      /* tp_hash  */
    0,                                                      /* tp_call */
    0,                                                      /* tp_str */
    0,                                                      /* tp_getattro */
    0,                                                      /* tp_setattro */
    0,                                                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,               /* tp_flags */
    "Extractor features iterator",                          /* tp_doc */
    0,                                                      /* tp_traverse */
    0,                                                      /* tp_clear */
    0,                                                      /* tp_richcompare */
    0,                                           /* tp_weaklistoffset */
    (getiterfunc)ExtractorFeatureIter_iter,      /* tp_iter */
    (iternextfunc)ExtractorFeatureIter_iternext, /* tp_iternext */
    0,                                           /* tp_methods */
    0,                                           /* tp_members */
    0,                                           /* tp_getset */
    0,                                           /* tp_base */
    0,                                           /* tp_dict */
    0,                                           /* tp_descr_get */
    0,                                           /* tp_descr_set */
    0,                                           /* tp_dictoffset */
    0,                                           /* tp_init */
    0,                                           /* tp_alloc */
    0,                                           /* tp_new */
};

ExtractorFeatureIter *ExtractorFeatureIter_new(ExtractorGraph *graph) {
  PyTypeObject *subtype = &ExtractorFeatureIterType;
  auto *self = (ExtractorFeatureIter *)subtype->tp_alloc(subtype, 0);
  Py_INCREF(graph);
  self->graph_ = graph;
  self->iter_ = 0;
  auto count = fm_comp_graph_nodes_size(graph->features->graph_);
  int i = 0;
  for (auto it = fm_comp_graph_nodes_begin(graph->features->graph_);
       it != fm_comp_graph_nodes_end(graph->features->graph_); ++i, ++it) {
    self->nodes_.push_back(*it);
  }
  if (count != fm_comp_subgraph_stable_top_sort(graph->features->graph_, count,
                                                self->nodes_.data())) {
    Py_XDECREF(self);
    PyErr_SetString(
        PyExc_RuntimeError,
        "Unable to sort graph nodes, circular dependencies were found.");
    return nullptr;
  }
  return self;
}

/////////////////////////////////////////////////////////////////
// here we define Extractor Graph object

static void ExtractorGraph_dealloc(ExtractorGraph *self) {
  for (auto *clbck : self->clbcks) {
    Py_XDECREF(clbck);
  }
  Py_XDECREF(self->features);
  self->~ExtractorGraph();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorGraph_getfeatures(ExtractorGraph *self,
                                            void *closure) {
  Py_INCREF(self->features);
  return (PyObject *)self->features;
}

static int ExtractorGraph_setfeatures(ExtractorGraph *self, PyObject *value,
                                      void *closure) {
  PyErr_SetString(PyExc_TypeError, "cannot set the features object");
  return -1;
}

static PyGetSetDef ExtractorGraph_getseters[] = {
    {(char *)"features", (getter)ExtractorGraph_getfeatures,
     (setter)ExtractorGraph_setfeatures,
     (char *)"Returns The Extractor feature generator object.\n"
             "The returned object can be used to add new feature instances to "
             "the graph.",
     NULL},
    {NULL, NULL, NULL, NULL, NULL} /* Sentinel */
};

PyObject *ExtractorStreamContext_new(fm_stream_ctx_t *ctx,
                                     ExtractorGraph *graph);

static PyObject *ExtractorGraph_stream_ctx(ExtractorGraph *self) {
  auto *sys = self->features->sys_;
  auto *graph = self->features->graph_;
  if (auto *ctx = fm_stream_ctx_get(sys, graph); ctx) {
    return ExtractorStreamContext_new(ctx, self);
  } else {
    PyErr_SetString(PyExc_RuntimeError, fm_comp_sys_error_msg(sys));
    return nullptr;
  }
}

static PyObject *ExtractorGraph_extend(ExtractorGraph *graph_obj,
                                       PyObject *args) {

  int n = PyTuple_Size(args);

  if (!n) {
    PyErr_SetString(PyExc_RuntimeError, "the module and inputs used to "
                                        "extend the graph must be provided "
                                        "as arguments");
    return nullptr;
  }

  auto *module_obj = PyTuple_GetItem(args, 0);

  if (!PyObject_TypeCheck(module_obj, &ExtractorModuleType)) {
    PyErr_SetString(PyExc_RuntimeError, "expecting a module object as "
                                        "first argument");
    return nullptr;
  }

  auto *module = ((ExtractorModule *)module_obj)->features->m_;
  int ninps = fm_module_inps_size(module);

  if (n != 1 + ninps) {
    std::string err_msg("incorrect number of inputs, expecting ");
    err_msg.append(std::to_string(ninps));
    PyErr_SetString(PyExc_RuntimeError, err_msg.c_str());
    return nullptr;
  }

  vector<fm_comp_t *> inputs(ninps);

  for (int i = 0; i < ninps; ++i) {
    inputs[i] = ((ExtractorComputation *)PyTuple_GetItem(args, i + 1))->comp_;
  }

  auto *sys = (graph_obj)->features->sys_;
  auto *graph = (graph_obj)->features->graph_;

  int nouts = fm_module_outs_size(module);
  vector<fm_comp_t *> outputs(nouts);

  bool inst = fm_module_inst(sys, graph, module, inputs.data(), outputs.data());

  if (!inst) {
    std::string error_msg = "unable to instantiate module in "
                            "graph: ";
    error_msg += fm_comp_sys_error_msg(sys);
    PyErr_SetString(PyExc_RuntimeError, error_msg.c_str());
    return nullptr;
  }

  PyObject *output_list = PyList_New(nouts);
  for (int i = 0; i < nouts; ++i) {
    PyList_SetItem(output_list, i,
                   ExtractorComputation_new(outputs[i], sys, graph));
  }

  return output_list;
}

static void comp_clbck(const fm_frame_t *frame, void *closure,
                       fm_call_ctx_t *ctx) {
  auto *cl = (PyObject *)closure;
  auto *obj = PyTuple_New(1);
  PyTuple_SET_ITEM(obj, 0, ExtractorFrame_new(frame));
  PyObject_CallObject(cl, obj);
  Py_XDECREF(obj);
  set_python_error(ctx->exec);
}

static PyObject *ExtractorGraph_callback(ExtractorGraph *self, PyObject *args) {
  PyObject *raw_obj = nullptr;
  PyObject *clbck;

  if (!PyArg_ParseTuple(args, "OO", &raw_obj, &clbck)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse args");
    return nullptr;
  }

  if (!PyObject_TypeCheck(raw_obj, &ExtractorComputationType)) {
    PyErr_SetString(PyExc_TypeError, "Argument provided must be an "
                                     "Extractor Computation");
    return nullptr;
  }

  if (!PyCallable_Check(clbck)) {
    PyErr_SetString(PyExc_TypeError,
                    "callback provided must be a callable object");
    return nullptr;
  }
  Py_XINCREF(clbck);
  self->clbcks.emplace_back(clbck);

  auto *s = (ExtractorComputation *)raw_obj;

  fm_comp_clbck_set(s->comp_, &comp_clbck, clbck);

  Py_RETURN_NONE;
}

static PyObject *ExtractorGraph_get_ref(ExtractorGraph *self, PyObject *args) {
  PyObject *raw_obj = nullptr;

  if (!PyArg_ParseTuple(args, "O", &raw_obj)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse args");
    return nullptr;
  }

  if (PyObject_TypeCheck(raw_obj, &ExtractorComputationType)) {
    auto comp = (ExtractorComputation *)raw_obj;
    return ExtractorResultRef_new(fm_result_ref_get(comp->comp_));
  } else if (PyUnicode_Check(raw_obj)) {
    auto *comp =
        fm_comp_find(self->features->graph_, PyUnicode_AsUTF8(raw_obj));
    if (!comp) {
      PyErr_SetString(PyExc_ValueError, "Unable to find computation in graph");
      return nullptr;
    }
    return ExtractorResultRef_new(fm_result_ref_get(comp));
  }

  PyErr_SetString(PyExc_TypeError, "Argument provided must be an "
                                   "Extractor Computation or a string");
  return nullptr;
}

static PyObject *ExtractorGraph_name(ExtractorGraph *self, PyObject *args) {
  PyObject *raw_obj = nullptr;

  if (!PyArg_ParseTuple(args, "O", &raw_obj)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse args");
    return nullptr;
  }

  if (!PyObject_TypeCheck(raw_obj, &ExtractorComputationType)) {
    PyErr_SetString(PyExc_TypeError, "Argument provided must be an "
                                     "Extractor Computation");
    return nullptr;
  }

  auto comp = (ExtractorComputation *)raw_obj;

  return PyUnicode_FromString(fm_comp_name(comp->comp_));
}

static PyObject *ExtractorGraph_inputs(ExtractorGraph *self, PyObject *args) {
  PyObject *raw_obj = nullptr;

  if (!PyArg_ParseTuple(args, "O", &raw_obj)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse args");
    return nullptr;
  }

  if (!PyObject_TypeCheck(raw_obj, &ExtractorComputationType)) {
    PyErr_SetString(PyExc_TypeError, "Argument provided must be an "
                                     "Extractor Computation");
    return nullptr;
  }

  auto comp = (ExtractorComputation *)raw_obj;

  auto node = fm_comp_node_ptr(comp->comp_);

  auto sz = fm_comp_node_inps_size(node);

  auto ret = PyList_New(sz);

  unsigned i = 0;
  for (auto inps_it = fm_comp_node_inps_begin(node);
       inps_it != fm_comp_node_inps_end(node); ++i, ++inps_it) {
    auto inp = fm_comp_node_obj(*inps_it);
    auto py_inp =
        ExtractorComputation_new(inp, self->features->sys_, comp->graph_);
    PyList_SetItem(ret, i, py_inp);
  }

  return ret;
}

static PyObject *ExtractorGraph_find(ExtractorGraph *self, PyObject *args,
                                     PyObject *keywds) {
  char *comp_name = nullptr;

  static char *kwlist[] = {(char *)"name", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "s", kwlist, &comp_name)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse keywords");
    return nullptr;
  }

  auto *graph = self->features->graph_;
  auto *comp = fm_comp_find(graph, comp_name);
  if (!comp) {
    PyErr_SetString(PyExc_ValueError, "Unable to find computation in graph");
    return nullptr;
  }
  return ExtractorComputation_new(comp, self->features->sys_, graph);
}

static PyMethodDef ExtractorGraph_methods[] = {
    {"stream_ctx", (PyCFunction)ExtractorGraph_stream_ctx, METH_NOARGS,
     "Creates a new computational graph.\n"
     "No arguments are required to create a new graph."},
    {"extend", (PyCFunction)ExtractorGraph_extend, METH_VARARGS,
     "Instantiates the module in a given graph.\n"
     "As a first argument the desired module to instanciate must be provided\n"
     "The number of inputs provided must match the number of inputs in the "
     "provided module.\n"
     "If the module has no inputs, no additional arguments are required."},
    {"callback", (PyCFunction)ExtractorGraph_callback, METH_VARARGS,
     "Sets the desired callback to the specified computation.\n"
     "The first argument provided must be the computation we would like to "
     "attach a callback to.\n"
     "The second argument provided must be a callable object that receives the "
     "frame of the desired computation as a single argument\n."},
    {"get_ref", (PyCFunction)ExtractorGraph_get_ref, METH_VARARGS,
     "Obtain the frame reference from a given computation.\n"
     "This method must receive as a single argument the desired computation "
     "instance used to obtain the frame reference."},
    {"name", (PyCFunction)ExtractorGraph_name, METH_VARARGS,
     "Obtain the name from a given computation.\n"
     "This method must receive as a single argument the desired computation "
     "instance used to obtain the name."},
    {"find", (PyCFunction)ExtractorGraph_find, METH_VARARGS | METH_KEYWORDS,
     "Return computation by the given name.\n"
     "This method must receive as a single argument the desired computation's "
     "name used to obtain the instance."},
    {"inputs", (PyCFunction)ExtractorGraph_inputs, METH_VARARGS | METH_KEYWORDS,
     "Return computation inputs as a list.\n"
     "This method must receive as a single argument the desired computation's "
     "name used to obtain the instance."},
    {NULL} /* Sentinel */
};

static PyObject *ExtractorGraph_iter(ExtractorGraph *self) {
  auto ret = ExtractorFeatureIter_new(self);
  return (PyObject *)ret;
}

static int ExtractorGraph_traverse(PyObject *obj, visitproc visit, void *arg) {
  auto *self = (ExtractorGraph *)obj;
  for (auto *clbck : self->clbcks) {
    Py_VISIT(clbck);
  }
  return 0;
}

static int ExtractorGraph_clear(PyObject *obj) {
  auto *self = (ExtractorGraph *)obj;
  for (auto *clbck : self->clbcks) {
    Py_XDECREF(clbck);
  }
  self->clbcks.clear();
  return 0;
}

static PyTypeObject ExtractorGraphType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Graph", /* tp_name */
    sizeof(ExtractorGraph),                           /* tp_basicsize */
    0,                                                /* tp_itemsize */
    (destructor)ExtractorGraph_dealloc,               /* tp_dealloc */
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
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE |
        Py_TPFLAGS_HAVE_GC,           /* tp_flags */
    "Graph objects",                  /* tp_doc */
    ExtractorGraph_traverse,          /* tp_traverse */
    ExtractorGraph_clear,             /* tp_clear */
    0,                                /* tp_richcompare */
    0,                                /* tp_weaklistoffset */
    (getiterfunc)ExtractorGraph_iter, /* tp_iter */
    0,                                /* tp_iternext */
    ExtractorGraph_methods,           /* tp_methods */
    0,                                /* tp_members */
    ExtractorGraph_getseters,         /* tp_getset */
    0,                                /* tp_base */
    0,                                /* tp_dict */
    0,                                /* tp_descr_get */
    0,                                /* tp_descr_set */
    0,                                /* tp_dictoffset */
    0,                                /* tp_init */
    0,                                /* tp_alloc */
    0,                                /* tp_new */
};

PyObject *ExtractorGraph_new(fm_comp_sys_t *sys, fm_comp_graph_t *graph,
                             bool to_delete) {
  PyTypeObject *type = &ExtractorGraphType;
  ExtractorGraph *self;

  self = (ExtractorGraph *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->features =
      (ExtractorFeatures *)ExtractorFeatures_new(NULL, sys, graph, to_delete);

  return (PyObject *)self;
}

PyObject *ExtractorGraph_py_new(PyObject *py_sys, fm_comp_sys_t *sys,
                                fm_comp_graph_t *graph, bool to_delete) {
  PyTypeObject *type = &ExtractorGraphType;
  ExtractorGraph *self;

  self = (ExtractorGraph *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->features =
      (ExtractorFeatures *)ExtractorFeatures_new(py_sys, sys, graph, to_delete);

  return (PyObject *)self;
}

struct ExtractorStreamContext {
  PyObject_HEAD fm_stream_ctx_t *ctx;
  ExtractorGraph *graph;
};

static void ExtractorStreamContext_dealloc(ExtractorStreamContext *self) {
  Py_XDECREF(self->graph);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

fm_stream_ctx_t *get_stream_ctx(ExtractorStreamContext *self) {
  return self->ctx;
}

static PyObject *ExtractorStreamContext_proc_one(ExtractorStreamContext *self,
                                                 PyObject *args) {
  ExtractorBaseTypeTime64 *time;
  if (!PyArg_ParseTuple(args, "O!", &ExtractorBaseTypeTime64Type, &time)) {
    PyErr_SetString(PyExc_RuntimeError, "expecting a now of type Time64");
    return nullptr;
  }
  if (!fm_stream_ctx_proc_one(self->ctx, time->val) &&
      fm_exec_ctx_is_error((fm_exec_ctx_t *)self->ctx)) {
    PyErr_SetString(PyExc_RuntimeError,
                    fm_exec_ctx_error_msg((fm_exec_ctx_t *)self->ctx));
    return nullptr;
  }

  Py_RETURN_NONE;
}

static PyObject *ExtractorStreamContext_run_to(ExtractorStreamContext *ctx_obj,
                                               PyObject *args) {
  PyObject *obj;
  if (!PyArg_ParseTuple(args, "O", &obj) || !PyDelta_Check(obj)) {
    PyErr_SetString(PyExc_RuntimeError, "expecting a timedelta object");
    return nullptr;
  }
  auto h = 24 * PyLong_AsLong(PyObject_GetAttrString(obj, "days"));
  auto sec = PyLong_AsLong(PyObject_GetAttrString(obj, "seconds"));
  auto us = PyLong_AsLong(PyObject_GetAttrString(obj, "microseconds"));
  auto tm =
      fm_time64_from_nanos(us * 1000) + fm_time64_from_seconds(h * 3600 + sec);

  if (!fm_stream_ctx_run_to(ctx_obj->ctx, tm)) {
    PyErr_SetString(PyExc_RuntimeError,
                    fm_exec_ctx_error_msg((fm_exec_ctx_t *)ctx_obj->ctx));
    return nullptr;
  }
  Py_RETURN_NONE;
}

static PyObject *ExtractorStreamContext_now(ExtractorStreamContext *self) {
  auto tm = fm_stream_ctx_now(self->ctx);
  return ExtractorBaseTypeTime64::py_new(tm);
}

static PyObject *
ExtractorStreamContext_next_time(ExtractorStreamContext *self) {
  auto tm = fm_stream_ctx_next_time(self->ctx);
  return ExtractorBaseTypeTime64::py_new(tm);
}

static PyObject *ExtractorStreamContext_run(ExtractorStreamContext *self) {
  if (!fm_stream_ctx_run(self->ctx)) {
    PyErr_SetString(PyExc_RuntimeError,
                    fm_exec_ctx_error_msg((fm_exec_ctx_t *)self->ctx));
    return nullptr;
  }
  Py_RETURN_NONE;
}

static PyObject *ExtractorStreamContext_run_live(ExtractorStreamContext *self) {
  if (!fm_stream_ctx_run_live(self->ctx)) {
    if (PyErr_Occurred()) {
      if (PyErr_ExceptionMatches(PyExc_SystemExit)) {
        PyErr_Restore(NULL, NULL, NULL);
        Py_RETURN_NONE;
      }
      return nullptr;
    }
    PyErr_Restore(NULL, NULL, NULL);
    PyErr_SetString(PyExc_RuntimeError,
                    fm_exec_ctx_error_msg((fm_exec_ctx_t *)self->ctx));
    return nullptr;
  }
  Py_RETURN_NONE;
}

static PyObject *ExtractorStreamContext_queued(ExtractorStreamContext *self) {
  if (fm_stream_ctx_queued(self->ctx)) {
    Py_RETURN_TRUE;
  }
  Py_RETURN_FALSE;
}

static PyMethodDef ExtractorStreamContext_methods[] = {
    {"proc_one", (PyCFunction)ExtractorStreamContext_proc_one, METH_VARARGS,
     "Runs a single cycle of the event loop.\n"
     "Expects as a single argument, the desired time used to run the event "
     "loop."},
    {"now", (PyCFunction)ExtractorStreamContext_now, METH_NOARGS,
     "Returns the context's current time.\n"
     "No arguments are required."},
    {"next_time", (PyCFunction)ExtractorStreamContext_next_time, METH_NOARGS,
     "Returns the next scheduled time for the context.\n"
     "No arguments are required."},
    {"run", (PyCFunction)ExtractorStreamContext_run, METH_NOARGS,
     "Runs the event loop until completion.\n"
     "No arguments are required."},
    {"run_to", (PyCFunction)ExtractorStreamContext_run_to, METH_VARARGS,
     "Runs the event loop until the given time.\n"
     "Expects as a single argument, the desired time used to run the event "
     "loop."},
    {"run_live", (PyCFunction)ExtractorStreamContext_run_live, METH_NOARGS,
     "Runs the event loop using the live system time.\n"
     "No arguments are required."},
    {"queued", (PyCFunction)ExtractorStreamContext_queued, METH_NOARGS,
     "Returns true if there are queued functions.\n"
     "No arguments are required."},
    {NULL} /* Sentinel */
};

static PyTypeObject ExtractorStreamContextType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.StreamContext", /* tp_name */
    sizeof(ExtractorStreamContext),                           /* tp_basicsize */
    0,                                                        /* tp_itemsize */
    (destructor)ExtractorStreamContext_dealloc,               /* tp_dealloc */
    0,                                                        /* tp_print */
    0,                                                        /* tp_getattr */
    0,                                                        /* tp_setattr */
    0,                                                        /* tp_reserved */
    0,                                                        /* tp_repr */
    0,                                                        /* tp_as_number */
    0,                                        /* tp_as_sequence */
    0,                                        /* tp_as_mapping */
    0,                                        /* tp_hash  */
    0,                                        /* tp_call */
    0,                                        /* tp_str */
    0,                                        /* tp_getattro */
    0,                                        /* tp_setattro */
    0,                                        /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "System objects",                         /* tp_doc */
    0,                                        /* tp_traverse */
    0,                                        /* tp_clear */
    0,                                        /* tp_richcompare */
    0,                                        /* tp_weaklistoffset */
    0,                                        /* tp_iter */
    0,                                        /* tp_iternext */
    ExtractorStreamContext_methods,           /* tp_methods */
    0,                                        /* tp_members */
    0,                                        /* tp_getset */
    0,                                        /* tp_base */
    0,                                        /* tp_dict */
    0,                                        /* tp_descr_get */
    0,                                        /* tp_descr_set */
    0,                                        /* tp_dictoffset */
    0,                                        /* tp_init */
    0,                                        /* tp_alloc */
    0,                                        /* tp_new */
};

PyObject *ExtractorStreamContext_new(fm_stream_ctx_t *ctx,
                                     ExtractorGraph *graph) {
  PyTypeObject *type = &ExtractorStreamContextType;
  ExtractorStreamContext *self;

  self = (ExtractorStreamContext *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->ctx = ctx;
  self->graph = graph;

  Py_INCREF(graph);

  return (PyObject *)self;
}
