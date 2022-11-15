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
 * @file book_trades.h
 * @author Andres Rangel
 * @date 10 Jan 2019
 * @brief File contains C definitions of the book trades operator
 *
 * This file contains declarations of of the book trades operator, which
 * take book updates as input and returns a frame that is updated with
 * all the trade book updates, such as trades or executions.
 */

#pragma once

#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"

#ifdef __cplusplus
extern "C" {
#endif

fm_ctx_def_t *fm_comp_book_trades_gen(fm_comp_sys_t *sys, fm_comp_def_cl,
                                      unsigned, fm_type_decl_cp[],
                                      fm_type_decl_cp, fm_arg_stack_t);

void fm_comp_book_trades_destroy(fm_comp_def_cl, fm_ctx_def_t *);

const fm_comp_def_t fm_comp_book_trades = {"book_trades",
                                           &fm_comp_book_trades_gen,
                                           &fm_comp_book_trades_destroy, NULL};
