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
 * @date 2 Aug 2017
 * @brief File contains C declaration of the execution context
 *
 * This file contains declarations of the execution context
 * @see http://www.featuremine.com
 */

#ifndef __FM_QUERY_CTX_H__
#define __FM_QUERY_CTX_H__

#include <fmc/platform.h>

/**
 * @brief defines execution context object
 * Execution context object is used to communicate the operation call
 * the current execution context of the system, such as the query range
 * or the current time of execution. It also provides ability to access
 * event loop methods in the case of stream execution context.
 */

enum FM_EXEC_CONTEXT_TYPE {
  FM_EXEC_CONTEXT_STREAM,
  FM_EXEC_CONTEXT_QUERY,
};

typedef stuct fm_exec_ctx fm_exec_ctx_t;

/**
 * @brief return execution context type
 */
FMMODFUNC FM_EXEC_CONTEXT_TYPE fm_exec_ctx_type(fm_exec_ctx_t *ctx);

FMMODFUNC bool fm_ctx_proc_one(fm_exec_ctx_t *ctx);

FMMODFUNC time_t fm_ctx_next_time(fm_exec_ctx_t *ctx);

FMMODFUNC bool fm_ctx_query(fm_exec_ctx_t *ctx, time_t t1, time_t t2);

#endif // __FM_QUERY_CTX_H__
