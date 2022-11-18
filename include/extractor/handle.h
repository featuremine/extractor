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

#include "fmc/platform.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief execution point object
 *
 * Execution point object represents a reference to call stack item
 */
typedef size_t fm_call_handle_t;

#ifdef __cplusplus
}
#endif
