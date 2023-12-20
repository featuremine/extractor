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
#include <fmc/fxpt128.h>

#include <Python.h>
#include <extractor/python/decimal128.h>
#include <extractor/python/fxpt128.h>
#include <extractor/python/rational64.h>
#include <extractor/python/rprice.h>
#include <fenv.h>

#include <extractor/python/decimal.h>

template <bool B> struct integral_value { typedef long long type; };
template <> struct integral_value<true> { typedef unsigned long long type; };

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
    } else if constexpr (is_same_v<T, FIXEDPOINT128>) {
      PyObject *temp;
      if (!PyArg_ParseTuple(args, "O", &temp)) {
        PyErr_SetString(PyExc_TypeError, "Expect single argument");
        return false;
      }
      if (FixedPoint128_Check(temp)) {
        val = FixedPoint128_val(temp);
        return !PyErr_Occurred();
      } else if (Rprice_Check(temp)) {
        fmc_rprice_t x = Rprice_val(temp);
        fmc_fxpt128_from_rprice(&val, &x);
        return true;
      } else if (PyFloat_Check(temp)) {
        fmc_fxpt128_from_double(&val, PyFloat_AsDouble(temp));
        return true;
      } else if (PyUnicode_Check(temp)) {
        Py_ssize_t sz = 0;
        const char *str = PyUnicode_AsUTF8AndSize(temp, &sz);
        if (sz > FMC_FXPT128_STR_SIZE) {
          PyErr_SetString(PyExc_TypeError, "expecting a valid string value");
          return false;
        }
        const char *end = nullptr;
        fmc_fxpt128_from_string(&val, str, &end);
        if (str + strlen(str) != end) {
          PyErr_SetString(PyExc_TypeError, "error converting from string");
          return false;
        }
        return true;
      } else if (PyLong_Check(temp)) {
        PyErr_Clear();
        int64_t i = PyLong_AsLongLong(temp);
        if (PyErr_Occurred()) {
          return false;
        } else {
          fmc_fxpt128_from_int(&val, i);
          return true;
        }
      }
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
        PyDecObject *typed = (PyDecObject *)temp;
        uint16_t flag =
            ((typed->dec.flags & MPD_NEG) == MPD_NEG) * FMC_DECIMAL128_NEG |
            ((typed->dec.flags & MPD_INF) == MPD_INF) * FMC_DECIMAL128_INF |
            ((typed->dec.flags & MPD_NAN) == MPD_NAN) * FMC_DECIMAL128_NAN |
            ((typed->dec.flags & MPD_SNAN) == MPD_SNAN) * FMC_DECIMAL128_SNAN;
        fmc_decimal128_set_triple(&val, typed->dec.data, typed->dec.len,
                                  typed->dec.exp, flag);
        return true;
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
