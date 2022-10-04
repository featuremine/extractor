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
 * @file operator_def.h
 * @author Leandro Leon
 * @date 22 Aug 2018
 * @brief File contains macros for operator definitions
 *
 * File contains macros for general purpose operator definitions
 * @see http://www.featuremine.com
 */

#ifndef __OPERATOR_DEF_H__
#define __OPERATOR_DEF_H__

#include "arg_stack.h"
#include "comp_def.h"

#define FM_DEFINE_COMP_OP(op_name)                                             \
  fm_ctx_def_t *fm_comp_##op_name##_gen(fm_comp_sys_t *sys, fm_comp_def_cl,    \
                                        unsigned, fm_type_decl_cp[],           \
                                        fm_type_decl_cp, fm_arg_stack_t);      \
                                                                               \
  void fm_comp_##op_name##_destroy(fm_comp_def_cl, fm_ctx_def_t *);            \
                                                                               \
  const fm_comp_def_t fm_comp_##op_name = {#op_name, &fm_comp_##op_name##_gen, \
                                           &fm_comp_##op_name##_destroy, NULL}

#endif
