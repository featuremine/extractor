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
 * @file frame_ytp_decode.h
 * @author Federico Ravchina
 * @date 7 Jan 2022
 * @brief File contains C definitions of the frame decode object
 *
 * This file contains declarations of the ytp record object
 * @see http://www.featuremine.com
 */

#pragma once

#include "arg_stack.h"
#include "comp_def.h"

#ifdef __cplusplus
extern "C" {
#endif

FMMODFUNC fm_ctx_def_t *fm_comp_frame_ytp_decode_gen(fm_comp_sys_t *sys,
                                                     fm_comp_def_cl, unsigned,
                                                     fm_type_decl_cp[],
                                                     fm_type_decl_cp,
                                                     fm_arg_stack_t);

FMMODFUNC void fm_comp_frame_ytp_decode_destroy(fm_comp_def_cl, fm_ctx_def_t *);

const fm_comp_def_t fm_comp_frame_ytp_decode = {
    "frame_ytp_decode", &fm_comp_frame_ytp_decode_gen,
    &fm_comp_frame_ytp_decode_destroy, NULL};

#ifdef __cplusplus
}
#endif
