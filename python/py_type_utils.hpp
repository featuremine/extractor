#pragma once

#include <fmc/decimal128.h>

#include <Python.h>
#include <extractor/python/extractor.h>
#include <fenv.h>

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
      }
    } else if constexpr (is_same_v<T, RPRICE>) {
      PyObject *temp;
      if (!PyArg_ParseTuple(args, "O", &temp)) {
        PyErr_SetString(PyExc_TypeError, "Expect single argument");
        return false;
      }
      if (PyFloat_Check(temp)) {
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
      if (PyFloat_Check(temp)) {
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
