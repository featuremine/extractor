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
 * @file frame.hpp
 * @author Andres Rangel
 * @date 3 May 2019
 * @brief File contains C++ definition of python frame
 *
 * This file describes python frame
 */

#pragma once

#include "comp.h"
#include "comp_graph.h"
#include "extractor/comp_sys.h"

#include "utils.hpp"
#include <fmc++/python/wrapper.hpp>
#include <Python.h>

using namespace fm;
using namespace fmc::python;

typedef struct {
  PyObject_HEAD fm_comp_sys_t *sys_;
  fm_comp_graph_t *graph_;
  bool to_delete;
  PyObject *py_sys_;
} ExtractorFeatures;

typedef struct {
  PyObject_HEAD ExtractorFeatures *features;
  std::vector<PyObject *> clbcks;
} ExtractorGraph;

typedef struct {
  PyObject_HEAD ExtractorGraph *graph_;
  std::vector<fm_comp_node_t *> nodes_;
  unsigned iter_;
} ExtractorFeatureIter;

typedef struct {
  PyObject_HEAD fm_result_ref_t *ref_;
} ExtractorResultRef;

typedef struct {
  PyObject_HEAD fm_frame_t *frame_;
  bool const_;
} ExtractorFrame;

typedef struct {
  PyObject_HEAD;
  object frame_;
  unsigned iter_;
} ExtractorFrameIter;

typedef struct {
  PyObject_HEAD PyObject *parent_;
  Py_ssize_t dim_idx_;
} ExtractorSubFrame;

typedef struct {
  PyObject_HEAD;
  object subframe_;
  unsigned iter_;
} ExtractorSubFrameIter;

template <typename T>
inline void print_str(T t, const int &width, ostringstream &s) {
  s << left << setw(width) << setfill(' ') << t;
}

inline void get_frame_column_widths(vector<short> &lengths,
                                    fm_type_decl_cp &frame_type_decl,
                                    unsigned int &fields_count) {
  lengths.resize(fields_count);
  for (unsigned i = 0; i < fields_count; ++i) {
    auto type_sz = type_size(fm_type_frame_field_type(frame_type_decl, i));

    auto field_name = fm_type_frame_field_name(frame_type_decl, i);
    auto current_size = max(strlen(field_name) + 1, (size_t)type_sz);
    lengths[i] = current_size;
  }
}

static PyObject *py_frame_str(fm_frame_t *frame) {

  if (!frame) {
    return PyUnicode_FromString("");
  }

  auto frame_type_decl = fm_frame_type(frame);
  auto fields_count = fm_type_frame_nfields(frame_type_decl);
  std::ostringstream o;

  // calculate lengths of columns:
  vector<short> lengths;
  get_frame_column_widths(lengths, frame_type_decl, fields_count);

  print_str("idx", 4, o);

  for (unsigned i = 0; i < fields_count; ++i) {
    auto field_name = fm_type_frame_field_name(frame_type_decl, i);
    if (!field_name) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to find name of field in frame.");
      return nullptr;
    }
    print_str(field_name, lengths[i], o);
  }

  auto rows_count = fm_frame_dim(frame, 0);

  for (int i = 0; i < rows_count; ++i) {
    o << endl;
    print_str(i, 4, o);
    for (unsigned j = 0; j < fields_count; ++j) {
      auto value = fm_frame_get_cptr1(frame, j, i);
      print_str(ptr_to_str(fm_type_frame_field_type(frame_type_decl, j), value),
                lengths[j], o);
    }
  }

  return PyUnicode_FromString(o.str().c_str());
}

PyObject *ExtractorSubFrame_new(PyObject *parent, Py_ssize_t idx);

