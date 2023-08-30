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
 * @file comp_base.hpp
 * @author Andres Rangel
 * @date 31 Mar 2020
 * @brief Python extension for extractor library
 *
 * This file contains Python C extention for extractor library
 */

#pragma once

#include "comp.h"
#include "extractor/comp_sys.h"
#include "extractor/python/py_api.h"

#include <Python.h>

PyObject *ExtractorComputation_new(fm_comp_t *, fm_comp_sys_t *,
                                   fm_comp_graph_t *);

bool ExtractorComputation_type_check(PyObject *);

fm_type_decl_cp fm_type_from_py_type(fm_type_sys_t *tsys, PyObject *obj);

static PyObject *create(PyTypeObject *type, PyObject *args, PyObject *kwds) {
  PyObject *input = NULL;
  static char *kwlist[] = {(char *)"input", NULL};
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &input)) {
    PyErr_SetString(PyExc_RuntimeError, "Unable to parse keywords");
    return nullptr;
  }
  if (!ExtractorComputation_type_check(input)) {
    PyErr_SetString(PyExc_RuntimeError, "Argument is not an extractor"
                                        " computation");
    return nullptr;
  }
  ExtractorComputation *i = (ExtractorComputation *)input;
  fm_comp_t *i_comp = i->comp_;
  fm_comp_sys_t *sys = i->sys_;
  fm_type_sys_t *tsys = fm_type_sys_get(sys);
  fm_comp_graph *graph = i->graph_;
  auto *comp =
      fm_comp_decl(sys, graph, "convert", 1,
                   fm_tuple_type_get(tsys, 1, fm_type_type_get(tsys)), i_comp,
                   fm_type_from_py_type(tsys, (PyObject *)type));
  if (!comp) {
    if (fm_type_sys_errno(tsys) != FM_TYPE_ERROR_OK) {
      PyErr_SetString(PyExc_RuntimeError, fm_type_sys_errmsg(tsys));
    } else if (fm_comp_sys_is_error(sys)) {
      PyErr_SetString(PyExc_RuntimeError, fm_comp_sys_error_msg(sys));
    }
    return nullptr;
  };
  return (PyObject *)ExtractorComputation_new(comp, sys, graph);
}
