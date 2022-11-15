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
 * @file split_by.h
 * @author Andres Rangel
 * @date 23 Jan 2019
 * @brief File contains C definitions of the split_by object
 *
 * This file contains declarations of the split_by object
 * @see http://www.featuremine.com
 */

#pragma once

#include "operator_def.h"

#ifdef __cplusplus
extern "C" {
#endif

bool fm_comp_split_by_add(fm_comp_sys_t *sys);
