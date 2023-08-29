/******************************************************************************

        COPYRIGHT (c) 2022 by Featuremine Corporation.
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
 * @file py_api.h
 * @date 4 Oct 2022
 * @brief File contains C declaration of yamal sequence Python API
 *
 * File contains C declaration of yamal sequence Python API
 * @see http://www.featuremine.com
 */

#pragma once

#include <Python.h>
#include <extractor/api.h>

#ifdef __cplusplus
extern "C" {
#endif

struct py_extractor_api_v1 {
  PyObject *(*resultref_new)(fm_result_ref_t *);
  PyObject *(*graph_new)(fm_comp_sys_t *sys, fm_comp_graph_t *graph,
                         bool to_delete);
  bool (*tradeside_check)(PyObject *);
  int (*tradeside_side)(PyObject *);
  PyObject *(*tradeside_bid)();
  PyObject *(*tradeside_ask)();
  PyObject *(*tradeside_unknown)();
};

struct PyExtractorAPIWrapper {
  PyObject_HEAD struct extractor_api_v1 *api;
  struct py_extractor_api_v1 *py_api;
};

typedef struct TradeSideStruct TradeSideS;

FMMODFUNC extern TradeSideS _TradeSide_UNKNOWN;
FMMODFUNC extern TradeSideS _TradeSide_BID;
FMMODFUNC extern TradeSideS _TradeSide_ASK;

#define TradeSide_UNKNOWN ((PyObject *)&_TradeSide_UNKNOWN)
#define TradeSide_BID ((PyObject *)&_TradeSide_BID)
#define TradeSide_ASK ((PyObject *)&_TradeSide_ASK)

FMMODFUNC bool TradeSide_TypeCheck(PyObject *obj);
FMMODFUNC int TradeSide_Side(PyObject *obj);

typedef struct {
  PyObject_HEAD fm_comp_sys_t *sys;
} ExtractorSystemStruct;

typedef struct {
  PyObject_HEAD fm_comp_sys_t *sys_;
  fm_comp_graph_t *graph_;
  fm_comp_t *comp_;
} ExtractorComputation;

#ifdef __cplusplus
}
#endif