static void ExtractorResultRef_dealloc(ExtractorResultRef *self) {
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static void ExtractorFrameIter_dealloc(ExtractorFrameIter *self) {
  self->~ExtractorFrameIter();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorFrameIter_iter(ExtractorFrameIter *self) {
  Py_XINCREF(self);
  return (PyObject *)self;
}

static PyObject *ExtractorFrameIter_iternext(ExtractorFrameIter *self);

static PyTypeObject ExtractorFrameIterType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.FrameIter", /* tp_name */
    sizeof(ExtractorFrameIter),                           /* tp_basicsize */
    0,                                                    /* tp_itemsize */
    (destructor)ExtractorFrameIter_dealloc,               /* tp_dealloc */
    0,                                                    /* tp_print */
    0,                                                    /* tp_getattr */
    0,                                                    /* tp_setattr */
    0,                                                    /* tp_reserved */
    0,                                                    /* tp_repr */
    0,                                                    /* tp_as_number */
    0,                                                    /* tp_as_sequence */
    0,                                                    /* tp_as_mapping */
    0,                                                    /* tp_hash  */
    0,                                                    /* tp_call */
    0,                                                    /* tp_str */
    0,                                                    /* tp_getattro */
    0,                                                    /* tp_setattro */
    0,                                                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,             /* tp_flags */
    "Extractor frame iterator",                           /* tp_doc */
    0,                                                    /* tp_traverse */
    0,                                                    /* tp_clear */
    0,                                                    /* tp_richcompare */
    0,                                         /* tp_weaklistoffset */
    (getiterfunc)ExtractorFrameIter_iter,      /* tp_iter */
    (iternextfunc)ExtractorFrameIter_iternext, /* tp_iternext */
    0,                                         /* tp_methods */
    0,                                         /* tp_members */
    0,                                         /* tp_getset */
    0,                                         /* tp_base */
    0,                                         /* tp_dict */
    0,                                         /* tp_descr_get */
    0,                                         /* tp_descr_set */
    0,                                         /* tp_dictoffset */
    0,                                         /* tp_init */
    0,                                         /* tp_alloc */
    0,                                         /* tp_new */
};

static void ExtractorSubFrameIter_dealloc(ExtractorSubFrameIter *self) {
  self->~ExtractorSubFrameIter();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorSubFrameIter_iter(ExtractorSubFrameIter *self) {
  Py_XINCREF(self);
  return (PyObject *)self;
}

static PyObject *ExtractorSubFrameIter_iternext(ExtractorSubFrameIter *self);

static PyTypeObject ExtractorSubFrameIterType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.SubFrameIter", /* tp_name */
    sizeof(ExtractorSubFrameIter),                           /* tp_basicsize */
    0,                                                       /* tp_itemsize */
    (destructor)ExtractorSubFrameIter_dealloc,               /* tp_dealloc */
    0,                                                       /* tp_print */
    0,                                                       /* tp_getattr */
    0,                                                       /* tp_setattr */
    0,                                                       /* tp_reserved */
    0,                                                       /* tp_repr */
    0,                                                       /* tp_as_number */
    0,                                            /* tp_as_sequence */
    0,                                            /* tp_as_mapping */
    0,                                            /* tp_hash  */
    0,                                            /* tp_call */
    0,                                            /* tp_str */
    0,                                            /* tp_getattro */
    0,                                            /* tp_setattro */
    0,                                            /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,     /* tp_flags */
    "Extractor frame iterator",                   /* tp_doc */
    0,                                            /* tp_traverse */
    0,                                            /* tp_clear */
    0,                                            /* tp_richcompare */
    0,                                            /* tp_weaklistoffset */
    (getiterfunc)ExtractorSubFrameIter_iter,      /* tp_iter */
    (iternextfunc)ExtractorSubFrameIter_iternext, /* tp_iternext */
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

static PyObject *ExtractorResultRef_as_pandas(ExtractorResultRef *ref,
                                              PyObject *args) {
  char *index = nullptr;

  if (!PyArg_ParseTuple(args, "|s", &index)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse args");
    return nullptr;
  }

  auto frame = fm_data_get(ref->ref_);
  if (!frame) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to find data in reference");
    return nullptr;
  }

  return result_as_pandas(frame, index);
}

static PyObject *ExtractorResultRef_fields(ExtractorResultRef *ref) {

  auto frame = fm_data_get(ref->ref_);
  if (!frame) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to find data in reference");
    return nullptr;
  }

  auto type = fm_frame_type(frame);
  auto nf = fm_type_frame_nfields(type);
  auto ret = PyList_New(nf);
  if (!ret) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to create list of fields");
    return nullptr;
  }
  for (unsigned i = 0; i < nf; ++i) {
    auto fieldname = fm_type_frame_field_name(type, i);
    if (!fieldname) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to find name of field in frame.");
      return nullptr;
    }
    auto str = PyUnicode_FromString(fieldname);
    if (PyList_SetItem(ret, i, str) == -1) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to insert field name to list.");
      return nullptr;
    }
  }
  return ret;
}

