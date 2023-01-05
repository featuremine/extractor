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
 * @file book.h
 * @author Andres Rangel
 * @date 2 Mar 2020
 * @brief File contains C definitions of the python book interface
 */

#pragma once

#include "extractor/book/book.h"
#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BookStruct Book;

FMMODFUNC bool PyBook_Check(PyObject *);

FMMODFUNC fm_book_shared_t *PyBook_SharedBook(PyObject *obj);

FMMODFUNC bool PyBook_AddTypes(PyObject *);

#ifdef __cplusplus
}
#endif
