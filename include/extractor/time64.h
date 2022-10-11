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
 * @file time64.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the time object
 *
 * This file contains declarations of the time object
 * @see http://www.featuremine.com
 */

// TODO: Delete this file, we should be using the common time

#ifndef __fmc_time64_H__
#define __fmc_time64_H__

#include "fmc/platform.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
  int64_t value;
} fmc_time64_t;

inline fmc_time64_t fmc_time64_from_raw(int64_t value) {
  fmc_time64_t res = {value};
  return res;
}

inline fmc_time64_t fmc_time64_from_nanos(int64_t value) {
  return fmc_time64_from_raw(value);
}

inline fmc_time64_t fmc_time64_from_seconds(int32_t value) {
  return fmc_time64_from_nanos(value * 1000000000ULL);
}

inline int64_t fmc_time64_to_nanos(fmc_time64_t t) { return t.value; }

inline double fmc_time64_to_fseconds(fmc_time64_t t) {
  return (double)t.value / 1000000000.0;
}

inline int64_t fmc_time64_raw(fmc_time64_t time) { return time.value; }

inline bool fmc_time64_less(fmc_time64_t a, fmc_time64_t b) {
  return a.value < b.value;
}

inline bool fmc_time64_greater(fmc_time64_t a, fmc_time64_t b) {
  return a.value > b.value;
}

inline bool fmc_time64_equal(fmc_time64_t a, fmc_time64_t b) {
  return a.value == b.value;
}

inline int64_t fmc_time64_div(fmc_time64_t a, fmc_time64_t b) {
  return a.value / b.value;
}

inline fmc_time64_t fmc_time64_add(fmc_time64_t a, fmc_time64_t b) {
  fmc_time64_t res = {a.value + b.value};
  return res;
}

inline void fmc_time64_inc(fmc_time64_t *a, fmc_time64_t b) {
  a->value += b.value;
}

inline fmc_time64_t fmc_time64_sub(fmc_time64_t a, fmc_time64_t b) {
  fmc_time64_t res = {a.value - b.value};
  return res;
}

inline fmc_time64_t fmc_time64_mul(fmc_time64_t a, int64_t b) {
  fmc_time64_t res = {a.value * b};
  return res;
}

inline fmc_time64_t fmc_time64_int_div(fmc_time64_t a, int64_t b) {
  fmc_time64_t res = {a.value / b};
  return res;
}

inline fmc_time64_t fmc_time64_start() {
  fmc_time64_t res = {INT64_MIN};
  return res;
}

inline fmc_time64_t fmc_time64_end() {
  fmc_time64_t res = {INT64_MAX};
  return res;
}

inline bool fmc_time64_is_end(fmc_time64_t time) {
  return time.value == INT64_MAX;
}

typedef struct {
  fmc_time64_t start;
  fmc_time64_t end;
} fmc_time64_range_t;

#endif // __fmc_time64_H__
