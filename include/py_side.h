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
 * @file py_side.h
 * @author Andres Rangel
 * @date 9 Mar 2020
 * @brief File contains C declaration of the call context
 *
 * This file contains declarations of the call context
 * @see http://www.featuremine.com
 */

#ifndef __FM_PY_SIDE_H__
#define __FM_PY_SIDE_H__

#include <Python.h>

#include <fmc/platform.h>

typedef struct TradeSideStruct TradeSideS;

FMMODFUNC extern TradeSideS _TradeSide_UNKNOWN;
FMMODFUNC extern TradeSideS _TradeSide_BID;
FMMODFUNC extern TradeSideS _TradeSide_ASK;

#define TradeSide_UNKNOWN ((PyObject *)&_TradeSide_UNKNOWN)
#define TradeSide_BID ((PyObject *)&_TradeSide_BID)
#define TradeSide_ASK ((PyObject *)&_TradeSide_ASK)

FMMODFUNC bool TradeSide_TypeCheck(PyObject *obj);
FMMODFUNC int TradeSide_Side(PyObject *obj);

#endif // __FM_PY_SIDE_H__
