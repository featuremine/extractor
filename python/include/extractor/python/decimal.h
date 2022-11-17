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
 * @file decimal.h
 * @date 2 Nov 2022
 * @brief Definitions for python decimal type support
 * */

#pragma once

#include "fmc/platform.h"
#include "libmpdec/mpdecimal.h"
#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

FMMODFUNC PyObject *PyDecimal_Type();

FMMODFUNC bool PyDecimal_Check(PyObject *obj);

#define _Py_DEC_MINALLOC 4

typedef struct {
  PyObject_HEAD Py_hash_t hash;
  mpd_t dec;
  mpd_uint_t data[_Py_DEC_MINALLOC];
} PyDecObject;

#ifdef __cplusplus
}
#endif
