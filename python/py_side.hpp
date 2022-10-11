#pragma once

#include "extractor/side.hpp"
#include <sstream>

extern "C" {
#include "extractor/python/py_side.h"
}

struct TradeSideStruct {
  PyObject_VAR_HEAD fm::trade_side side_;
};

static void TradeSide_dealloc(PyObject *self) { Py_TYPE(self)->tp_free(self); }

static PyObject *TradeSide_str(TradeSideS *self) {
  std::ostringstream o;
  o << self->side_;
  return PyUnicode_FromString(o.str().c_str());
}

PyObject *TradeSide_method_UNKNOWN(PyObject *self);

PyObject *TradeSide_method_BID(PyObject *self);

PyObject *TradeSide_method_ASK(PyObject *self);

PyObject *TradeSide_method_other_side(PyObject *self);

static PyMethodDef TradeSide_methods[] = {
    {"other_side", (PyCFunction)TradeSide_method_other_side, METH_NOARGS,
     "Returns the opposite trade side to the side of the object.\n"
     "No arguments are required."},
    {"UNKNOWN", (PyCFunction)TradeSide_method_UNKNOWN,
     METH_NOARGS | METH_STATIC,
     "Returns TradeSide UNKNOWN.\n"
     "No arguments are required."},
    {"BID", (PyCFunction)TradeSide_method_BID, METH_NOARGS | METH_STATIC,
     "Returns TradeSide BID.\n"
     "No arguments are required."},
    {"ASK", (PyCFunction)TradeSide_method_ASK, METH_NOARGS | METH_STATIC,
     "Returns TradeSide ASK.\n"
     "No arguments are required."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyObject *TradeSide_richcompare(PyObject *obj1, PyObject *obj2, int op);

static PyTypeObject TradeSide_type = {
    PyVarObject_HEAD_INIT(NULL, 0) "trade_side", /* tp_name */
    sizeof(TradeSideS),                          /* tp_basicsize */
    0,                                           /* tp_itemsize */
    (destructor)TradeSide_dealloc,               /* tp_dealloc */
    0,                                           /* tp_print */
    0,                                           /* tp_getattr */
    0,                                           /* tp_setattr */
    0,                                           /* tp_reserved */
    (reprfunc)TradeSide_str,                     /* tp_repr */
    0,                                           /* tp_as_number */
    0,                                           /* tp_as_sequence */
    0,                                           /* tp_as_mapping */
    0,                                           /* tp_hash  */
    0,                                           /* tp_call */
    (reprfunc)TradeSide_str,                     /* tp_str */
    0,                                           /* tp_getattro */
    0,                                           /* tp_setattro */
    0,                                           /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,    /* tp_flags */
    "trade_side objects",                        /* tp_doc */
    0,                                           /* tp_traverse */
    0,                                           /* tp_clear */
    TradeSide_richcompare,                       /* tp_richcompare */
    0,                                           /* tp_weaklistoffset */
    0,                                           /* tp_iter */
    0,                                           /* tp_iternext */
    TradeSide_methods,                           /* tp_methods */
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

bool TradeSide_TypeCheck(PyObject *obj) {
  return PyObject_TypeCheck(obj, &TradeSide_type);
}

int TradeSide_Side(PyObject *obj) { return ((TradeSideS *)obj)->side_.value; }

TradeSideS _TradeSide_UNKNOWN = {
    PyVarObject_HEAD_INIT(&TradeSide_type, 0){trade_side::SIDE::UNKNOWN}};

TradeSideS _TradeSide_BID = {
    PyVarObject_HEAD_INIT(&TradeSide_type, 1){trade_side::SIDE::BID}};

TradeSideS _TradeSide_ASK = {
    PyVarObject_HEAD_INIT(&TradeSide_type, 2){trade_side::SIDE::ASK}};

PyObject *TradeSide_method_other_side(PyObject *self) {
  if (self == TradeSide_BID) {
    Py_INCREF(TradeSide_ASK);
    return TradeSide_ASK;
  }
  Py_INCREF(TradeSide_BID);
  return TradeSide_BID;
}

PyObject *TradeSide_method_UNKNOWN(PyObject *self) {
  Py_INCREF(TradeSide_UNKNOWN);
  return TradeSide_UNKNOWN;
}

PyObject *TradeSide_method_BID(PyObject *self) {
  Py_INCREF(TradeSide_BID);
  return TradeSide_BID;
}

PyObject *TradeSide_method_ASK(PyObject *self) {
  Py_INCREF(TradeSide_ASK);
  return TradeSide_ASK;
}

static PyObject *TradeSide_richcompare(PyObject *obj1, PyObject *obj2, int op) {
  switch (op) {
  case Py_LT:
    break;
  case Py_LE:
    break;
  case Py_EQ:
    if (TradeSide_TypeCheck(obj1) && TradeSide_TypeCheck(obj2)) {
      if (obj1 == obj2) {
        Py_RETURN_TRUE;
      }
    }
    break;
  case Py_NE:
    if (TradeSide_TypeCheck(obj1) && TradeSide_TypeCheck(obj2)) {
      if (obj1 != obj2) {
        Py_RETURN_TRUE;
      }
    }
    break;
  case Py_GT:
    break;
  case Py_GE:
    break;
  }
  Py_RETURN_FALSE;
}

PyObject *TradeSide_AddType(PyObject *m) {
  if (PyType_Ready(&TradeSide_type) < 0)
    return nullptr;
  if (m) {
    Py_INCREF(&TradeSide_type);
    if (PyModule_AddObject(m, "trade_side", (PyObject *)&TradeSide_type) < 0) {
      Py_DECREF(&TradeSide_type);
      return nullptr;
    }
  }
  return (PyObject *)&TradeSide_type;
}
