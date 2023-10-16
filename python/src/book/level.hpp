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

#pragma once

#include <Python.h>

#include "extractor/book/book.h"
#include "extractor/python/book.h"
#include "extractor/python/decimal128.h"
#include "extractor/python/py_api.h"

#include "fmc++/decimal128.hpp"
#include "fmc++/python/wrapper.hpp"
#include "fmc++/side.hpp"
#include <fmc++/python/wrapper.hpp>

typedef struct Levels {
  PyObject_HEAD fm_levels_t *levels_;
  fmc::python::object book_;
  Book *book() { return (Book *)book_.get_ref(); }
} Levels;

typedef struct Level {
  PyObject_HEAD fm_level_t *level_;
  fmc::python::object levels_;
  Levels *levels() { return (Levels *)levels_.get_ref(); }
} Level;

typedef struct LevelIter {
  PyObject_HEAD;
  unsigned done_;
  fmc::python::object levels_;
  Levels *levels() { return (Levels *)levels_.get_ref(); }
} LevelIter;

typedef struct Order {
  PyObject_HEAD fm_order_t *order_;
  fmc::python::object level_;
  Level *level() { return (Level *)level_.get_ref(); }
} Order;

typedef struct OrderIter {
  PyObject_HEAD;
  unsigned done_;
  fmc::python::object level_;
  Level *level() { return (Level *)level_.get_ref(); }
} OrderIter;

fmc_time64_t fm_book_order_rec(fm_order_t *lvl);
fmc_time64_t fm_book_order_ven(fm_order_t *lvl);

static PyObject *Order_prio(Order *self, void *) {
  return PyLong_FromUnsignedLongLong(fm_book_order_prio(self->order_));
}

static PyObject *Order_id(Order *self, void *) {
  return PyLong_FromUnsignedLongLong(fm_book_order_id(self->order_));
}

static PyObject *Order_qty(Order *self, void *) {
  return Decimal128_new(fm_book_order_qty(self->order_));
}

static PyObject *Order_rec(Order *self, void *) {
  auto rec_time = fm_book_order_rec(self->order_);
  using namespace std::chrono;
  auto t = nanoseconds(fmc_time64_to_nanos(rec_time));
  auto us = duration_cast<microseconds>(t);
  auto sec = duration_cast<seconds>(us);
  auto tmp = duration_cast<microseconds>(sec);
  auto rem = us - tmp;
  return fmc::python::datetime::timedelta(0, sec.count(), rem.count())
      .steal_ref();
}

static PyObject *Order_ven(Order *self, void *) {
  auto ven_time = fm_book_order_ven(self->order_);
  using namespace std::chrono;
  auto t = nanoseconds(fmc_time64_to_nanos(ven_time));
  auto us = duration_cast<microseconds>(t);
  auto sec = duration_cast<seconds>(us);
  auto tmp = duration_cast<microseconds>(sec);
  auto rem = us - tmp;
  return fmc::python::datetime::timedelta(0, sec.count(), rem.count())
      .steal_ref();
}

static PyObject *Order_seq(Order *self, void *) {
  return PyLong_FromUnsignedLongLong(fm_book_order_seq(self->order_));
}

static PyGetSetDef Order_getseters[] = {
    {(char *)"priority", (getter)Order_prio, NULL, (char *)"Order priority.",
     NULL},
    {(char *)"id", (getter)Order_id, NULL, (char *)"Order id.", NULL},
    {(char *)"qty", (getter)Order_qty, NULL, (char *)"Order quantity.", NULL},
    {(char *)"received", (getter)Order_rec, NULL, (char *)"Order received.",
     NULL},
    {(char *)"ven", (getter)Order_ven, NULL,
     (char *)"Order time difference between received time and received time by "
             "broker.",
     NULL},
    {(char *)"seqnum", (getter)Order_seq, NULL,
     (char *)"Order sequence number.", NULL},
    {NULL, NULL, NULL, NULL, NULL} /* Sentinel */
};