static PyMethodDef ExtractorResultRef_methods[] = {
    {"as_pandas", (PyCFunction)ExtractorResultRef_as_pandas, METH_VARARGS,
     "Returns the frame content as a pandas dataframe.\n"
     "As an optional parameter, the column name of the desired index can be "
     "specified."},
    {"fields", (PyCFunction)ExtractorResultRef_fields, METH_NOARGS,
     "Returns the frame field names in a list.\n"
     "No arguments are required."},
    {NULL} /* Sentinel */
};

static int ExtractorResultRef_mp_length(ExtractorResultRef *ref) {
  auto frame = fm_data_get(ref->ref_);
  if (!frame) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to find data in reference");
    return -1;
  }
  // Change for ndims when multiple dimensions are supported
  return fm_frame_dim(frame, 0);
}

static PyObject *ExtractorResultRef_mp_subscript(ExtractorResultRef *ref,
                                                 PyObject *key) {
  auto frame = fm_data_get(ref->ref_);
  if (!frame) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to find data in reference");
    return nullptr;
  }
  auto subframe_dim = dim_from_key(frame, key);
  if (subframe_dim < 0)
    return nullptr;
  return ExtractorSubFrame_new((PyObject *)ref, subframe_dim);
}

static int ExtractorResultRef_mp_ass_subscript(ExtractorResultRef *ref,
                                               PyObject *key, PyObject *other) {
  PyErr_SetString(PyExc_RuntimeError, "Row assignment is not supported");
  return -1;
}

static PyMappingMethods ExtractorResultRef_as_mapping = {
    (lenfunc)ExtractorResultRef_mp_length,              /* mp_length */
    (binaryfunc)ExtractorResultRef_mp_subscript,        /* mp_subscript */
    (objobjargproc)ExtractorResultRef_mp_ass_subscript, /* mp_ass_subscript */
};

static PyObject *ExtractorResultRef_str(ExtractorResultRef *self) {
  auto frame = fm_data_get(self->ref_);
  if (!frame) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to find data in reference");
    return nullptr;
  }

  return py_frame_str(frame);
}

PyObject *ExtractorResultRef_iter(ExtractorResultRef *ref) {
  auto frame = fm_data_get(ref->ref_);
  if (!frame) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to find data in reference");
    return nullptr;
  }

  PyTypeObject *subtype = &ExtractorFrameIterType;
  auto *ret = (ExtractorFrameIter *)subtype->tp_alloc(subtype, 0);
  ret->frame_ = object::from_borrowed((PyObject *)frame);
  ret->iter_ = 0;
  return (PyObject *)ret;
}

static PyTypeObject ExtractorResultRefType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.ResultRef", /* tp_name */
    sizeof(ExtractorResultRef),                           /* tp_basicsize */
    0,                                                    /* tp_itemsize */
    (destructor)ExtractorResultRef_dealloc,               /* tp_dealloc */
    0,                                                    /* tp_print */
    0,                                                    /* tp_getattr */
    0,                                                    /* tp_setattr */
    0,                                                    /* tp_reserved */
    (reprfunc)ExtractorResultRef_str,                     /* tp_repr */
    0,                                                    /* tp_as_number */
    0,                                                    /* tp_as_sequence */
    &ExtractorResultRef_as_mapping,                       /* tp_as_mapping */
    0,                                                    /* tp_hash  */
    0,                                                    /* tp_call */
    (reprfunc)ExtractorResultRef_str,                     /* tp_str */
    0,                                                    /* tp_getattro */
    0,                                                    /* tp_setattro */
    0,                                                    /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,             /* tp_flags */
    "Result reference objects",                           /* tp_doc */
    0,                                                    /* tp_traverse */
    0,                                                    /* tp_clear */
    0,                                                    /* tp_richcompare */
    0,                                    /* tp_weaklistoffset */
    (getiterfunc)ExtractorResultRef_iter, /* tp_iter */
    0,                                    /* tp_iternext */
    ExtractorResultRef_methods,           /* tp_methods */
    0,                                    /* tp_members */
    0,                                    /* tp_getset */
    0,                                    /* tp_base */
    0,                                    /* tp_dict */
    0,                                    /* tp_descr_get */
    0,                                    /* tp_descr_set */
    0,                                    /* tp_dictoffset */
    0,                                    /* tp_init */
    0,                                    /* tp_alloc */
    0,                                    /* tp_new */
};

