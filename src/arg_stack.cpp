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
 * @file arg_stack.cpp
 * @author Maxim Trokhimtchouk
 * @date 8 Nov 2017
 * @brief File contains C definitions of the argument stack object
 *
 * This file contains definitions of the argument stack object
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/arg_stack.h"
#include <fmc/alignment.h>
}

fm_arg_stack_t *fm_arg_stack_alloc(size_t tsize) {
  if (tsize <= sizeof(fm_arg_stack_header_t))
    return nullptr;
  size_t size = tsize - sizeof(fm_arg_stack_header_t);
  fm_arg_stack_t *ptr = (fm_arg_stack_t *)calloc(1, tsize);
  if (!ptr)
    return nullptr;
  ptr->header.size = size;
  ptr->header.cursor = &(ptr->buffer[size]);
  return ptr;
}

fm_arg_stack_t *fm_arg_stack_copy(fm_arg_stack_t *ptr) {
  size_t oldsize = &(ptr->buffer[ptr->header.size]) - ptr->header.cursor;
  size_t size = fmc_wordceil(oldsize);
  fm_arg_stack_t *nptr =
      fm_arg_stack_alloc(size + sizeof(fm_arg_stack_header_t));
  if (!nptr)
    return nullptr;
  nptr->header.cursor = &nptr->buffer[size] - oldsize;
  memcpy(nptr->header.cursor, ptr->header.cursor, oldsize);
  return nptr;
}

fm_arg_stack_t *fm_arg_stack_realloc(fm_arg_stack_t *ptr, size_t tsize) {
  if (tsize <= sizeof(fm_arg_stack_header_t)) {
    return nullptr;
  }
  size_t size = tsize - sizeof(fm_arg_stack_header_t);
  size_t oldsize = &(ptr->buffer[ptr->header.size]) - ptr->header.cursor;
  if (oldsize > size) {
    return nullptr;
  }
  fm_arg_stack_t *nptr = (fm_arg_stack_t *)calloc(1, tsize);
  if (!nptr)
    return nullptr;
  nptr->header.size = size;
  nptr->header.cursor = &nptr->buffer[size] - oldsize;
  memcpy(nptr->header.cursor, ptr->header.cursor, oldsize);
  free((void *)ptr);
  return nptr;
}

bool fm_arg_stack_double(fm_arg_stack_t **ptr) {
  fm_arg_stack_t *nptr = fm_arg_stack_realloc(
      *ptr, 2 * (sizeof(fm_arg_stack_header_t) + (*ptr)->header.size));
  if (nptr) {
    *ptr = nptr;
    return true;
  } else {
    return false;
  }
}

fm_arg_stack_t fm_arg_stack_args(fm_arg_stack_t *stack) {
  return fm_arg_stack_t{{stack->header.size,
                        &(stack->buffer[stack->header.size])}};
}

void fm_arg_stack_free(fm_arg_stack_t *ptr) { free((void *)ptr); }
