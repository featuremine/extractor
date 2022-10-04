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
 * @file book_play_ext.c
 * @date 23 Aug 2021
 * @brief Book play split extension sample
 *
 * @see http://www.featuremine.com
 */

#include "extractor/comp_sys.h"
#include "book_play_split.h"
/**
 * Computation definition
 * It must provide the following information to describe a computation:
 * name: used by the computing system to identify the computation, this name
 * MUST be unique. generate: reference to the operator's generator function
 * destroy: reference to the operator's destruction function
 * closure: pointer to the closure that is shared between all operator instances
 */
fm_comp_def_t book_play_split_comp_def = {
    "book_play_split_custom",         // name
    &fm_comp_book_play_split_gen,     // generate
    &fm_comp_book_play_split_destroy, // destroy
    NULL                              // closure
};

/**
 * Registers the operator definition in the computational system.
 * The name of this function MUST start with FmInit_ to allow the system
 * extension loader to find the function in external modules.
 *
 * @param sys computing system
 */
void FmInit_book_play_split(fm_comp_sys_t *sys) {
  fm_comp_type_add(sys, &book_play_split_comp_def);
}
