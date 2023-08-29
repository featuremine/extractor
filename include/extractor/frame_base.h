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
 * @file frame.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C declaration of the frame object
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include "call_ctx_base.h"
#include "extractor/api.h"
#include "extractor/type_decl.h"
#include "fmc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fm_frame fm_frame_t;

typedef struct fm_frame_alloc fm_frame_alloc_t;

FMMODFUNC void fm_frame_clone_copy(fm_frame_t *dest, const fm_frame_t *src);

FMMODFUNC fm_frame_alloc_t *fm_frame_alloc_new();

FMMODFUNC void fm_frame_alloc_del(fm_frame_alloc_t *obj);

FMMODFUNC fm_frame_t *fm_frame_alloc_clone(fm_frame_alloc_t *alloc,
                                           const fm_frame_t *src);

typedef void *fm_frame_clbck_cl;

/**
 * @brief type definition for operation call back
 *
 * The function is called by the stack with the result of the operation
 * if the operation has modified the result.
 */
typedef void (*fm_frame_clbck_p)(const fm_frame_t *, fm_frame_clbck_cl,
                                 fm_call_ctx_t *);

#ifdef __cplusplus
}
#endif
