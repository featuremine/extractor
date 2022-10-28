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
 * @file comp_sys_capture.h
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C declaration of the computational system capture
 *
 * This file contains declarations of the computational system capture.
 * @see http://www.featuremine.com
 */

#ifndef __FM_COMP_SYS_CAPTURE_H__
#define __FM_COMP_SYS_CAPTURE_H__

#include "extractor/comp_sys.h"
#include "extractor/serial.h"
#include "fmc/platform.h"

/**
 * @brief Record enabled context
 * Returns context that records required information to replay
 */

FMMODFUNC fm_stream_ctx_t *fm_stream_ctx_recorded(fm_comp_sys_t *,
                                                  fm_comp_graph_t *,
                                                  fm_writer w, void *cl);

/**
 * @brief Replay enabled context
 * Returns context that replays what was recorded in a record enabled context.
 */
FMMODFUNC fm_stream_ctx_t *fm_stream_ctx_replayed(fm_comp_sys_t *,
                                                  fm_comp_graph_t *,
                                                  fm_reader w, void *cl);
#endif //__FM_COMP_SYS_CAPTURE_H__
