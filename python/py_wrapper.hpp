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
 * @file wrapper.hpp
 * @author Maxim Trokhimtchouk
 * @date 12 Oct 2017
 * @brief File contains C++ python C API wrapper
 *
 * This file provides useful wrappers for python object
 */

#pragma once

#include "fmc/time.h"
#include "fmc++/time.hpp"
#include "fmc++/mpl.hpp"
#include <Python.h>
#include <chrono>
#include <datetime.h>

namespace fm {
namespace python {

class object {
public:
  object() = default;

  object(object &&m) : obj_(m.obj_) { m.obj_ = nullptr; }
  object(const object &c) : obj_(c.obj_) { Py_XINCREF(obj_); }
  ~object() { Py_XDECREF(obj_); }
  object &operator=(object &&o) {
    Py_XDECREF(obj_);
    obj_ = o.obj_;
    o.obj_ = nullptr;
    return *this;
  }
  object &operator=(const object &o) {
    Py_XDECREF(obj_);
    obj_ = o.obj_;
    Py_XINCREF(obj_);
    return *this;
  }
  operator bool() const { return obj_ != nullptr; }
  bool is_None() const { return obj_ == Py_None; }
  object get_attr(const char *attr_name) const {
    PyObject *attr = nullptr;
    if (obj_ != nullptr)
      attr = PyObject_GetAttrString(obj_, attr_name);
    return object::from_new(attr);
  }
  object operator[](const char *attr_name) const { return get_attr(attr_name); }
  object operator[](const std::string &name) const {
    return get_attr(name.c_str());
  }
  object operator[](int pos) const {
    if (obj_ == nullptr)
      return object::from_new(nullptr);
    auto obj = PyTuple_GetItem(obj_, pos);
    return object::from_borrowed(obj);
  }
  template <class... Args> object operator()(Args &&...args) const {
    const size_t n = std::tuple_size<std::tuple<Args...>>::value;
    auto *obj = PyTuple_New(n);
    if constexpr (n > 0) {
      PyObject *argv[n];
      size_t i = 0;
      fmc::for_each([&](auto &&a) { argv[i++] = object(a).steal_ref(); },
                    args...);
      for (size_t i = 0; i < n; ++i) {
        PyTuple_SET_ITEM(obj, i, argv[i]);
      }
    }
    object arg_tuple = object::from_new(obj);
    auto *retobj = PyObject_CallObject(get_ref(), arg_tuple.get_ref());
    return object::from_new(retobj);
  }
  object get_item(object &&idx) {
    auto *obj = PyObject_GetItem(get_ref(), idx.get_ref());
    return object::from_new(obj);
  }
  object iter() { return object::from_new(PyObject_GetIter(obj_)); }
  object next() { return object::from_new(PyIter_Next(obj_)); }
  PyObject *get_ref() const { return obj_; }
  PyObject *steal_ref() {
    auto *ret = obj_;
    obj_ = nullptr;
    return ret;
  }
  std::string str() const {
    if (!obj_)
      return "";
    auto tmp = object(PyObject_Str(obj_));
    if (!tmp)
      return "";
    return PyUnicode_AsUTF8(tmp.get_ref());
  }
  static object from_borrowed(PyObject *obj) {
    Py_XINCREF(obj);
    return object(obj);
  }
  static object from_new(PyObject *obj) { return object(obj); }
  object type() { return object::from_new(PyObject_Type(obj_)); }

private:
  object(PyObject *obj) : obj_(obj) {}
  PyObject *obj_ = nullptr;
};

class module : public object {
public:
  module() :object() {}
  explicit module(const char *name)
      : object(object::from_new(PyImport_ImportModule(name))) {}
};

class datetime : public object {
public:
  datetime(object &&obj) : object(obj) {}
  operator fmc_time64_t() {
    using namespace std::chrono;
    if (PyDelta_Check(get_ref())) {
      auto h = 24 * PyLong_AsLong(PyObject_GetAttrString(get_ref(), "days"));
      auto sec = PyLong_AsLong(PyObject_GetAttrString(get_ref(), "seconds"));
      auto us =
          PyLong_AsLong(PyObject_GetAttrString(get_ref(), "microseconds"));
      return fmc_time64_from_nanos(us * 1000) +
             fmc_time64_from_seconds(h * 3600 + sec);
    } else if (PyFloat_Check(get_ref())) {
      auto fdur = duration<double>(PyFloat_AsDouble(get_ref()));
      auto nanos = duration_cast<nanoseconds>(fdur);
      return fmc_time64_from_nanos(nanos.count());
    } else if (PyLong_Check(get_ref()))
      return fmc_time64_from_nanos(PyLong_AsLongLong(get_ref()));
    else if (is_pandas_timestamp_type(get_ref())) {
      return fmc_time64_from_nanos(
          PyLong_AsLongLong((*this)["value"].get_ref()));
    }
    PyErr_SetString(PyExc_RuntimeError, "unsupported datetime type");
    return fmc_time64_from_nanos(0);
  }

  static bool is_pandas_timestamp_type(PyObject *obj) {
    return strcmp(Py_TYPE(obj)->tp_name, "Timestamp") == 0;
  }

  static object get_pandas_dttz_type() {
    static auto datetime =
        module("pandas")["core"]["dtypes"]["dtypes"]["DatetimeTZDtype"];
    return datetime;
  }
};

} // namespace python
} // namespace fm
