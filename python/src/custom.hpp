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
 * @file custom.cpp
 * @author Andres Rangel
 * @date 13 Mar 2019
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "frame.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <Python.h>

#pragma once

using namespace fm;
using namespace python;

struct custom_cl {
  custom_cl(object op_obj) : op_obj_(op_obj) {}
  bool init(fm_frame_t *result, fm_exec_ctx_t *ctx, size_t argc,
            const fm_frame_t *const argv[]) {
    inputs_ = object::from_new(PyTuple_New(argc + 1));
    PyTuple_SET_ITEM(inputs_.get_ref(), 0, ExtractorFrame_new(result, false));
    for (size_t i = 0; i < argc; ++i) {
      PyTuple_SET_ITEM(inputs_.get_ref(), i + 1,
                       ExtractorFrame_new(argv[i], true));
    }
    if (PyErr_Occurred()) {
      set_python_error(ctx);
      return false;
    }
    auto ret = object::from_new(
        PyObject_CallObject(op_obj_["init"].get_ref(), inputs_.get_ref()));
    if (PyErr_Occurred()) {
      set_python_error(ctx);
      return false;
    }

    if (PyObject_IsTrue(ret.get_ref())) {
      return true;
    } else if (!PyBool_Check(ret.get_ref())) {
      fm_exec_ctx_error_set(ctx,
                            "Value returned by init method must be boolean");
      return false;
    }
    return false;
  }
  bool exec(fm_exec_ctx_t *ctx) {
    auto ret = object::from_new(
        PyObject_CallObject(op_obj_["exec"].get_ref(), inputs_.get_ref()));
    if (PyErr_Occurred()) {
      set_python_error(ctx);
      return false;
    }

    if (PyObject_IsTrue(ret.get_ref())) {
      return true;
    } else if (!PyBool_Check(ret.get_ref())) {
      fm_exec_ctx_error_set(ctx,
                            "Value returned by init method must be boolean");
      return false;
    }
    return false;
  }
  object inputs_;
  object op_obj_;
  object data_;
};

bool fm_comp_custom_call_stream_init(fm_frame_t *result, size_t args,
                                     const fm_frame_t *const argv[],
                                     fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto *pycl = (custom_cl *)ctx->comp;
  return pycl->init(result, ctx->exec, args, argv);
}

bool fm_comp_custom_stream_exec(fm_frame_t *result, size_t args,
                                const fm_frame_t *const argv[],
                                fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *pycl = (custom_cl *)ctx->comp;
  return pycl->exec(ctx->exec);
}