PyObject *ExtractorResultRef_new(fm_result_ref_t *ref) {
  PyTypeObject *type = &ExtractorResultRefType;
  ExtractorResultRef *self;

  self = (ExtractorResultRef *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->ref_ = ref;

  return (PyObject *)self;
}

static void ExtractorFrame_dealloc(ExtractorFrame *self) {
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorFrame_as_pandas(ExtractorFrame *s, PyObject *args) {
  char *index = nullptr;

  if (!PyArg_ParseTuple(args, "|s", &index)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse args");
    return nullptr;
  }

  return result_as_pandas(s->frame_, index);
}

static PyObject *ExtractorFrame_fields(ExtractorFrame *s) {

  auto type = fm_frame_type(s->frame_);
  auto nf = fm_type_frame_nfields(type);
  auto ret = PyList_New(nf);
  if (!ret) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to create list of fields");
    return nullptr;
  }
  for (unsigned i = 0; i < nf; ++i) {
    auto fieldname = fm_type_frame_field_name(type, i);
    if (!fieldname) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to find name of field in frame.");
      return nullptr;
    }
    auto str = PyUnicode_FromString(fieldname);
    if (PyList_SetItem(ret, i, str) == -1) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to insert field name to list.");
      return nullptr;
    }
  }
  return ret;
}

static PyMethodDef ExtractorFrame_methods[] = {
    {"as_pandas", (PyCFunction)ExtractorFrame_as_pandas, METH_VARARGS,
     "Returns the frame content as a pandas dataframe.\n"
     "As an optional parameter, the column name of the desired index can be "
     "specified."},
    {"fields", (PyCFunction)ExtractorFrame_fields, METH_NOARGS,
     "Returns the frame field names in a list.\n"
     "No arguments are required."},
    {NULL} /* Sentinel */
};

static int ExtractorFrame_mp_length(ExtractorFrame *frame) {
  // Change for ndims when multiple dimensions are supported
  return fm_frame_dim(frame->frame_, 0);
}

static PyObject *ExtractorFrame_mp_subscript(ExtractorFrame *frame,
                                             PyObject *key) {
  auto subframe_dim = dim_from_key(frame->frame_, key);
  if (subframe_dim < 0)
    return nullptr;
  return ExtractorSubFrame_new((PyObject *)frame, subframe_dim);
}

static int ExtractorFrame_mp_ass_subscript(ExtractorFrame *frame, PyObject *key,
                                           PyObject *other) {
  PyErr_SetString(PyExc_RuntimeError, "Row assignment is not supported");
  return -1;
}

static PyMappingMethods ExtractorFrame_as_mapping = {
    (lenfunc)ExtractorFrame_mp_length,              /* mp_length */
    (binaryfunc)ExtractorFrame_mp_subscript,        /* mp_subscript */
    (objobjargproc)ExtractorFrame_mp_ass_subscript, /* mp_ass_subscript */
};

static PyObject *ExtractorFrame_str(ExtractorFrame *self) {
  return py_frame_str(self->frame_);
}

PyObject *ExtractorFrame_iter(ExtractorFrame *frame) {
  PyTypeObject *subtype = &ExtractorFrameIterType;
  auto *ret = (ExtractorFrameIter *)subtype->tp_alloc(subtype, 0);
  ret->frame_ = object::from_borrowed((PyObject *)frame);
  ret->iter_ = 0;
  return (PyObject *)ret;
}

