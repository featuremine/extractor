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
 * @file comp_base.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the base of the compute object
 *
 * This file contains declarations of the comp context
 * @see http://www.featuremine.com
 */

#pragma once

#include "call_obj.h"
#include "extractor/call_ctx.h"
#include "extractor/frame_base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fm_comp fm_comp_t;

/**
 * @brief
 */
fm_call_obj_t *fm_stream_call_obj_new(fm_comp_t *comp, fm_exec_ctx_p ctx,
                                      unsigned argc);

/**
 * @brief
 */
bool fm_comp_inplace(const fm_comp_t *obj);

/**
 * @brief
 */
bool fm_comp_volatile(const fm_comp_t *obj);

/**
 * @brief
 */
bool fm_comp_data_required(const fm_comp_t *obj);

/**
 * @brief
 */
fm_frame_t *fm_comp_frame_mk(const fm_comp_t *obj, fm_frame_alloc_t *alloc);

/**
 * @brief
 */
bool fm_comp_call_init(fm_comp_t *obj, fm_call_obj_t *);

/**
 * @brief
 */
void fm_comp_call_destroy(fm_comp_t *);

/**
 * @brief
 */
const char *fm_comp_type(const struct fm_comp *obj);

#ifdef __cplusplus
}
#endif
