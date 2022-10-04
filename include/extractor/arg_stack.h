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

#ifndef __FM_ARG_STACK_H__
#define __FM_ARG_STACK_H__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <fmc/alignment.h>
#include <fmc/platform.h>

typedef struct {
  size_t size;
  char *cursor;
} fm_arg_stack_header_t;

typedef struct {
  fm_arg_stack_header_t header;
  char buffer[1];
} fm_arg_stack_t;

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

#define STACK_OFF(stack, what)                                                 \
  (char *)((size_t)(stack).header.cursor &                                     \
           (~((FMC_WORDSIZE - 1) & (sizeof(what) - 1))))

#define STACK_CHECK(stack, what)                                               \
  (STACK_OFF((stack), what) >= &(stack).buffer[0] + sizeof(what))

#define STACK_PUSH(stack, what)                                                \
  ((stack).header.cursor = STACK_OFF((stack), (what)) - sizeof(what),          \
   memcpy((stack).header.cursor, (void *)&(what), sizeof(what)))

#define STACK_SAFE_PUSH(stack, what)                                           \
  (STACK_CHECK((stack), (what)) ? (STACK_PUSH((stack), (what)), true) : false)

#define HEAP_STACK_PUSH(stack, what)                                           \
  (STACK_CHECK((*stack), (what))                                               \
       ? (STACK_PUSH((*stack), (what)), true)                                  \
       : (fm_arg_stack_double(&stack) ? STACK_SAFE_PUSH((*stack), (what))      \
                                      : false))

#define STACK_POP(stack, what)                                                 \
  ((stack).header.cursor = STACK_OFF((stack), what) - sizeof(what),            \
   *(what *)(stack).header.cursor)

#endif /* __FM_ARG_STACK_H__ */
