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
 * @file stream_ctx.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C declaration of the stream execution context
 *
 * This file contains declarations of the stream execution context
 * @see http://www.featuremine.com
 */

#ifndef __FM_STREAM_CTX_H__
#define __FM_STREAM_CTX_H__

#include "extractor/handle.h"
#include "extractor/time64.h"
#include <fmc/platform.h>

/**
 * @brief defines stream execution context object
 * Execution context object is responsible for execution of
 * the compute graph. A specific execution context is first created
 * from a given compute graph. In the process it initializes
 * various operations and creates a call stack for efficient
 * computation of the compute graph. It is also used to communicate
 * the operation call the current execution context of the system,
 * such as the query range or the current time of execution. It
 * also provides ability to access event loop methods in the case
 * of stream execution context.
 */
typedef struct fm_stream_ctx fm_stream_ctx_t;

typedef void *fm_ctx_cl;

typedef void (*fm_ctx_clbck_p)(fm_stream_ctx_t *, fm_ctx_cl);

/**
 * @brief queues a given call to be processed
 */
FMMODFUNC void fm_stream_ctx_queue(fm_stream_ctx_t *ctx,
                                   fm_call_handle_t handle);

/**
 * @brief schedules a given call for processing
 */
FMMODFUNC void fm_stream_ctx_schedule(fm_stream_ctx_t *ctx,
                                      fm_call_handle_t handle,
                                      fm_time64_t time);

/**
 * @brief schedules a given call for processing
 */
FMMODFUNC bool fm_stream_ctx_scheduled(fm_stream_ctx_t *ctx);

/**
 * @brief returns true if there are queued functions
 */
FMMODFUNC bool fm_stream_ctx_queued(fm_stream_ctx_t *ctx);

/**
 * @brief returns true is there is nothing else to do
 */
FMMODFUNC bool fm_stream_ctx_idle(fm_stream_ctx_t *ctx);

/**
 * @brief processes the streaming context onces
 */
FMMODFUNC bool fm_stream_ctx_proc_one(fm_stream_ctx_t *ctx, fm_time64_t now);

/**
 * @brief returns the next time for which there is a scheduled event
 */
FMMODFUNC fm_time64_t fm_stream_ctx_next_time(fm_stream_ctx_t *ctx);

/**
 * @brief returns the now for the stream execution context
 */
FMMODFUNC fm_time64_t fm_stream_ctx_now(fm_stream_ctx_t *ctx);

/**
 * @brief runs the context to end time
 *
 * @return returns true if the run is successful
 */
FMMODFUNC bool fm_stream_ctx_run_to(fm_stream_ctx_t *ctx, fm_time64_t e);

/**
 * @brief runs the context from a zero time to the end of time
 *
 * @return returns true if the run is successful
 */
FMMODFUNC bool fm_stream_ctx_run(fm_stream_ctx_t *ctx);

/**
 * @brief runs the context using live system time
 *
 * @return returns true if the run is successful
 */
FMMODFUNC bool fm_stream_ctx_run_live(fm_stream_ctx_t *ctx);

/**
 * @brief sets the pre-processing callback for the execution context
 *
 * @return returns true if callback was set successfully
 */
FMMODFUNC void fm_stream_ctx_preproc_clbck_set(fm_stream_ctx_t *ctx,
                                               fm_ctx_clbck_p f, fm_ctx_cl cl);

/**
 * @brief sets the post-processing callback for the execution context
 *
 * @return returns true if callback was set successfully
 */
FMMODFUNC void fm_stream_ctx_postproc_clbck_set(fm_stream_ctx_t *ctx,
                                                fm_ctx_clbck_p f, fm_ctx_cl cl);

#endif // __FM_STREAM_CTX_H__
