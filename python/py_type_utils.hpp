#pragma once

extern "C" {
#include <fmc/decimal128.h>
}

#include <Python.h>

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
      if (PyUnicode_Check(temp)) {
        Py_ssize_t sz = 0;
        const char *str = PyUnicode_AsUTF8AndSize(temp, &sz);
        if (sz > FMC_DECIMAL128_STR_SIZE) {
          PyErr_SetString(PyExc_TypeError, "expecting a valid string value");
          return false;
        }
        fmc_error_t *err;
        fmc_decimal128_from_str(&val, str, &err);
        return !bool(err);
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
    }
    PyErr_SetString(PyExc_TypeError, "unknown type");
    return false;
  }
};
