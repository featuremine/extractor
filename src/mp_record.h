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
 * @file mp_record.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the MsgPack record object
 *
 * This file contains declarations of the MsgPack record object
 * @see http://www.featuremine.com
 */

#ifndef __FM_MP_RECORD_H__
#define __FM_MP_RECORD_H__

#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"

fm_ctx_def_t *fm_comp_mp_record_gen(fm_comp_sys_t *sys, fm_comp_def_cl,
                                    unsigned, fm_type_decl_cp[],
                                    fm_type_decl_cp, fm_arg_stack_t);

void fm_comp_mp_record_destroy(fm_comp_def_cl, fm_ctx_def_t *);

const fm_comp_def_t fm_comp_mp_record = {"mp_record", &fm_comp_mp_record_gen,
                                         &fm_comp_mp_record_destroy, NULL};

#endif // __FM_MP_RECORD_H__
