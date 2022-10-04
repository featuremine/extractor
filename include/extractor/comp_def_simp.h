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
 * @file comp_def_simp.h
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C declaration of the simplified computation interface.
 *
 * This file contains declarations of the simplified computation interface.
 * @see http://www.featuremine.com
 */

#ifndef __FM_COMP_DEF_SIMP_H__
#define __FM_COMP_DEF_SIMP_H__

#include "extractor/arg_stack.h"
#include "extractor/call_ctx.h"
#include "extractor/comp_def.h"
#include "extractor/time64.h"
#include "fmc/platform.h"

typedef struct fm_comp_def_simp {
  bool inplace;
  fm_type_decl_cp (*type)(fm_type_sys_t *, unsigned, fm_type_decl_cp[]);
  fm_call_exec_p stream_exec;
  fm_call_exec_p query_exec;
  fm_call_init_p init;
  fm_call_destroy_p destroy;
  fm_call_range_p range;
  bool (*new_cl)(fm_ctx_def_cl *, fm_type_sys_t *, fm_type_decl_cp,
                 fm_arg_stack_t);
  void (*del_cl)(fm_ctx_def_cl);
} fm_comp_def_simp_t;

#define FM_COMP_DEF_SIMP(VAR, NAME, S1, S2, S3, S4, S5, S6, S7, S8, S9)        \
  const struct fm_comp_def_simp VAR##_simp_def_##__LINE__ = {                  \
      S1, S2, S3, S4, S5, S6, S7, S8, S9};                                     \
  fm_comp_def_t VAR = {NAME, fm_comp_def_simp_generate,                        \
                       fm_comp_def_simp_destroy,                               \
                       (fm_comp_def_cl)&VAR##_simp_def_##__LINE__}

FMMODFUNC fm_ctx_def_t *
fm_comp_def_simp_generate(fm_comp_sys_t *sys, fm_comp_def_cl, unsigned,
                          fm_type_decl_cp[], fm_type_decl_cp, fm_arg_stack_t);

FMMODFUNC void fm_comp_def_simp_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def);

#endif // __FM_COMP_DEF_SIMP_H__
