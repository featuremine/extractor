/******************************************************************************

        COPYRIGHT (c) 2023 by Featuremine Corporation.
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
 * @file skip_unless.h
 * @authors Ivan Gonzalez
 * @date 7 Mar 2023
 * @brief File contains C definitions of the "skip_unless" logical operator
 * object
 *
 * This file contains declarations of the "skip_unless" logical operator object
 * @see http://www.featuremine.com
 */

#ifndef __FM_SKIP_UNLESS_H__
#define __FM_SKIP_UNLESS_H__

#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"

#ifdef __cplusplus
extern "C" {
#endif

fm_ctx_def_t *fm_comp_skip_unless_gen(fm_comp_sys_t *sys, fm_comp_def_cl,
                                      unsigned, fm_type_decl_cp[],
                                      fm_type_decl_cp, fm_arg_stack_t);

void fm_comp_skip_unless_destroy(fm_comp_def_cl, fm_ctx_def_t *);

const fm_comp_def_t fm_comp_skip_unless = {"skip_unless",
                                           &fm_comp_skip_unless_gen,
                                           &fm_comp_skip_unless_destroy, NULL};

#ifdef __cplusplus
}
#endif

#endif // __FM_SKIP_UNLESS_H__