static PyTypeObject ExtractorFrameType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Frame", /* tp_name */
    sizeof(ExtractorFrame),                           /* tp_basicsize */
    0,                                                /* tp_itemsize */
    (destructor)ExtractorFrame_dealloc,               /* tp_dealloc */
    0,                                                /* tp_print */
    0,                                                /* tp_getattr */
    0,                                                /* tp_setattr */
    0,                                                /* tp_reserved */
    (reprfunc)ExtractorFrame_str,                     /* tp_repr */
    0,                                                /* tp_as_number */
    0,                                                /* tp_as_sequence */
    &ExtractorFrame_as_mapping,                       /* tp_as_mapping */
    0,                                                /* tp_hash  */
    0,                                                /* tp_call */
    (reprfunc)ExtractorFrame_str,                     /* tp_str */
    0,                                                /* tp_getattro */
    0,                                                /* tp_setattro */
    0,                                                /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,         /* tp_flags */
    "Frame objects",                                  /* tp_doc */
    0,                                                /* tp_traverse */
    0,                                                /* tp_clear */
    0,                                                /* tp_richcompare */
    0,                                                /* tp_weaklistoffset */
    (getiterfunc)ExtractorFrame_iter,                 /* tp_iter */
    0,                                                /* tp_iternext */
    ExtractorFrame_methods,                           /* tp_methods */
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

PyObject *ExtractorFrame_new(const fm_frame_t *frame, bool constval) {
  PyTypeObject *type = &ExtractorFrameType;
  ExtractorFrame *self;

  self = (ExtractorFrame *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->frame_ = (fm_frame_t *)frame;
  self->const_ = constval;

  return (PyObject *)self;
}

static int ExtractorSubFrame_setattr(PyObject *obj, PyObject *attr_name,
                                     PyObject *other) {
  auto subframe = (ExtractorSubFrame *)obj;

  fm_frame_t *frame = nullptr;

  if (PyObject_TypeCheck(subframe->parent_, &ExtractorFrameType)) {
    auto typedparent = (ExtractorFrame *)subframe->parent_;
    if (typedparent->const_) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Field is constant, cannot be modified.");
      return -1;
    }
    frame = typedparent->frame_;
  } else {
    auto typedparent = (ExtractorResultRef *)subframe->parent_;
    frame = fm_data_get(typedparent->ref_);
    if (!frame) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to find data in reference");
      return -1;
    }
  }

  auto name = PyUnicode_AsUTF8(attr_name);

  if (PyErr_Occurred()) {
    return -1;
  }

  auto field = fm_frame_field(frame, name);
  if (field < 0) {
    PyErr_SetString(PyExc_RuntimeError,
                    "Provided key is not the name of a field in frame");
    return -1;
  }

  auto *target = fm_frame_get_ptr1(frame, field, subframe->dim_idx_);

  auto type = fm_type_frame_field_type(fm_frame_type(frame), field);
  if (get_py_field_converter(type)(target, other) && !PyErr_Occurred())
    return 0;

  if (!PyErr_Occurred())
    PyErr_SetString(PyExc_RuntimeError, "Unable to set value in field.");
  return -1;
}

static void ExtractorSubFrame_dealloc(ExtractorSubFrame *self) {
  Py_XDECREF(self->parent_);
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *ExtractorSubFrame_dir(ExtractorSubFrame *subframe) {
  fm_frame_t *frame = nullptr;

  if (PyObject_TypeCheck(subframe->parent_, &ExtractorFrameType)) {
    auto typedparent = (ExtractorFrame *)subframe->parent_;
    frame = typedparent->frame_;
  } else {
    auto typedparent = (ExtractorResultRef *)subframe->parent_;
    frame = fm_data_get(typedparent->ref_);
    if (!frame) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to find data in reference");
      return nullptr;
    }
  }

  auto type = fm_frame_type(frame);
  auto nf = fm_type_frame_nfields(type);
  auto ret = PyList_New(nf);
  if (!ret) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to create list of fields");
    return nullptr;
  }
  for (unsigned i = 0; i < nf; ++i) {
    auto fieldname = fm_type_frame_field_name(type, i);
    if (!fieldname) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to find name of field in frame.");
      return nullptr;
    }
    auto str = PyUnicode_FromString(fieldname);
    if (PyList_SetItem(ret, i, str) == -1) {
      PyErr_SetString(PyExc_RuntimeError,
                      "Unable to insert field name to list.");
      return nullptr;
    }
  }
  return ret;
}