fm_call_def *fm_comp_custom_stream_call(fm_comp_def_cl comp_cl,
                                        const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_custom_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_custom_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_custom_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                 unsigned argc, fm_type_decl_cp argv[],
                                 fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  fm::python::object op_class;

  auto args_sz = ptype ? fm_type_tuple_size(ptype) : 0;
  int j = !closure;
  if (closure != nullptr) {
    op_class = fm::python::object::from_borrowed((PyObject *)closure);
    ++args_sz;
  } else {
    auto error = [&]() {
      const char *errstr =
          "expecting a python class with init and exec methods "
          "and arguments for constructor";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    };

    if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) < 1)
      return error();

    auto rec_t = fm_record_type_get(sys, "PyObject*", sizeof(PyObject *));
    auto *param0 = fm_type_tuple_arg(ptype, 0);
    if (!fm_type_is_record(param0) || !fm_type_equal(rec_t, param0))
      return error();
    op_class = fm::python::object::from_borrowed(STACK_POP(plist, PyObject *));
    if (!PyType_Check(op_class.get_ref())) {
      return error();
    }
  }

  if (!PyObject_GetAttrString(op_class.get_ref(), "init")) {
    const char *errstr = "unable to find init method";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }
  if (!PyObject_GetAttrString(op_class.get_ref(), "exec")) {
    const char *errstr = "unable to find exec method";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto type_tup = fm::python::object::from_new(PyTuple_New(argc));
  for (size_t i = 0; i < argc; ++i) {
    auto nfields = fm_type_frame_nfields(argv[i]);
    auto fields_tup = fm::python::object::from_new(PyTuple_New(nfields));
    for (size_t j = 0; j < nfields; ++j) {
      auto f_type = fm_type_frame_field_type(argv[i], j);
      auto field_tup = fm::python::object::from_new(PyTuple_New(2));
      auto py_type = fm::python::object::from_new(
          (PyObject *)py_type_from_fm_type(f_type));
      if (!(bool)py_type) {
        const char *errstr = "Unsupported type in input";
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
        return nullptr;
      }
      PyTuple_SET_ITEM(
          field_tup.get_ref(), 1,
          PyUnicode_FromString(fm_type_frame_field_name(argv[i], j)));
      PyTuple_SET_ITEM(field_tup.get_ref(), 0, py_type.steal_ref());
      PyTuple_SET_ITEM(fields_tup.get_ref(), j, field_tup.steal_ref());
    }
    PyTuple_SET_ITEM(type_tup.get_ref(), i, fields_tup.steal_ref());
  }

  auto raw_op_args = object::from_new(PyTuple_New(args_sz));
  PyTuple_SET_ITEM(raw_op_args.get_ref(), 0, type_tup.steal_ref());

  for (unsigned i = 1; i < args_sz; ++i, ++j) {
    auto param0 = fm_type_tuple_arg(ptype, j);
    PyTuple_SET_ITEM(raw_op_args.get_ref(), i,
                     get_py_obj_from_arg_stack(param0, plist));
  }

  auto op_obj = object::from_new(
      PyObject_CallObject(op_class.get_ref(), raw_op_args.get_ref()));

  if (PyErr_Occurred()) {
    set_python_error(csys);
    return nullptr;
  }
  if (!PyCallable_Check(op_obj["init"].get_ref())) {
    const char *errstr = "exec is not callable";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }
  if (!PyCallable_Check(op_obj["exec"].get_ref())) {
    const char *errstr = "exec is not callable";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto type_error = [&]() {
    const char *errstr = "returning type must be a tuple of tuples with the "
                         "name and type of the output frame fields.";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!PyObject_GetAttrString(op_obj.get_ref(), "type")) {
    return type_error();
  }

  auto out_type_tup = op_obj["type"];

  if (!PyTuple_Check(out_type_tup.get_ref())) {
    return type_error();
  }

  auto tup_size = PyTuple_Size(out_type_tup.get_ref());
  if (tup_size != 2) {
    const char *errstr = "expecting dimensions and fields tuple description.";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto raw_dim = fm::python::object::from_borrowed(
      PyTuple_GetItem(out_type_tup.get_ref(), 0));

  int dim = PyLong_AsLong(raw_dim.get_ref());

  if (PyErr_Occurred()) {
    set_python_error(csys);
    return nullptr;
  }

  auto raw_type_tup = fm::python::object::from_borrowed(
      PyTuple_GetItem(out_type_tup.get_ref(), 1));

  if (!PyTuple_Check(raw_type_tup.get_ref())) {
    return type_error();
  }

  tup_size = PyTuple_Size(raw_type_tup.get_ref());
  if (tup_size < 1) {
    const char *errstr = "expecting the description of at least one field";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  vector<const char *> names(tup_size);
  vector<fm_type_decl_cp> types(tup_size);
  int dims[1] = {dim};

  for (int i = 0; i < tup_size; ++i) {
    auto item = fm::python::object::from_borrowed(
        PyTuple_GetItem(raw_type_tup.get_ref(), i));
    if (!PyTuple_Check(item.get_ref())) {
      return type_error();
    }
    if (PyTuple_Size(item.get_ref()) != 2) {
      return type_error();
    }
    auto raw_name =
        fm::python::object::from_borrowed(PyTuple_GetItem(item.get_ref(), 1));
    if (!PyUnicode_Check(raw_name.get_ref())) {
      const char *errstr = "Provided name is not a string";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    }
    auto raw_type =
        fm::python::object::from_borrowed(PyTuple_GetItem(item.get_ref(), 0));
    if (!PyType_Check(raw_type.get_ref()) &&
        !PyObject_IsInstance(raw_type.get_ref(),
                             (PyObject *)&ExtractorArrayTypeType)) {
      const char *errstr = "Provided field type is not a supported type";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    }

    names[i] = PyUnicode_AsUTF8(raw_name.get_ref());
    types[i] = fm_type_from_py_type(sys, raw_type.get_ref());

    if (types[i] == nullptr) {
      const char *errstr =
          "Unable to generate Extractor type from provided Python type";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    }
  }

  if (PyErr_Occurred()) {
    set_python_error(csys);
    return nullptr;
  }

  auto *type =
      fm_frame_type_get1(sys, tup_size, names.data(), types.data(), 1, dims);
  if (!type)
    return nullptr;

  auto *cl = new custom_cl(op_obj);
  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_custom_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_custom_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (custom_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}

const fm_comp_def_t fm_comp_custom = {"custom", &fm_comp_custom_gen,
                                      &fm_comp_custom_destroy, NULL};
