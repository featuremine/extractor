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
 * @file arg_stack.h
 * @author Maxim Trokhimtchouk
 * @date 8 Nov 2017
 * @brief File contains C declaration of the argument stack object
 *
 * This file contains declarations of the argument stack object
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "extractor/api.h"
#include "fmc/alignment.h"
#include "fmc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STACK_ARGS(stack)                                                      \
  (fm_arg_stack_t{{stack.header.size, &(stack).buffer[0] + stack.header.size}})

#define STACK_FWD(stack) ((fm_arg_stack_t *)&(stack))

#define STACK(SIZE, var)                                                       \
  struct {                                                                     \
    struct {                                                                   \
      size_t size;                                                             \
      char *cursor;                                                            \
    } header;                                                                  \
    char buffer[SIZE];                                                         \
  } var = {{SIZE, &var.buffer[0] + SIZE}}

FMMODFUNC fm_arg_stack_t *fm_arg_stack_alloc(size_t tsize);
FMMODFUNC fm_arg_stack_t *fm_arg_stack_copy(fm_arg_stack_t *ptr);
FMMODFUNC fm_arg_stack_t *fm_arg_stack_realloc(fm_arg_stack_t *ptr,
                                               size_t tsize);
FMMODFUNC bool fm_arg_stack_double(fm_arg_stack_t **ptr);
FMMODFUNC fm_arg_stack_t fm_arg_stack_args(fm_arg_stack_t *stack);
FMMODFUNC void fm_arg_stack_free(fm_arg_stack_t *ptr);

#ifdef __cplusplus
}
#endif