static PyMethodDef ExtractorSubFrame_methods[] = {
    {"__dir__", (PyCFunction)ExtractorSubFrame_dir, METH_NOARGS,
     "Returns the frame field names in a list.\n"
     "No arguments are required."},
    {NULL} /* Sentinel */
};

static PyObject *ExtractorSubFrame_getattr(PyObject *obj, PyObject *attr_name);

static PyObject *ExtractorSubFrame_str(ExtractorSubFrame *self) {

  fm_frame_t *frame = nullptr;

  if (PyObject_TypeCheck(self->parent_, &ExtractorFrameType)) {
    auto typedparent = (ExtractorFrame *)self->parent_;
    frame = typedparent->frame_;
  } else {
    auto typedparent = (ExtractorResultRef *)self->parent_;
    frame = fm_data_get(typedparent->ref_);
    if (!frame) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to find data in reference");
      return nullptr;
    }
  }
  auto idx = self->dim_idx_;
  auto frame_type_decl = fm_frame_type(frame);
  auto fields_count = fm_type_frame_nfields(frame_type_decl);
  std::ostringstream o;
  vector<short> lengths;
  get_frame_column_widths(lengths, frame_type_decl, fields_count);
  print_str(idx, 4, o);
  for (unsigned j = 0; j < fields_count; ++j) {
    auto value = fm_frame_get_cptr1(frame, j, idx);
    print_str(ptr_to_str(fm_type_frame_field_type(frame_type_decl, j), value),
              lengths[j], o);
  }

  return PyUnicode_FromString(o.str().c_str());
}

PyObject *ExtractorSubFrame_iter(ExtractorSubFrame *subframe) {
  PyTypeObject *subtype = &ExtractorSubFrameIterType;
  auto *ret = (ExtractorSubFrameIter *)subtype->tp_alloc(subtype, 0);
  ret->subframe_ = object::from_borrowed((PyObject *)subframe);
  ret->iter_ = 0;
  return (PyObject *)ret;
}

static PyTypeObject ExtractorSubFrameType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.SubFrame", /* tp_name */
    sizeof(ExtractorSubFrame),                           /* tp_basicsize */
    0,                                                   /* tp_itemsize */
    (destructor)ExtractorSubFrame_dealloc,               /* tp_dealloc */
    0,                                                   /* tp_print */
    0,                                                   /* tp_getattr */
    0,                                                   /* tp_setattr */
    0,                                                   /* tp_reserved */
    (reprfunc)ExtractorSubFrame_str,                     /* tp_repr */
    0,                                                   /* tp_as_number */
    0,                                                   /* tp_as_sequence */
    0,                                                   /* tp_as_mapping */
    0,                                                   /* tp_hash  */
    0,                                                   /* tp_call */
    (reprfunc)ExtractorSubFrame_str,                     /* tp_str */
    ExtractorSubFrame_getattr,                           /* tp_getattro */
    ExtractorSubFrame_setattr,                           /* tp_setattro */
    0,                                                   /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,            /* tp_flags */
    "SubFrame objects",                                  /* tp_doc */
    0,                                                   /* tp_traverse */
    0,                                                   /* tp_clear */
    0,                                                   /* tp_richcompare */
    0,                                                   /* tp_weaklistoffset */
    (getiterfunc)ExtractorSubFrame_iter,                 /* tp_iter */
    0,                                                   /* tp_iternext */
    ExtractorSubFrame_methods,                           /* tp_methods */
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

