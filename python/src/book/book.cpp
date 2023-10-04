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
 * @file py_book.cpp
 * @author Andres Rangel
 * @date 2 Mar 2020
 * @brief Python extension for python book object
 */

#include <Python.h>

#include "extractor/book/book.h"
#include "extractor/python/book.h"
#include "extractor/python/py_api.h"

#include "book/level.hpp"
#include "fmc++/side.hpp"

struct BookStruct {
  PyObject_HEAD fm_book_shared_t *book_;
  ~BookStruct() { fm_book_shared_dec(book_); }
};

typedef struct LevelsIter {
  PyObject_HEAD;
  unsigned done_;
  fmc::python::object book_;
  Book *book() { return (Book *)book_.get_ref(); }
} LevelsIter;

// Levels iterator

PyObject *LevelsIter_iter(PyObject *self) {
  LevelsIter *p = (LevelsIter *)self;
  p->done_ = 0;
  return self;
}

PyObject *LevelsIter_iternext(PyObject *self) {
  LevelsIter *p = (LevelsIter *)self;
  if (p->done_ > 1) {
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }
  auto *ret = PyTuple_New(2);
  PyObject *side = nullptr;
  if (p->done_ == 0) {
    side = TradeSide_BID;
  } else {
    side = TradeSide_ASK;
  }
  Py_INCREF(side);
  PyTuple_SET_ITEM(ret, 0, side);
  auto *levels =
      fm_book_levels(fm_book_shared_get(p->book()->book_), p->done_ == 0);
  ++p->done_;
  PyTuple_SET_ITEM(ret, 1, Levels_new(levels, p->book()));
  return ret;
}

