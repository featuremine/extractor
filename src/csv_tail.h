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
 * @file csv_tail.h
 * @author Andrus Suvalau
 * @date 17 Mar 2020
 * @brief File contains C definitions of the csv_tail operator
 *
 * The csv_tail operator is designed to tail csv files in
 * real live mode.
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"

fm_ctx_def_t *fm_comp_csv_tail_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
                                   fm_type_decl_cp ptype, fm_arg_stack_t plist);

void fm_comp_csv_tail_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def);

const fm_comp_def_t fm_comp_csv_tail = {"csv_tail", &fm_comp_csv_tail_gen,
                                        &fm_comp_csv_tail_destroy, NULL};
