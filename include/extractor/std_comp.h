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
 * @file std_comp.h
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C declaration of the computational system
 *
 * This file contains declarations of the computational system.
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/frame_base.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_decl.h"
#include "fmc/platform.h"
#include "fmc/time.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief loads standard computations
 */
FMMODFUNC bool fm_comp_sys_std_comp(fm_comp_sys_t *);

#ifdef __cplusplus
}
#endif
