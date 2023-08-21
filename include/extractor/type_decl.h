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
 * @file type_decl.h
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C declaration of the type declaration object
 *
 * This file contains declarations of the type declaration object
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

#pragma once

#include "fmc/platform.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "fmc/decimal128.h"
#include "fmc/rational64.h"
#include "fmc/rprice.h"
#include "fmc/time.h"

#include "extractor/api.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief type declaration object
 *
 * object used to store data relevant to type declaration.
 */
typedef const struct fm_type_decl *fm_type_decl_cp;

typedef const char *(*fm_base_type_parser)(const char *, const char *, void *,
                                           const char *);

typedef bool (*fm_base_type_fwriter)(FILE *, const void *, const char *);

FMMODFUNC fm_base_type_parser fm_base_type_parser_get(FM_BASE_TYPE t);

FMMODFUNC fm_base_type_fwriter fm_base_type_fwriter_get(FM_BASE_TYPE t);

FMMODFUNC size_t fm_base_type_sizeof(FM_BASE_TYPE t);

FMMODFUNC const char *fm_base_type_name(FM_BASE_TYPE t);

#ifdef __cplusplus
}
#endif
