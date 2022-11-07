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
#include "extractor/python/py_extractor.h"
}

#include <Python.h>

PyMODINIT_FUNC PyInit_extractor(void) FMMODFUNC FMPYMODPUB;

PyMODINIT_FUNC PyInit_extractor(void) { return fm_extractor_py_init(); }
