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
 * @file exec_ctx.h
 * @author Maxim Trokhimtchouk
 * @date 15 Sept 2017
 * @brief File contains C declaration of the execution context
 *
 * This file contains declarations of the execution context
 * @see http://www.featuremine.com
 */

#ifndef __FM_EXEC_CTX_H__
#define __FM_EXEC_CTX_H__

#include "frame.h"
#include <fmc/platform.h>

/**
 * @brief defines execution context object
 * Execution context object is responsible for execution of
 * the compute graph. A specific execution context is first created
 * from a given compute graph. Each execution context is derived from
 * a base that provides common functionality.
 */
typedef struct fm_exec_ctx fm_exec_ctx_t;

FMMODFUNC bool fm_exec_ctx_is_error(fm_exec_ctx_t *ctx);
FMMODFUNC const char *fm_exec_ctx_error_msg(fm_exec_ctx_t *ctx);
FMMODFUNC void fm_exec_ctx_error_set(fm_exec_ctx_t *ctx, const char *msg, ...);
FMMODFUNC void fm_exec_ctx_error_clear(fm_exec_ctx_t *ctx);
FMMODFUNC fm_frame_alloc_t *fm_exec_ctx_frames(fm_exec_ctx_t *ctx);

#endif // __FM_EXEC_CTX_H__
