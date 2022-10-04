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
 * @file py_extractor.cpp
 * @author Maxim Trokhimtchouk
 * @date 5 Oct 2017
 * @brief Python extension for extractor library
 *
 * This file contains Python C extention for extractor library
 */

extern "C" {
#include "py_extractor.h"
#include "py_side.h"
#include "python/book/py_book.h"
#include "ytp.h"
}

#include "python/custom.hpp"
#include "python/live_batch.hpp"
#include "python/live_poll.hpp"
#include "python/pandas_play.hpp"
#include "python/py_comp.hpp"
#include "python/py_comp_sys.hpp"
#include "python/py_frame.hpp"
#include "python/py_graph.hpp"
#include "python/py_module.hpp"
#include "python/py_play.hpp"
#include "python/py_side.hpp"
#include "python/py_utils.hpp"
#include "python/sim_poll.hpp"
#include "python/tuple_msg.hpp"
#include "python/tuple_split.hpp"

#include "fmc/files.h"
#include "fmc/platform.h"
#include "fmc/test.h"

#include "ytp/py_wrapper.h"

#include <Python.h>
#include <numpy/arrayobject.h>
#include <python/py_wrapper.hpp>

static PyObject *Extractor_fflush(PyObject *self, PyObject *args) {
  fmc_fflush();
  Py_RETURN_NONE;
}

static PyObject *Extractor_set_license(PyObject *self, PyObject *args) {
  const char *file_name;
  if (!PyArg_ParseTuple(args, "s", &file_name)) {
    PyErr_SetString(PyExc_RuntimeError, "expecting license file");
    return nullptr;
  }
  license_file_unit(file_name);
  Py_RETURN_NONE;
}

static PyObject *Extractor_assert_base(PyObject *self, PyObject *args) {
  const char *base;
  const char *test;
  if (!PyArg_ParseTuple(args, "ss", &base, &test)) {
    PyErr_SetString(PyExc_RuntimeError, "expecting base and test files");
    return nullptr;
  }
  try {
    if (!fmc_run_base_vs_test_diff(base, test)) {
      std::string err_msg(base);
      err_msg.append(" not equal to ");
      err_msg.append(test);
      PyErr_SetString(PyExc_RuntimeError, err_msg.c_str());
      return nullptr;
    }
  } catch (exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return nullptr;
  }
  Py_RETURN_NONE;
}

static PyObject *Extractor_assert_numdiff(PyObject *self, PyObject *args) {
  const char *base;
  const char *test;

  if (!PyArg_ParseTuple(args, "ss", &base, &test)) {
    PyErr_SetString(PyExc_RuntimeError, "expecting base and test files");
    return nullptr;
  }
  try {
    if (!fmc_numdiff_base_vs_test(base, test)) {
      std::string err_msg(base);
      err_msg.append(" not equal to ");
      err_msg.append(test);
      PyErr_SetString(PyExc_RuntimeError, err_msg.c_str());
      return nullptr;
    }
  } catch (exception &e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return nullptr;
  }
  Py_RETURN_NONE;
}

static PyObject *Extractor_result_as_pandas(PyObject *self, PyObject *args,
                                            PyObject *keywds) {
  PyObject *raw_obj = nullptr;
  char *index = nullptr;

  static char *kwlist[] = {(char *)"index", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "O|s", kwlist, &raw_obj,
                                   &index)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse keywords");
    return nullptr;
  }

  if (!PyObject_TypeCheck(raw_obj, &ExtractorComputationType)) {
    PyErr_SetString(PyExc_TypeError, "Argument provided must be an "
                                     "Extractor Computation");
    return nullptr;
  }

  ExtractorComputation *obj = (ExtractorComputation *)raw_obj;

  auto *result = fm_data_get(fm_result_ref_get(obj->comp_));
  if (result == nullptr) {
    PyErr_SetString(PyExc_RuntimeError, "Computation must be initialized "
                                        "to access result frame");
    return nullptr;
  }

  return result_as_pandas(result, index);
}