static PyObject *ExtractorSubFrame_getattr(PyObject *obj, PyObject *attr_name) {
  auto subframe = (ExtractorSubFrame *)obj;

  fm_frame_t *frame = nullptr;

  auto name = PyUnicode_AsUTF8(attr_name);

  auto validate_non_fields = [&name]() -> PyObject * {
    if (strcmp(name, "__class__") == 0) {
      Py_INCREF(&ExtractorSubFrameType);
      return (PyObject *)&ExtractorSubFrameType;
    }
    return nullptr;
  };

  if (PyObject_TypeCheck(subframe->parent_, &ExtractorFrameType)) {
    auto typedparent = (ExtractorFrame *)subframe->parent_;
    frame = typedparent->frame_;
  } else {
    auto typedparent = (ExtractorResultRef *)subframe->parent_;
    frame = fm_data_get(typedparent->ref_);
    if (!frame) {
      auto ret = validate_non_fields();
      if (!ret) {
        PyErr_SetString(PyExc_AttributeError,
                        "No data in frame reference, need to run context");
      }
      return ret;
    }
  }

  if (PyErr_Occurred()) {
    return nullptr;
  }

  auto field = fm_frame_field(frame, name);
  if (field < 0) {
    auto ret = validate_non_fields();
    if (!ret) {
      auto err = std::string("no attribute <") + name + ">";
      PyErr_SetString(PyExc_AttributeError, err.c_str());
    }
    return ret;
  }

  auto type = fm_type_frame_field_type(fm_frame_type(frame), field);
  auto *target = fm_frame_get_ptr1(frame, field, subframe->dim_idx_);
  return get_py_obj_from_ptr(type, target);
}

PyObject *ExtractorSubFrame_new(PyObject *parent, Py_ssize_t idx) {
  PyTypeObject *type = &ExtractorSubFrameType;
  ExtractorSubFrame *self;

  self = (ExtractorSubFrame *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  Py_INCREF(parent);
  self->parent_ = parent;
  self->dim_idx_ = idx;

  return (PyObject *)self;
}

static fm_frame_t *get_fm_frame(PyObject *frame) {
  fm_frame_t *ret = nullptr;
  if (PyObject_TypeCheck(frame, &ExtractorFrameType)) {
    ret = ((ExtractorFrame *)frame)->frame_;
  } else {
    ret = fm_data_get(((ExtractorResultRef *)frame)->ref_);
    if (!frame) {
      PyErr_SetString(PyExc_RuntimeError, "Unable to find data in reference");
      return nullptr;
    }
  }
  return ret;
}

static PyObject *ExtractorFrameIter_iternext(ExtractorFrameIter *self) {
  fm_frame_t *frame = get_fm_frame(self->frame_.get_ref());
  if (self->iter_ >= (unsigned)fm_frame_dim(frame, 0)) {
    PyErr_SetNone(PyExc_StopIteration);
    return nullptr;
  }
  return ExtractorSubFrame_new((PyObject *)self->frame_.get_ref(),
                               self->iter_++);
}

static PyObject *ExtractorSubFrameIter_iternext(ExtractorSubFrameIter *self) {
  auto *subframe = (ExtractorSubFrame *)self->subframe_.get_ref();
  fm_frame_t *frame = get_fm_frame(subframe->parent_);
  auto *frame_type = fm_frame_type(frame);
  auto size = fm_type_frame_nfields(frame_type);
  if (self->iter_ >= size) {
    PyErr_SetNone(PyExc_StopIteration);
    return nullptr;
  }
  auto error = [](const char *err) {
    PyErr_SetString(PyExc_RuntimeError, err);
    return nullptr;
  };
  auto *fname = fm_type_frame_field_name(frame_type, self->iter_);
  auto *field = fm_frame_get_ptr1(frame, self->iter_, (subframe)->dim_idx_);
  auto type = fm_type_frame_field_type(frame_type, self->iter_);
  auto *py_field = get_py_obj_from_ptr(type, field);
  if (py_field == nullptr)
    return error("Unable to get py object for field");
  auto *name = PyUnicode_FromString(fname);
  if (name == nullptr)
    return error("Unable to create field name");
  auto *ret = PyTuple_Pack(2, name, py_field);
  if (ret == nullptr)
    return error("Unable to pack tuple");
  ++(self->iter_);
  return ret;
}
