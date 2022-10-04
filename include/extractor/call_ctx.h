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
 * @file call_ctx.h
 * @author Maxim Trokhimtchouk
 * @date 2 Aug 2017
 * @brief File contains C declaration of the call context
 *
 * This file contains declarations of the call context
 * @see http://www.featuremine.com
 */

#ifndef __FM_CALL_CTX_H__
#define __FM_CALL_CTX_H__

#include "call_ctx_base.h"
#include "exec_ctx.h"
#include "frame.h"
#include "handle.h"
#include <fmc/platform.h>

/**
 * @brief type definition of the context provided by computation generator
 */
typedef const void *fm_comp_ctx_p;

/**
 * @brief pointer to a generic execution context
 */
typedef fm_exec_ctx_t *fm_exec_ctx_p;

typedef void *fm_call_exec_cl;

/**
 * @brief defines context structure for a call
 */
typedef struct fm_call_ctx {
  fm_comp_ctx_p comp;
  fm_exec_ctx_p exec;
  fm_call_handle_t handle;
  size_t depc;
  const fm_call_handle_t *deps;
} fm_call_ctx_t;

/**
 * @brief pointer to a queuing callback
 */
typedef void (*fm_call_queuer_p)(size_t, fm_call_ctx_t *);

/**
 * @brief typedef to a call function pointer
 *
 * The actual function used to execute the operation. It is provided
 * with a call context and an array of data frame objects for input.
 * It should return true if result has changed.
 * @return bool indication of change of result
 */
typedef bool (*fm_call_exec_p)(fm_frame_t *, size_t,
                               const fm_frame_t *const argv[], fm_call_ctx_t *,
                               fm_call_exec_cl);

typedef struct fm_comp_clbck {
  fm_frame_clbck_p clbck;
  fm_frame_clbck_cl cl;
} fm_comp_clbck_t;

typedef fm_comp_clbck_t *fm_comp_clbck_it;

#endif // __FM_CALL_CTX_H__