static PyMethodDef extractorMethods[] = {
    {"flush", Extractor_fflush, METH_NOARGS,
     "Flush user space file buffers.\n"
     "No arguments are required."},
    {"assert_base", Extractor_assert_base, METH_VARARGS,
     "Compares two files.\n"
     "Expects the paths of the files to compare to be passed as arguments."},
    {"assert_numdiff", Extractor_assert_numdiff, METH_VARARGS,
     "Compares two files with numdiff.\n"
     "Expects the paths of the files to compare to be passed as arguments."},
    {"set_license", Extractor_set_license, METH_VARARGS,
     "Set Extractor license.\n"
     "Expects as a single argument the path of the license or a command to "
     "pipe the license.\n"
     "When the license is piped with a command, the string to specify the "
     "command must end with the pipe character '|'."},
    {"result_as_pandas", (PyCFunction)Extractor_result_as_pandas,
     METH_VARARGS | METH_KEYWORDS,
     "Returns the result frame of the specified feature as a pandas "
     "dataframe.\n"
     "The desired computation object must be passed as the first argument.\n"
     "As an optional second parameter, the column name of the desired index "
     "can be specified."},
    {NULL, NULL, 0, NULL} /* Sentinel */
};

static PyModuleDef extractormodule = {PyModuleDef_HEAD_INIT,
                                      "extractor",
                                      "FeatureMine Extractor extension module.",
                                      -1,
                                      extractorMethods,
                                      NULL,
                                      NULL,
                                      NULL,
                                      NULL};

static int python_to_stack_arg(fm_type_sys_t *tsys, PyObject *obj,
                               comp_array &inputs, fm_arg_stack_t *&s,
                               fm_type_decl_cp *type) {
  if (PyUnicode_Check(obj)) {
    const char *str = PyUnicode_AsUTF8(obj);
    HEAP_STACK_PUSH(s, str);
    *type = fm_cstring_type_get(tsys);
  } else if (PyBool_Check(obj)) {
    bool val = obj == Py_True;
    HEAP_STACK_PUSH(s, val);
    *type = fm_base_type_get(tsys, FM_TYPE_BOOL);
  } else if (PyLong_Check(obj)) {
    int64_t num = PyLong_AsLongLong(obj);
    HEAP_STACK_PUSH(s, num);
    *type = fm_base_type_get(tsys, FM_TYPE_INT64);
  } else if (PyFloat_Check(obj)) {
    double num = PyFloat_AsDouble(obj);
    HEAP_STACK_PUSH(s, num);
    *type = fm_base_type_get(tsys, FM_TYPE_FLOAT64);
  } else if (PyTuple_Check(obj)) {
    auto size = PyTuple_GET_SIZE(obj);
    vector<fm_type_decl_cp> types(size);
    int count = 0;
    for (int i = 0; i < size; ++i) {
      auto res = python_to_stack_arg(tsys, PyTuple_GET_ITEM(obj, i), inputs, s,
                                     &types[count]);
      if (res != 0)
        return res;
      if (types[count] != nullptr)
        ++count;
    }
    *type = fm_tuple_type_get1(tsys, count, types.data());
  } else if (PyObject_TypeCheck(obj, &ExtractorComputationType) ||
             PyObject_TypeCheck(obj, &ExtractorModuleComputationType)) {
    visit(
        fmc::overloaded{
            [obj, type](vector<fm_comp_t *> &inps) {
              inps.push_back(((ExtractorComputation *)obj)->comp_);
              *type = nullptr;
            },
            [obj, type](vector<fm_module_comp_t *> &inps) {
              inps.push_back(((ExtractorModuleComputation *)obj)->comp_);
              *type = nullptr;
            },
        },
        inputs);
  } else if (PyBook_Check(obj)) {
    auto *shared_book = PyBook_SharedBook(obj);
    HEAP_STACK_PUSH(s, shared_book);
    *type = fm_record_type_get(tsys, "fm_book_shared_t*",
                               sizeof(fm_book_shared_t *));
  } else if (PyYTPSequence_Check(obj)) {
    ytp_sequence_wrapper stream;
    stream.sequence = PyYTPSequence_Shared(obj);
    HEAP_STACK_PUSH(s, stream);
    *type = fm_record_type_get(tsys, "ytp_sequence_wrapper",
                               sizeof(ytp_sequence_wrapper));
  } else if (PyYTPStream_Check(obj)) {
    ytp_stream_wrapper stream;
    stream.sequence = PyYTPStream_Shared(obj);
    stream.peer = PyYTPStream_PeerId(obj);
    stream.channel = PyYTPStream_ChannelId(obj);
    HEAP_STACK_PUSH(s, stream);
    *type = fm_record_type_get(tsys, "ytp_stream_wrapper",
                               sizeof(ytp_stream_wrapper));
  } else if (PyYTPPeer_Check(obj)) {
    ytp_peer_wrapper peer;
    peer.sequence = PyYTPPeer_Shared(obj);
    peer.peer = PyYTPPeer_Id(obj);
    HEAP_STACK_PUSH(s, peer);
    *type =
        fm_record_type_get(tsys, "ytp_peer_wrapper", sizeof(ytp_peer_wrapper));
  } else if (PyYTPChannel_Check(obj)) {
    ytp_channel_wrapper channel;
    channel.sequence = PyYTPChannel_Shared(obj);
    channel.channel = PyYTPChannel_Id(obj);
    HEAP_STACK_PUSH(s, channel);
    *type = fm_record_type_get(tsys, "ytp_channel_wrapper",
                               sizeof(ytp_channel_wrapper));
  } else if (PyDelta_Check(obj) ||
             fm::python::datetime::is_pandas_timestamp_type(obj)) {
    fm::python::datetime dt(fm::python::object::from_borrowed(obj));
    auto tm = static_cast<fm_time64_t>(dt);
    HEAP_STACK_PUSH(s, tm);
    *type = fm_base_type_get(tsys, FM_TYPE_TIME64);
  } else if (PyObject_TypeCheck(obj, &ExtractorModuleType)) {
    auto m = ((ExtractorModule *)obj)->features->m_;
    HEAP_STACK_PUSH(s, m);
    *type = fm_module_type_get(tsys, fm_module_inps_size(m),
                               fm_module_outs_size(m));
  } else if (auto td = fm_type_from_py_type(tsys, obj); td) {
    HEAP_STACK_PUSH(s, td);
    *type = fm_type_type_get(tsys);
  } else if (auto td = fm_type_from_py_obj(tsys, obj, s); td) {
    *type = td;
  } else {
    HEAP_STACK_PUSH(s, obj);
    *type = fm_record_type_get(tsys, "PyObject*", sizeof(PyObject *));
  }
  return 0;
}