static void Order_dealloc(Order *self) {
  self->~Order();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

PyTypeObject OrderType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Order", /* tp_name */
    sizeof(Order),                                    /* tp_basicsize */
    0,                                                /* tp_itemsize */
    (destructor)Order_dealloc,                        /* tp_dealloc */
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
    "Order objects",                                  /* tp_doc */
    0,                                                /* tp_traverse */
    0,                                                /* tp_clear */
    0,                                                /* tp_richcompare */
    0,                                                /* tp_weaklistoffset */
    0,                                                /* tp_iter */
    0,                                                /* tp_iternext */
    0,                                                /* tp_methods */
    0,                                                /* tp_members */
    Order_getseters,                                  /* tp_getset */
    0,                                                /* tp_base */
    0,                                                /* tp_dict */
    0,                                                /* tp_descr_get */
    0,                                                /* tp_descr_set */
    0,                                                /* tp_dictoffset */
    0,                                                /* tp_init */
    0,                                                /* tp_alloc */
    0,                                                /* tp_new */
};

static PyObject *Order_new(fm_order_t *order, Level *lvl) {
  PyTypeObject *type = &OrderType;
  Order *self = nullptr;
  self = (Order *)type->tp_alloc(type, 0);
  if (!self)
    return nullptr;
  self->order_ = order;
  self->level_ = fmc::python::object::from_borrowed((PyObject *)lvl);
  return (PyObject *)self;
}

// Order iterator

PyObject *OrderIter_iter(PyObject *self) {
  OrderIter *p = (OrderIter *)self;
  p->done_ = 0;
  return self;
}

PyObject *OrderIter_iternext(PyObject *self) {
  OrderIter *p = (OrderIter *)self;
  if (p->done_ >= fm_book_level_ord(p->level()->level_)) {
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }
  auto *order = fm_book_level_order(p->level()->level_, p->done_++);
  return Order_new(order, p->level());
}

