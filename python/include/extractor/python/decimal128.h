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
 * @file decimal128.h
 * @date 9 Nov 2022
 * @brief File contains C Python api for Decimal128 Type
 *
 * This file contains C Python api for Decimal128 Type
 * @see http://www.featuremine.com
 */

#pragma once

#include <Python.h>

#include "fmc/decimal128.h"
#include "fmc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

FMMODFUNC bool Decimal128_Check(PyObject *obj);

FMMODFUNC fmc_decimal128_t Decimal128_val(PyObject *obj);

FMMODFUNC PyObject *Decimal128_new(fmc_decimal128_t obj);

#ifdef __cplusplus
}
#endif
