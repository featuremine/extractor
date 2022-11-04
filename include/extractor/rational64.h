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
 * @file rational.h
 * @author Maxim Trokhimtchouk
 * @date 28 Dec 2018
 * @brief File contains C definitions of the rational object
 *
 * This file contains declarations of the rational object
 * @see http://www.featuremine.com
 */

#ifndef __FM_RATIONAL64_H__
#define __FM_RATIONAL64_H__

#include <math.h>
#include <stddef.h>
#include <stdint.h>

#include "fmc/rprice.h"
#include "fmc/platform.h"

typedef struct {
  int32_t num;
  int32_t den;
} fm_rational64_t;

FMMODFUNC fm_rational64_t fm_rational64_zero();
FMMODFUNC fm_rational64_t fm_rational64_new(int32_t num, int32_t den);
FMMODFUNC fm_rational64_t fm_rational64_new2(int64_t num, int64_t den);
FMMODFUNC fm_rational64_t fm_rational64_from_double(double value, int32_t base);
FMMODFUNC fm_rational64_t fm_rational64_from_int(int value);
FMMODFUNC fm_rational64_t fm_rational64_from_rprice(fmc_rprice_t t);
FMMODFUNC double fm_rational64_to_double(fm_rational64_t t);
FMMODFUNC fmc_rprice_t fm_rational64_to_rprice(fm_rational64_t t);
FMMODFUNC fm_rational64_t fm_rational64_div(fm_rational64_t a,
                                            fm_rational64_t b);
FMMODFUNC fm_rational64_t fm_rational64_add(fm_rational64_t a,
                                            fm_rational64_t b);
FMMODFUNC bool fm_rational64_less(fm_rational64_t a, fm_rational64_t b);
FMMODFUNC bool fm_rational64_greater(fm_rational64_t a, fm_rational64_t b);
FMMODFUNC bool fm_rational64_equal(fm_rational64_t a, fm_rational64_t b);
FMMODFUNC bool fm_rational64_notequal(fm_rational64_t a, fm_rational64_t b);
FMMODFUNC fm_rational64_t fm_rational64_sub(fm_rational64_t a,
                                            fm_rational64_t b);
FMMODFUNC fm_rational64_t fm_rational64_mul(fm_rational64_t a,
                                            fm_rational64_t b);
FMMODFUNC fm_rational64_t fm_rational64_inf();
FMMODFUNC fm_rational64_t fm_rational64_nan();
FMMODFUNC bool fm_rational64_isnan(fm_rational64_t a);
FMMODFUNC bool fm_rational64_isinf(fm_rational64_t a);

#endif // __FM_RATIONAL64_H__