static void OrderIter_dealloc(OrderIter *self) {
  self->~OrderIter();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyTypeObject OrderIterType = {
    PyVarObject_HEAD_INIT(NULL, 0) "OrderIter", /* tp_name */
    sizeof(OrderIter),                          /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)OrderIter_dealloc,              /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash  */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "order iterator objects",                   /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    OrderIter_iter,                             /* tp_iter */
    OrderIter_iternext,                         /* tp_iternext */
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

PyObject *OrderIter_new(Level *level) {
  PyTypeObject *type = &OrderIterType;
  OrderIter *self = nullptr;
  self = (OrderIter *)type->tp_alloc(type, 0);
  if (!self)
    return nullptr;
  self->done_ = 0;
  self->level_ = fmc::python::object::from_borrowed((PyObject *)level);
  return (PyObject *)self;
}

// Level

static int Level_mp_length(Level *ref) {
  return fm_book_level_ord(ref->level_);
}

static PyObject *Level_mp_subscript(Level *ref, PyObject *key) {
  if (PyLong_Check(key)) {
    auto order = PyLong_AsLong(key);
    long sz = fm_book_level_ord(ref->level_);
    if (order < 0 && order >= -sz) {
      return Order_new(fm_book_level_order(ref->level_, sz + order), ref);
    } else if (order >= 0 && order < sz) {
      return Order_new(fm_book_level_order(ref->level_, order), ref);
    }
    PyErr_SetString(PyExc_IndexError, "Provided index out of range");
    return nullptr;
  }
  PyErr_SetString(PyExc_IndexError,
                  "Unsupported key, please use an integer index");
  return nullptr;
}

static int Level_mp_ass_subscript(Level *ref, PyObject *key, PyObject *other) {
  PyErr_SetString(PyExc_RuntimeError, "Level assignment is not supported");
  return -1;
}

static PyMappingMethods Level_as_mapping = {
    (lenfunc)Level_mp_length,              /* mp_length */
    (binaryfunc)Level_mp_subscript,        /* mp_subscript */
    (objobjargproc)Level_mp_ass_subscript, /* mp_ass_subscript */
};

static PyObject *Level_px(Level *self, void *) {
  return Decimal128_new(fm_book_level_prx(self->level_));
}

static PyObject *Level_shr(Level *self, void *) {
  return Decimal128_new(fm_book_level_shr(self->level_));
}

static PyObject *Level_ord(Level *self, void *) {
  return PyLong_FromLong(fm_book_level_ord(self->level_));
}

static PyGetSetDef Level_getseters[] = {
    {(char *)"px", (getter)Level_px, NULL, (char *)"Level price.", NULL},
    {(char *)"shares", (getter)Level_shr, NULL, (char *)"Level shares count.",
     NULL},
    {(char *)"orders", (getter)Level_ord, NULL, (char *)"Level orders count.",
     NULL},
    {NULL, NULL, NULL, NULL, NULL} /* Sentinel */
};

static void Level_dealloc(Level *self) {
  self->~Level();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

PyTypeObject LevelType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Level", /* tp_name */
    sizeof(Level),                                    /* tp_basicsize */
    0,                                                /* tp_itemsize */
    (destructor)Level_dealloc,                        /* tp_dealloc */
    0,                                                /* tp_print */
    0,                                                /* tp_getattr */
    0,                                                /* tp_setattr */
    0,                                                /* tp_reserved */
    0,                                                /* tp_repr */
    0,                                                /* tp_as_number */
    0,                                                /* tp_as_sequence */
    &Level_as_mapping,                                /* tp_as_mapping */
    0,                                                /* tp_hash  */
    0,                                                /* tp_call */
    0,                                                /* tp_str */
    0,                                                /* tp_getattro */
    0,                                                /* tp_setattro */
    0,                                                /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,         /* tp_flags */
    "Level objects",                                  /* tp_doc */
    0,                                                /* tp_traverse */
    0,                                                /* tp_clear */
    0,                                                /* tp_richcompare */
    0,                                                /* tp_weaklistoffset */
    (getiterfunc)OrderIter_new,                       /* tp_iter */
    0,                                                /* tp_iternext */
    0,                                                /* tp_methods */
    0,                                                /* tp_members */
    Level_getseters,                                  /* tp_getset */
    0,                                                /* tp_base */
    0,                                                /* tp_dict */
    0,                                                /* tp_descr_get */
    0,                                                /* tp_descr_set */
    0,                                                /* tp_dictoffset */
    0,                                                /* tp_init */
    0,                                                /* tp_alloc */
    0,                                                /* tp_new */
};

static PyObject *Level_new(fm_level_t *level, Levels *levels) {
  PyTypeObject *type = &LevelType;
  Level *self = nullptr;
  self = (Level *)type->tp_alloc(type, 0);
  if (!self)
    return nullptr;
  self->level_ = level;
  self->levels_ = fmc::python::object::from_borrowed((PyObject *)levels);
  return (PyObject *)self;
}

// Level iterator

PyObject *LevelIter_iter(PyObject *self) {
  LevelIter *p = (LevelIter *)self;
  p->done_ = 0;
  return self;
}

PyObject *LevelIter_iternext(PyObject *self) {
  LevelIter *p = (LevelIter *)self;
  if (p->done_ >= fm_book_levels_size(p->levels()->levels_)) {
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }
  auto *ret = PyTuple_New(2);
  auto *level = fm_book_level(p->levels()->levels_, p->done_++);
  PyTuple_SET_ITEM(ret, 0, Decimal128_new(fm_book_level_prx(level)));
  PyTuple_SET_ITEM(ret, 1, Level_new(level, p->levels()));
  return ret;
}

static void LevelIter_dealloc(LevelIter *self) {
  self->~LevelIter();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyTypeObject LevelIterType = {
    PyVarObject_HEAD_INIT(NULL, 0) "LevelIter", /* tp_name */
    sizeof(LevelIter),                          /* tp_basicsize */
    0,                                          /* tp_itemsize */
    (destructor)LevelIter_dealloc,              /* tp_dealloc */
    0,                                          /* tp_print */
    0,                                          /* tp_getattr */
    0,                                          /* tp_setattr */
    0,                                          /* tp_reserved */
    0,                                          /* tp_repr */
    0,                                          /* tp_as_number */
    0,                                          /* tp_as_sequence */
    0,                                          /* tp_as_mapping */
    0,                                          /* tp_hash  */
    0,                                          /* tp_call */
    0,                                          /* tp_str */
    0,                                          /* tp_getattro */
    0,                                          /* tp_setattro */
    0,                                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,   /* tp_flags */
    "order iterator objects",                   /* tp_doc */
    0,                                          /* tp_traverse */
    0,                                          /* tp_clear */
    0,                                          /* tp_richcompare */
    0,                                          /* tp_weaklistoffset */
    LevelIter_iter,                             /* tp_iter */
    LevelIter_iternext,                         /* tp_iternext */
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

PyObject *LevelIter_new(Levels *levels) {
  PyTypeObject *type = &LevelIterType;
  LevelIter *self = nullptr;
  self = (LevelIter *)type->tp_alloc(type, 0);
  if (!self)
    return nullptr;
  self->done_ = 0;
  self->levels_ = fmc::python::object::from_borrowed((PyObject *)levels);
  return (PyObject *)self;
}

// Levels

static int Levels_mp_length(Levels *ref) {
  return fm_book_levels_size(ref->levels_);
}

static PyObject *Levels_mp_subscript(Levels *ref, PyObject *key) {
  long sz = fm_book_levels_size(ref->levels_);
  if (PyFloat_Check(key)) {
    auto px =
        fmc::conversion<double, fmc_decimal128_t>()(PyFloat_AsDouble(key));
    for (auto i = 0U; i < sz; ++i) {
      auto *lvl = fm_book_level(ref->levels_, i);
      if (fm_book_level_prx(lvl) == px) {
        return Level_new(lvl, ref);
      }
    }
    PyErr_SetString(PyExc_IndexError, "Invalid price");
    return nullptr;
  } else if (PyLong_Check(key)) {
    auto k = PyLong_AsLong(key);
    if (k < 0 && k >= -sz) {
      return Level_new(fm_book_level(ref->levels_, sz + k), ref);
    } else if (k >= 0 && k < sz) {
      return Level_new(fm_book_level(ref->levels_, k), ref);
    }
    PyErr_SetString(PyExc_IndexError, "Invalid index");
    return nullptr;
  }
  PyErr_SetString(PyExc_IndexError,
                  "Unsupported key, please use an integer index or a price");
  return nullptr;
}

static int Levels_mp_ass_subscript(Levels *ref, PyObject *key,
                                   PyObject *other) {
  PyErr_SetString(PyExc_RuntimeError, "Levels assignment is not supported");
  return -1;
}

static PyMappingMethods Levels_as_mapping = {
    (lenfunc)Levels_mp_length,              /* mp_length */
    (binaryfunc)Levels_mp_subscript,        /* mp_subscript */
    (objobjargproc)Levels_mp_ass_subscript, /* mp_ass_subscript */
};

static void Levels_dealloc(Levels *self) {
  self->~Levels();
  Py_TYPE(self)->tp_free((PyObject *)self);
}

PyTypeObject LevelsType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Levels", /* tp_name */
    sizeof(Levels),                                    /* tp_basicsize */
    0,                                                 /* tp_itemsize */
    (destructor)Levels_dealloc,                        /* tp_dealloc */
    0,                                                 /* tp_print */
    0,                                                 /* tp_getattr */
    0,                                                 /* tp_setattr */
    0,                                                 /* tp_reserved */
    0,                                                 /* tp_repr */
    0,                                                 /* tp_as_number */
    0,                                                 /* tp_as_sequence */
    &Levels_as_mapping,                                /* tp_as_mapping */
    0,                                                 /* tp_hash  */
    0,                                                 /* tp_call */
    0,                                                 /* tp_str */
    0,                                                 /* tp_getattro */
    0,                                                 /* tp_setattro */
    0,                                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,          /* tp_flags */
    "Levels objects",                                  /* tp_doc */
    0,                                                 /* tp_traverse */
    0,                                                 /* tp_clear */
    0,                                                 /* tp_richcompare */
    0,                                                 /* tp_weaklistoffset */
    (getiterfunc)LevelIter_new,                        /* tp_iter */
    0,                                                 /* tp_iternext */
    0,                                                 /* tp_methods */
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

static PyObject *Levels_new(fm_levels_t *levels, Book *book) {
  PyTypeObject *type = &LevelsType;
  Levels *self = nullptr;
  self = (Levels *)type->tp_alloc(type, 0);
  if (!self)
    return nullptr;
  self->levels_ = levels;
  self->book_ = fmc::python::object::from_borrowed((PyObject *)book);
  return (PyObject *)self;
}
