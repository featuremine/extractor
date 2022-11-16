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
 * @file pydecimal.h
 * @date 2 Nov 2022
 * @brief Definitions for python decimal type support
 * */

#include "extractor/python/mpdecimal.h"
#include <Python.h>

PyObject *PyDecimal_Type() {
  static PyObject *dectype = NULL;
  if (!dectype) {
    PyObject *decmodule = PyImport_ImportModule((char *)"decimal");
    if (!decmodule) {
      return NULL;
    }
    dectype = PyObject_GetAttrString(decmodule, (char *)"Decimal");
    Py_XDECREF(decmodule);
    if (!dectype) {
      return NULL;
    }
  }
}

bool PyDecimal_Check(PyObject *obj) {
  PyObject *dectype = PyDecimal_Type();
  if (!dectype) {
    return false;
  }
  return PyObject_IsInstance(obj, dectype);
}

#define _Py_DEC_MINALLOC 4

typedef struct {
  PyObject_HEAD Py_hash_t hash;
  mpd_t dec;
  mpd_uint_t data[_Py_DEC_MINALLOC];
} PyDecObject;
