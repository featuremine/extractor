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
 * @brief File contains private C declaration of the stream execution context
 *
 * This file contains private declarations of the stream execution context
 * @see http://www.featuremine.com
 */

#pragma once

#include "call_stack.h"
#include "comp_graph.h"
#include "extractor/handle.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief
 *
 * Returns a new stream execution context.
 * @note Assumes the graph is
 * topologically sorted with necessary optimizations for stack
 * order already in place.
 */
fm_stream_ctx_t *fm_stream_ctx_new(fm_comp_graph_t *g);

/**
 * @brief deletes the stream context object
 */
void fm_stream_ctx_del(fm_stream_ctx_t *);

/**
 * @brief get the call queue object
 */
fm_call_queue_t *fm_stream_ctx_get_queue(fm_stream_ctx_t *ctx);