bool fm_comp_sys_py_comp(fm_comp_sys_t *sys) {
  return fm_comp_type_add(sys, &fm_comp_pandas_play) &&
         fm_comp_type_add(sys, &fm_comp_scheduled_play) &&
         fm_comp_type_add(sys, &fm_comp_immediate_play) &&
         fm_comp_type_add(sys, &fm_comp_live_poll) &&
         fm_comp_type_add(sys, &fm_comp_live_batch) &&
         fm_comp_type_add(sys, &fm_comp_sim_poll) &&
         fm_comp_type_add(sys, &fm_comp_tuple_msg) &&
         fm_comp_type_add(sys, &fm_comp_tuple_split) &&
         fm_comp_type_add(sys, &fm_comp_custom);
}

PyMODINIT_FUNC fm_extractor_py_init(void) {
  static bool numpy_init_ = false;
  if (!numpy_init_) {
    import_array();
    numpy_init_ = true;
  }

  PyDateTime_IMPORT;

  PyObject *m;

  m = PyModule_Create(&extractormodule);
  if (m == NULL)
    return NULL;

  if (PyModule_AddStringConstant(m, "__version__", PY_EXTR_VER) == -1)
    return NULL;

  if (!init_type_wrappers(m))
    return NULL;

  if (PyType_Ready(&ExtractorStreamContextType) < 0)
    return NULL;
  Py_INCREF(&ExtractorStreamContextType);
  PyModule_AddObject(m, "StreamContext",
                     (PyObject *)&ExtractorStreamContextType);

  if (PyType_Ready(&ExtractorGraphType) < 0)
    return NULL;
  Py_INCREF(&ExtractorGraphType);
  PyModule_AddObject(m, "Graph", (PyObject *)&ExtractorGraphType);

  if (PyType_Ready(&ExtractorModuleType) < 0)
    return NULL;
  Py_INCREF(&ExtractorModuleType);
  PyModule_AddObject(m, "Module", (PyObject *)&ExtractorModuleType);

  if (PyType_Ready(&ExtractorFeaturesType) < 0)
    return NULL;
  Py_INCREF(&ExtractorFeaturesType);
  PyModule_AddObject(m, "Features", (PyObject *)&ExtractorFeaturesType);

  if (PyType_Ready(&ExtractorModuleFeaturesType) < 0)
    return NULL;
  Py_INCREF(&ExtractorModuleFeaturesType);
  PyModule_AddObject(m, "ModuleFeatures",
                     (PyObject *)&ExtractorModuleFeaturesType);

  if (PyType_Ready(&ExtractorFeatureType) < 0)
    return NULL;
  Py_INCREF(&ExtractorFeatureType);
  PyModule_AddObject(m, "Feature", (PyObject *)&ExtractorFeatureType);

  if (PyType_Ready(&ExtractorFeatureIterType) < 0)
    return NULL;
  Py_INCREF(&ExtractorFeatureIterType);
  PyModule_AddObject(m, "FeatureIter", (PyObject *)&ExtractorFeatureIterType);

  if (PyType_Ready(&ExtractorModuleFeatureType) < 0)
    return NULL;
  Py_INCREF(&ExtractorModuleFeatureType);
  PyModule_AddObject(m, "ModuleFeature",
                     (PyObject *)&ExtractorModuleFeatureType);

  if (PyType_Ready(&ExtractorComputationType) < 0)
    return NULL;
  Py_INCREF(&ExtractorComputationType);
  PyModule_AddObject(m, "Feature", (PyObject *)&ExtractorComputationType);

  if (PyType_Ready(&ExtractorComputationDescriptionIterType) < 0)
    return NULL;
  Py_INCREF(&ExtractorComputationDescriptionIterType);
  PyModule_AddObject(m, "FeatureDescriptionIter",
                     (PyObject *)&ExtractorComputationDescriptionIterType);

  if (PyType_Ready(&ExtractorModuleComputationType) < 0)
    return NULL;
  Py_INCREF(&ExtractorModuleComputationType);
  PyModule_AddObject(m, "ModuleFeature",
                     (PyObject *)&ExtractorModuleComputationType);

  if (PyType_Ready(&ExtractorResultRefType) < 0)
    return NULL;
  Py_INCREF(&ExtractorResultRefType);
  PyModule_AddObject(m, "ResultRef", (PyObject *)&ExtractorResultRefType);

  if (PyType_Ready(&ExtractorFrameType) < 0)
    return NULL;
  Py_INCREF(&ExtractorFrameType);
  PyModule_AddObject(m, "Frame", (PyObject *)&ExtractorFrameType);

  if (PyType_Ready(&ExtractorSubFrameType) < 0)
    return NULL;
  Py_INCREF(&ExtractorSubFrameType);
  PyModule_AddObject(m, "SubFrame", (PyObject *)&ExtractorSubFrameType);

  if (PyType_Ready(&ExtractorFrameIterType) < 0)
    return NULL;
  Py_INCREF(&ExtractorFrameIterType);
  PyModule_AddObject(m, "FrameIter", (PyObject *)&ExtractorFrameIterType);

  if (PyType_Ready(&ExtractorSubFrameIterType) < 0)
    return NULL;
  Py_INCREF(&ExtractorSubFrameIterType);
  PyModule_AddObject(m, "SubFrameIter", (PyObject *)&ExtractorSubFrameIterType);

  if (PyType_Ready(&ExtractorSystemType) < 0)
    return NULL;
  Py_INCREF(&ExtractorSystemType);
  PyModule_AddObject(m, "System", (PyObject *)&ExtractorSystemType);
  PyModule_AddObject(m, "system", ExtractorSystem_new());

  if (!TradeSide_AddType(m))
    return NULL;

  if (!PyBook_AddTypes(m))
    return NULL;

  return m;
}
