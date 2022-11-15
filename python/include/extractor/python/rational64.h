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
 * @file rational64.h
 * @date 9 Nov 2022
 * @brief File contains C Python api for Rational64 Type
 *
 * This file contains C Python api for Rational64 Type
 * @see http://www.featuremine.com
 */

#pragma once

#include <Python.h>

#include "fmc/platform.h"
#include "fmc/rational64.h"

#ifdef __cplusplus
extern "C" {
#endif

FMMODFUNC bool Rational64_Check(PyObject *obj);

FMMODFUNC fmc_rational64_t Rational64_val(PyObject *obj);

#ifdef __cplusplus
}
#endif
