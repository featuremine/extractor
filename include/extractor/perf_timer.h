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
 * @file perf_timer.h
 * @author Andres Rangel
 * @date 16 Jan 2019
 * @brief File contains C definitions of the performance timer objects
 *
 * This file contains declarations of the performance timer objects
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "fmc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

FMMODFUNC fm_ctx_def_t *fm_comp_perf_timer_start_gen(fm_comp_sys_t *sys,
                                                     fm_comp_def_cl, unsigned,
                                                     fm_type_decl_cp[],
                                                     fm_type_decl_cp,
                                                     fm_arg_stack_t);

FMMODFUNC void fm_comp_perf_timer_start_destroy(fm_comp_def_cl, fm_ctx_def_t *);

FMMODFUNC fm_ctx_def_t *
fm_comp_perf_timer_stop_gen(fm_comp_sys_t *sys, fm_comp_def_cl, unsigned,
                            fm_type_decl_cp[], fm_type_decl_cp, fm_arg_stack_t);

FMMODFUNC void fm_comp_perf_timer_stop_destroy(fm_comp_def_cl, fm_ctx_def_t *);

FMMODFUNC bool fm_comp_perf_timer_add(fm_comp_sys_t *sys, void *samples);

#ifdef __cplusplus
}
#endif
