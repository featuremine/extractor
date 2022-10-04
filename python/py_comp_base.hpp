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
 * @file py_comp_base.hpp
 * @author Andres Rangel
 * @date 31 Mar 2020
 * @brief Python extension for extractor library
 *
 * This file contains Python C extention for extractor library
 */

#pragma once

extern "C" {
#include "comp_sys.h"
#include "src/comp.h"
}

#include <Python.h>

typedef struct {
  PyObject_HEAD fm_comp_sys_t *sys_;
  fm_comp_graph_t *graph_;
  fm_comp_t *comp_;
} ExtractorComputation;

PyObject *ExtractorComputation_new(fm_comp_t *, fm_comp_sys_t *,
                                   fm_comp_graph_t *);

bool ExtractorComputation_type_check(PyObject *);
