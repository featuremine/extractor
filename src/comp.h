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
 * @file comp.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the comp object
 *
 * This file contains declarations of the comp context
 * @see http://www.featuremine.com
 */

#pragma once

#include "arg_serial.h"
#include "comp_base.h"
#include "extractor/call_ctx.h"
#include "extractor/comp_def.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fm_result_ref fm_result_ref_t;

typedef struct fm_comp_node fm_comp_node_t;

/**
 * @brief
 */
FMMODFUNC fm_comp_t *fm_comp_new(const fm_comp_def_t *, fm_ctx_def_t *,
                                 const char *);

/**
 * @brief
 */
FMMODFUNC void fm_comp_del(fm_comp_t *);

/**
 * @brief
 */
FMMODFUNC fm_comp_node_t *fm_comp_node_ptr(fm_comp_t *);

/**
 * @brief
 */
FMMODFUNC void fm_comp_node_ptr_set(fm_comp_t *, fm_comp_node_t *);

/**
 * @brief
 */
FMMODFUNC fm_type_decl_cp fm_comp_result_type(const fm_comp_t *);

/**
 * @brief
 */
FMMODFUNC void fm_comp_set_args(fm_comp_t *, fm_type_decl_cp, fm_arg_stack_t);

/**
 * @brief
 */
FMMODFUNC const fm_arg_buffer_t *fm_comp_arg_buffer(const struct fm_comp *obj);

/**
 * @brief
 */
FMMODFUNC const fm_comp_def_t *fm_comp_get_def(const fm_comp_t *comp);

/**
 * @brief
 */
FMMODFUNC const fm_ctx_def_t *fm_comp_ctx_def(const fm_comp_t *obj);

/**
 * @brief
 */
FMMODFUNC const fm_comp_node_t *fm_comp_node_cptr(const fm_comp_t *obj);

FMMODFUNC fm_call_obj_t *fm_comp_call(const fm_comp_t *obj);

FMMODFUNC bool fm_comp_clbck_has(const fm_comp_t *obj);

FMMODFUNC fm_comp_clbck_it fm_comp_clbck_begin(fm_comp_t *obj);

FMMODFUNC fm_comp_clbck_it fm_comp_clbck_end(fm_comp_t *obj);

FMMODFUNC void fm_comp_result_set(fm_result_ref_t *ref, fm_frame_t *frame);

#ifdef __cplusplus
}
#endif
