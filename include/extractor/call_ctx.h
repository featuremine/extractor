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

#pragma once

#include "call_ctx_base.h"
#include "extractor/exec_ctx.h"
#include "extractor/frame.h"
#include "extractor/handle.h"
#include "extractor/api.h"
#include "fmc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief typedef to a call function pointer
 *
 * The actual function used to execute the operation. It is provided
 * with a call context and an array of data frame objects for input.
 * It should return true if result has changed.
 * @return bool indication of change of result
 */
typedef struct fm_comp_clbck {
  fm_frame_clbck_p clbck;
  fm_frame_clbck_cl cl;
} fm_comp_clbck_t;

typedef fm_comp_clbck_t *fm_comp_clbck_it;

#ifdef __cplusplus
}
#endif
