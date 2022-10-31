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
 * @file book_msg.h
 * @author Andres Rangel
 * @date 10 Jan 2019
 * @brief File contains C definitions of the book message operator
 *
 * This file contains declarations of the book message operator, which
 * take book updates as input and returns a frame with the desired the
 * book updates of the desired type.
 *
 * @see http://www.featuremine.com
 */

#ifndef __FM_book_msg_H__
#define __FM_book_msg_H__

#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"

fm_ctx_def_t *fm_comp_book_msg_gen(fm_comp_sys_t *sys, fm_comp_def_cl, unsigned,
                                   fm_type_decl_cp[], fm_type_decl_cp,
                                   fm_arg_stack_t);

void fm_comp_book_msg_destroy(fm_comp_def_cl, fm_ctx_def_t *);

const fm_comp_def_t fm_comp_book_msg = {"book_msg", &fm_comp_book_msg_gen,
                                        &fm_comp_book_msg_destroy, NULL};

fm_ctx_def_t *fm_comp_book_header_gen(fm_comp_sys_t *sys, fm_comp_def_cl,
                                      unsigned, fm_type_decl_cp[],
                                      fm_type_decl_cp, fm_arg_stack_t);

void fm_comp_book_header_destroy(fm_comp_def_cl, fm_ctx_def_t *);

const fm_comp_def_t fm_comp_book_header = {"book_header",
                                           &fm_comp_book_header_gen,
                                           &fm_comp_book_header_destroy, NULL};

#endif // __FM_book_msg_H__
