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
 * @file nan.h
 * @author Andres Rangel
 * @date 2 Oct 2018
 * @brief File contains C definitions of the map nan object
 *
 * This file contains declarations of the map nan object
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"

#ifdef __cplusplus
extern "C" {
#endif

fm_ctx_def_t *fm_comp_nan_gen(fm_comp_sys_t *sys, fm_comp_def_cl, unsigned,
                              fm_type_decl_cp[], fm_type_decl_cp,
                              fm_arg_stack_t);

const fm_comp_def_t fm_comp_nan = {"nan", &fm_comp_nan_gen, NULL, NULL};
