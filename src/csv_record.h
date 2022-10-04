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
 * @file csv_record.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the CSV record object
 *
 * This file contains declarations of the CSV record object
 * @see http://www.featuremine.com
 */

#ifndef __FM_CSV_RECORD_H__
#define __FM_CSV_RECORD_H__

#include "arg_stack.h"
#include "comp_def.h"

#ifdef __cplusplus
extern "C" {
#endif

FMMODFUNC fm_ctx_def_t *fm_comp_csv_record_gen(fm_comp_sys_t *sys,
                                               fm_comp_def_cl, unsigned,
                                               fm_type_decl_cp[],
                                               fm_type_decl_cp, fm_arg_stack_t);

FMMODFUNC void fm_comp_csv_record_destroy(fm_comp_def_cl, fm_ctx_def_t *);

const fm_comp_def_t fm_comp_csv_record = {"csv_record", &fm_comp_csv_record_gen,
                                          &fm_comp_csv_record_destroy, NULL};

#ifdef __cplusplus
}
#endif

#endif // __FM_CSV_RECORD_H__
