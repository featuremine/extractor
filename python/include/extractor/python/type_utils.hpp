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
 * @file type_utils.hpp
 * @date 2 Nov 2022
 * @brief Python type utilities
 * */

#pragma once

#include <fmc/decimal128.h>

#include <Python.h>
#include <extractor/python/decimal128.h>
#include <extractor/python/extractor.h>
#include <extractor/python/rational64.h>
#include <extractor/python/rprice.h>
#include <fenv.h>

#include <mpdecimal.h>

template <bool B> struct integral_value { typedef long long type; };
template <> struct integral_value<true> { typedef unsigned long long type; };

bool PyDecimal_Check(PyObject *obj){
  PyObject *dectype = PyObject_GetAttrString(PyImport_ImportModule((char *) "decimal"), (char *) "Decimal");
  return PyObject_IsInstance(obj, dectype);
}

#define _Py_DEC_MINALLOC 4

typedef struct {
    PyObject_HEAD
    Py_hash_t hash;
    mpd_t dec;
    mpd_uint_t data[_Py_DEC_MINALLOC];
} PyDecObject;

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
      if (Decimal128_Check(temp)) {
        val = Decimal128_val(temp);
        return !PyErr_Occurred();
      } else if (PyFloat_Check(temp)) {
        fmc_decimal128_from_double(&val, PyFloat_AsDouble(temp));
        return true;
      } else if (PyUnicode_Check(temp)) {
        Py_ssize_t sz = 0;
        const char *str = PyUnicode_AsUTF8AndSize(temp, &sz);
        if (sz > FMC_DECIMAL128_STR_SIZE) {
          PyErr_SetString(PyExc_TypeError, "expecting a valid string value");
          return false;
        }
        fmc_error_t *err;
        feclearexcept(FE_ALL_EXCEPT);
        fmc_decimal128_from_str(&val, str, &err);
        if (err && !fetestexcept(FE_INEXACT)) {
          PyErr_SetString(PyExc_TypeError, "error converting from string");
          return false;
        }
        fmc_decimal128_pretty(&val);
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
      } else if (PyDecimal_Check(temp)) {
        PyDecObject *typed = (PyDecObject*)temp;
        mpd_context_t ctx;
        mpd_ieee_context(&ctx, MPD_DECIMAL128);
        mpd_uint_t buf[2] = {0};
        mpd_t res;
        res.data = buf;
        res.alloc = 2;
        mpd_set_static_data(&res);
        mpd_copy(&res, &typed->dec, &ctx);
        if (mpd_getstatus(&ctx)) {
          return false;
        }
        fmc_error_t *err;s
        fmc_decimal128_set_triple(&val, res.data[0], res.data[1], res.exp, (res.flags & MPD_NEG) == MPD_NEG, res.&err);
        return !bool(err);
      }
    } else if constexpr (is_same_v<T, RPRICE>) {
      PyObject *temp;
      if (!PyArg_ParseTuple(args, "O", &temp)) {
        PyErr_SetString(PyExc_TypeError, "Expect single argument");
        return false;
      }
      if (Rprice_Check(temp)) {
        val = Rprice_val(temp);
        return !PyErr_Occurred();
      } else if (PyFloat_Check(temp)) {
        fmc_rprice_from_double(&val, PyFloat_AsDouble(temp));
        return true;
      } else if (PyLong_Check(temp)) {
        int64_t i = PyLong_AsLongLong(temp);
        if (PyErr_Occurred()) {
          return false;
        } else {
          fmc_rprice_from_int(&val, i);
          return true;
        }
      }
    } else if constexpr (is_same_v<T, RATIONAL64>) {
      PyObject *temp;
      if (!PyArg_ParseTuple(args, "O", &temp)) {
        PyErr_SetString(PyExc_TypeError, "Expect single argument");
        return false;
      }
      if (Rational64_Check(temp)) {
        val = Rational64_val(temp);
        return !PyErr_Occurred();
      } else if (PyFloat_Check(temp)) {
        fmc_rational64_from_double(&val, PyFloat_AsDouble(temp));
        return true;
      } else if (PyLong_Check(temp)) {
        int64_t i = PyLong_AsLongLong(temp);
        if (PyErr_Occurred()) {
          return false;
        } else {
          fmc_rational64_from_int(&val, i);
          return true;
        }
      }
    }
    PyErr_SetString(PyExc_TypeError, "unknown type");
    return false;
  }
};
