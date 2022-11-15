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
 * @file comp_def.cpp
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C++ implementation of the computation interface
 *
 * This file contains implementation of the computation interface.
 * @see http://www.featuremine.com
 */

#include "extractor/comp_def.h"
#include "extractor/call_ctx.h"
#include "extractor/frame_base.h"
#include "fmc/time.h"

#include <stdarg.h>

struct fm_call_def {
  fm_call_init_p init;
  fm_call_destroy_p destroy;
  fm_call_range_p range;
  fm_call_exec_p exec;
};

struct fm_ctx_def {
  fm_type_decl_cp type;
  fm_call_def *(*stream)(fm_comp_def_cl, const fm_ctx_def_cl);
  fm_call_def *(*query)(fm_comp_def_cl, const fm_ctx_def_cl);
  fm_call_queuer_p queuer = nullptr;
  int16_t inplace = 0;
  int16_t volatile_result = 0;
  fm_ctx_def_cl closure;
};

fm_ctx_def_t *fm_ctx_def_new() {
  auto *obj = new fm_ctx_def_t();
  obj->type = nullptr;
  obj->stream = nullptr;
  obj->query = nullptr;
  obj->queuer = nullptr;
  obj->inplace = false;
  obj->volatile_result = 0;
  obj->closure = nullptr;
  return obj;
}

void fm_ctx_def_del(fm_ctx_def_t *obj) { delete obj; }

fm_type_decl_cp fm_ctx_def_type(const fm_ctx_def_t *obj) { return obj->type; }

void fm_ctx_def_type_set(fm_ctx_def_t *obj, fm_type_decl_cp type) {
  obj->type = type;
}

fm_type_decl_cp fm_ctx_def_type_get(fm_ctx_def_t *obj) { return obj->type; }

bool fm_ctx_def_inplace(const fm_ctx_def_t *obj) { return obj->inplace; }

bool fm_ctx_def_volatile(const fm_ctx_def_t *obj) {
  return obj->volatile_result != 0;
}

void fm_ctx_def_inplace_set(fm_ctx_def_t *obj, bool inplace) {
  obj->inplace = inplace;
}

void fm_ctx_def_volatile_set(fm_ctx_def_t *obj, unsigned vol) {
  obj->volatile_result = vol;
}

unsigned fm_ctx_def_volatile_get(const fm_ctx_def_t *obj) {
  return obj->volatile_result;
}

void fm_ctx_def_closure_set(fm_ctx_def_t *obj, fm_ctx_def_cl closure) {
  obj->closure = closure;
}

void fm_ctx_def_queuer_set(fm_ctx_def_t *obj, fm_call_queuer_p q) {
  obj->queuer = q;
}

void fm_ctx_def_stream_call_set(fm_ctx_def_t *obj,
                                fm_call_def_t *(*stream)(fm_comp_def_cl,
                                                         const fm_ctx_def_cl)) {
  obj->stream = stream;
}

void fm_ctx_def_query_call_set(fm_ctx_def_t *obj,
                               fm_call_def_t *(*query)(fm_comp_def_cl,
                                                       const fm_ctx_def_cl)) {
  obj->query = query;
}

fm_call_def_t *fm_ctx_def_stream_call(fm_comp_def_cl comp_cl,
                                      fm_ctx_def_t *obj) {
  return obj->stream(comp_cl, obj->closure);
}

fm_call_def_t *fm_ctx_def_query_call(fm_comp_def_cl comp_cl,
                                     fm_ctx_def_t *obj) {
  return obj->query(comp_cl, obj->closure);
}

fm_ctx_def_cl fm_ctx_def_closure(fm_ctx_def_t *obj) { return obj->closure; }

fm_call_queuer_p fm_ctx_def_queuer(fm_ctx_def_t *obj) { return obj->queuer; }

fm_call_def_t *fm_call_def_new() {
  auto *obj = new fm_call_def_t();
  obj->init = nullptr;
  obj->destroy = nullptr;
  obj->range = nullptr;
  return obj;
}

void fm_call_def_del(fm_call_def_t *obj) { delete obj; }

void fm_call_def_init_set(fm_call_def_t *obj, fm_call_init_p init) {
  obj->init = init;
}

void fm_call_def_destroy_set(fm_call_def_t *obj, fm_call_destroy_p d) {
  obj->destroy = d;
}

void fm_call_def_range_set(fm_call_def_t *obj, fm_call_range_p range) {
  obj->range = range;
}

void fm_call_def_exec_set(fm_call_def_t *obj, fm_call_exec_p exec) {
  obj->exec = exec;
}

fm_call_init_p fm_call_def_init(fm_call_def_t *obj) { return obj->init; }

fm_call_destroy_p fm_call_def_destroy(fm_call_def_t *obj) {
  return obj->destroy;
}

fm_call_range_p fm_call_def_range(fm_call_def_t *obj) { return obj->range; }

fm_call_exec_p fm_call_def_exec(fm_call_def_t *obj) { return obj->exec; }
