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

#ifndef __FM_TYPE_DECL_H__
#define __FM_TYPE_DECL_H__

#include "fmc/platform.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "extractor/decimal64.h"
#include "extractor/rational64.h"
#include "fmc/decimal128.h"
#include "fmc/time.h"

/**
 * @brief type declaration object
 *
 * object used to store data relevant to type declaration.
 */
typedef const struct fm_type_decl *fm_type_decl_cp;

/**
 * @brief enum for base types
 *
 * Defines based types for type declaration
 */

// NOTE: Move these definitions to the common library
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef float FLOAT32;
typedef double FLOAT64;
typedef fm_rational64_t RATIONAL64;
typedef fm_decimal64_t DECIMAL64;
typedef fmc_decimal128_t DECIMAL128;
typedef fmc_time64_t TIME64;
typedef char CHAR;
typedef wchar_t WCHAR;
#ifndef FM_SYS_WIN
typedef bool BOOL;
#else
#include <windows.h>
#endif

typedef enum {
  FM_TYPE_INT8 = 0,
  FM_TYPE_INT16,
  FM_TYPE_INT32,
  FM_TYPE_INT64,
  FM_TYPE_UINT8,
  FM_TYPE_UINT16,
  FM_TYPE_UINT32,
  FM_TYPE_UINT64,
  FM_TYPE_FLOAT32,
  FM_TYPE_FLOAT64,
  FM_TYPE_RATIONAL64,
  FM_TYPE_DECIMAL64,
  FM_TYPE_DECIMAL128,
  FM_TYPE_TIME64,
  FM_TYPE_CHAR,
  FM_TYPE_WCHAR,
  FM_TYPE_BOOL,
  FM_TYPE_LAST
} FM_BASE_TYPE;

typedef const char *(*fm_base_type_parser)(const char *, const char *, void *,
                                           const char *);

typedef bool (*fm_base_type_fwriter)(FILE *, const void *, const char *);

FMMODFUNC fm_base_type_parser fm_base_type_parser_get(FM_BASE_TYPE t);

FMMODFUNC fm_base_type_fwriter fm_base_type_fwriter_get(FM_BASE_TYPE t);

FMMODFUNC size_t fm_base_type_sizeof(FM_BASE_TYPE t);

FMMODFUNC const char *fm_base_type_name(FM_BASE_TYPE t);

#endif /* __FM_TYPE_DECL_H__ */
