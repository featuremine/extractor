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
 * @file fmtron_sys.h
 * @author Andrus Suvalau
 * @date 13 Apr 2020
 * @brief File contains C declarions of the fmtron operator
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include "arg_stack.h"
#include "comp_def.h"

fm_ctx_def_t *fm_comp_fmtron_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                 unsigned argc, fm_type_decl_cp argv[],
                                 fm_type_decl_cp ptype, fm_arg_stack_t plist);

void fm_comp_fmtron_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def);

extern const fm_comp_def_t fm_comp_fmtron;