static void LevelsIter_dealloc(LevelsIter *self) {
  self->~LevelsIter();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyTypeObject LevelsIterType = {
    PyVarObject_HEAD_INIT(NULL, 0) "LevelsIter", /* tp_name */
    sizeof(LevelsIter),                          /* tp_basicsize */
    0,                                           /* tp_itemsize */
    (destructor)LevelsIter_dealloc,              /* tp_dealloc */
    0,                                           /* tp_print */
    0,                                           /* tp_getattr */
    0,                                           /* tp_setattr */
    0,                                           /* tp_reserved */
    0,                                           /* tp_repr */
    0,                                           /* tp_as_number */
    0,                                           /* tp_as_sequence */
    0,                                           /* tp_as_mapping */
    0,                                           /* tp_hash  */
    0,                                           /* tp_call */
    0,                                           /* tp_str */
    0,                                           /* tp_getattro */
    0,                                           /* tp_setattro */
    0,                                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,    /* tp_flags */
    "order iterator objects",                    /* tp_doc */
    0,                                           /* tp_traverse */
    0,                                           /* tp_clear */
    0,                                           /* tp_richcompare */
    0,                                           /* tp_weaklistoffset */
    LevelsIter_iter,                             /* tp_iter */
    LevelsIter_iternext,                         /* tp_iternext */
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

PyObject *LevelsIter_new(Book *book) {
  PyTypeObject *type = &LevelsIterType;
  LevelsIter *self = nullptr;
  self = (LevelsIter *)type->tp_alloc(type, 0);
  if (!self)
    return nullptr;
  self->done_ = 0;
  self->book_ = fmc::python::object::from_borrowed((PyObject *)book);
  return (PyObject *)self;
}

// Book

static void Book_dealloc(Book *self) {
  self->~Book();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *Book_new(PyTypeObject *subtype, PyObject *args,
                          PyObject *kwds) {
  Book *self = nullptr;

  self = (Book *)subtype->tp_alloc(subtype, 0);
  if (self == nullptr)
    return nullptr;

  self->book_ = fm_book_shared_new();

  return (PyObject *)self;
}

static int Book_mp_length(Book *ref) { return 2; }

static PyObject *Book_mp_subscript(Book *ref, PyObject *key) {
  if (!TradeSide_TypeCheck(key)) {
    PyErr_SetString(PyExc_IndexError,
                    "Unsupported key, please use an extractor side object");
    return nullptr;
  }
  auto is_bid = key == TradeSide_BID;
  auto *levels = fm_book_levels(fm_book_shared_get(ref->book_), is_bid);
  return Levels_new(levels, ref);
}

static int Book_mp_ass_subscript(Book *ref, PyObject *key, PyObject *other) {
  PyErr_SetString(PyExc_RuntimeError, "Levels assignment is not supported");
  return -1;
}

static PyMappingMethods Book_as_mapping = {
    (lenfunc)Book_mp_length,              /* mp_length */
    (binaryfunc)Book_mp_subscript,        /* mp_subscript */
    (objobjargproc)Book_mp_ass_subscript, /* mp_ass_subscript */
};

PyTypeObject BookType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Book", /* tp_name */
    sizeof(Book),                                    /* tp_basicsize */
    0,                                               /* tp_itemsize */
    (destructor)Book_dealloc,                        /* tp_dealloc */
    0,                                               /* tp_print */
    0,                                               /* tp_getattr */
    0,                                               /* tp_setattr */
    0,                                               /* tp_reserved */
    0,                                               /* tp_repr */
    0,                                               /* tp_as_number */
    0,                                               /* tp_as_sequence */
    &Book_as_mapping,                                /* tp_as_mapping */
    0,                                               /* tp_hash  */
    0,                                               /* tp_call */
    0,                                               /* tp_str */
    0,                                               /* tp_getattro */
    0,                                               /* tp_setattro */
    0,                                               /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,        /* tp_flags */
    "Book objects",                                  /* tp_doc */
    0,                                               /* tp_traverse */
    0,                                               /* tp_clear */
    0,                                               /* tp_richcompare */
    0,                                               /* tp_weaklistoffset */
    (getiterfunc)LevelsIter_new,                     /* tp_iter */
    0,                                               /* tp_iternext */
    0,                                               /* tp_methods */
    0,                                               /* tp_members */
    0,                                               /* tp_getset */
    0,                                               /* tp_base */
    0,                                               /* tp_dict */
    0,                                               /* tp_descr_get */
    0,                                               /* tp_descr_set */
    0,                                               /* tp_dictoffset */
    0,                                               /* tp_init */
    0,                                               /* tp_alloc */
    Book_new,                                        /* tp_new */
};

bool PyBook_Check(PyObject *obj) { return PyObject_TypeCheck(obj, &BookType); }

fm_book_shared_t *PyBook_SharedBook(PyObject *obj) {
  if (!PyBook_Check(obj)) {
    return nullptr;
  }
  return ((Book *)obj)->book_;
}

bool PyBook_AddTypes(PyObject *m) {
  PyDateTime_IMPORT;
  if (PyType_Ready(&BookType) < 0)
    return false;
  if (PyType_Ready(&LevelsType) < 0)
    return false;
  if (PyType_Ready(&LevelsIterType) < 0)
    return false;
  if (PyType_Ready(&LevelType) < 0)
    return false;
  if (PyType_Ready(&LevelIterType) < 0)
    return false;
  if (PyType_Ready(&OrderType) < 0)
    return false;
  if (PyType_Ready(&OrderIterType) < 0)
    return false;
  if (m) {
    Py_INCREF(&BookType);
    if (PyModule_AddObject(m, "Book", (PyObject *)&BookType) < 0) {
      Py_DECREF(&BookType);
      return false;
    }
    Py_INCREF(&LevelsType);
    if (PyModule_AddObject(m, "Levels", (PyObject *)&LevelsType) < 0) {
      Py_DECREF(&LevelsType);
      return false;
    }
    Py_INCREF(&LevelsIterType);
    if (PyModule_AddObject(m, "LevelsIter", (PyObject *)&LevelsIterType) < 0) {
      Py_DECREF(&LevelsIterType);
      return false;
    }
    Py_INCREF(&LevelType);
    if (PyModule_AddObject(m, "Level", (PyObject *)&LevelType) < 0) {
      Py_DECREF(&LevelType);
      return false;
    }
    Py_INCREF(&LevelIterType);
    if (PyModule_AddObject(m, "LevelIter", (PyObject *)&LevelIterType) < 0) {
      Py_DECREF(&LevelIterType);
      return false;
    }
    Py_INCREF(&OrderType);
    if (PyModule_AddObject(m, "Order", (PyObject *)&OrderType) < 0) {
      Py_DECREF(&OrderType);
      return false;
    }
    Py_INCREF(&OrderIterType);
    if (PyModule_AddObject(m, "OrderIter", (PyObject *)&OrderIterType) < 0) {
      Py_DECREF(&OrderIterType);
      return false;
    }
  }
  return true;
}
