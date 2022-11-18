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
 * @file Extractor.h
 * @author Maxim Trokhimtchouk
 * @date 2 Aug 2017
 * @brief File contains C declaration of the call context
 *
 * This file contains declarations of the call context
 * @see http://www.featuremine.com
 */

#pragma once

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <Python.h>

#include "extractor/comp_sys.h"
#include "extractor/frame_base.h"
#include "fmc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief creates extractor frame object
 */
FMMODFUNC PyObject *ExtractorFrame_new(const fm_frame_t *frame,
                                       bool constval = true);

/**
 * @brief creates extractor frame result reference object
 */
FMMODFUNC PyObject *ExtractorResultRef_new(fm_result_ref_t *ref);

/**
 * @brief creates extractor graph object
 */
FMMODFUNC PyObject *ExtractorGraph_new(fm_comp_sys_t *sys,
                                       fm_comp_graph_t *graph, bool to_delete);

/**
 * @brief creates extractor graph object with parent python system object
 */
FMMODFUNC PyObject *ExtractorGraph_py_new(PyObject *py_sys, fm_comp_sys_t *sys,
                                          fm_comp_graph_t *graph,
                                          bool to_delete);

/**
 * @brief checks whether python object is of Extractor System type
 */
FMMODFUNC fm_comp_sys_t *ExtractorSystem_get(PyObject *);

/**
 * @brief Adds python specific computations to extractor system
 */
FMMODFUNC bool fm_comp_sys_py_comp(fm_comp_sys_t *sys);

/**
 * @brief main module init function
 */
PyMODINIT_FUNC fm_extractor_py_init(void) FMMODFUNC;

#ifdef __cplusplus
}
#endif
