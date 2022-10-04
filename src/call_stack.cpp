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
 * @file call_stack.cpp
 * @author Maxim Trokhimtchouk
 * @date 2 Aug 2017
 * @brief File contains C++ implementation of the call stack object
 *
 * This file contains declarations of the call stack interface
 * @see http://www.featuremine.com
 */

extern "C" {
#include "call_stack.h"
#include "call_obj.h"
#include <fmc/alignment.h>
}
#include "unique_pq.hpp"

#include <queue>
#include <stdlib.h>
#include <string.h>

/**
 * @brief execution item object
 *
 * Execution item object represents a callable object
 * on a dependency call stack. It can be executed and
 * has dependencies down the stack.
 */
typedef struct fm_call_item {
  unsigned depc;
  bool term;
  fm_call_handle_t deps[];
} fm_call_item_t;

fm_call_obj_t *fm_call_item_obj(fm_call_item_t *p) {
  return (fm_call_obj_t *)(&p->deps[p->depc]);
}

size_t fm_call_item_size_needed(size_t depc, fm_call_obj_t *obj) {
  return fmc_wordceil(sizeof(fm_call_item) + depc * sizeof(fm_call_handle_t) +
                      fm_call_obj_size(obj));
}
size_t fm_call_item_size(fm_call_item_t *p) {
  return fm_call_item_size_needed(p->depc, fm_call_item_obj(p));
}

fm_call_item_t *fm_call_item_make(char *ptr, unsigned depc,
                                  fm_call_handle_t *deps, fm_call_obj_t *obj) {
  fm_call_item_t *p = (fm_call_item_t *)ptr;
  p->depc = depc;
  p->term = true;
  if (depc)
    memcpy(p->deps, deps, depc * sizeof(fm_call_handle_t));

  fm_call_obj_copy(fm_call_item_obj(p), obj);
  return p;
}

struct fm_call_stack {
  char *begin = nullptr;
  char *end = nullptr;
  size_t size = 0;
};

fm_call_stack_t *fm_call_stack_new() { return new fm_call_stack(); }

char *fm_call_stack_front(fm_call_stack_t *s) { return s->end - s->size; }

fm_call_item_t *fm_call_stack_begin(fm_call_stack_t *s) {
  return (fm_call_item_t *)(s->begin);
}

fm_call_item_t *fm_call_stack_end(fm_call_stack_t *s) {
  return (fm_call_item_t *)(s->end);
}

fm_call_item_t *fm_call_stack_next(fm_call_item_t *p) {
  return (fm_call_item_t *)((char *)p + fm_call_item_size(p));
}

fm_call_item_t *fm_call_stack_item(fm_call_stack_t *s, fm_call_handle_t off) {
  return (fm_call_item_t *)(s->end - off);
}

void fm_call_stack_del(fm_call_stack_t *s) {
  if (s->end) {
    for (auto i = fm_call_stack_begin(s); i != fm_call_stack_end(s);
         i = fm_call_stack_next(i)) {
      fm_call_obj_cleanup(fm_call_item_obj(i));
    }
    free(fm_call_stack_front(s));
  }
  delete s;
}

fm_call_obj_t *fm_call_stack_obj(fm_call_stack_t *s, fm_call_handle_t off) {
  return fm_call_item_obj(fm_call_stack_item(s, off));
}

size_t fm_call_stack_item_depc(fm_call_stack_t *s, fm_call_handle_t off) {
  return fm_call_stack_item(s, off)->depc;
}

const fm_call_handle_t *fm_call_stack_item_deps(fm_call_stack_t *s,
                                                fm_call_handle_t off) {
  return fm_call_stack_item(s, off)->deps;
}

fm_call_handle_t fm_call_stack_offset(fm_call_stack_t *s, fm_call_item_t *i) {
  return s->end - (char *)i;
}

char *fm_call_stack_mem_ensure(fm_call_stack_t *s, size_t needed) {
  if (s->end + needed > s->size + s->begin) {
    size_t used = s->end - s->begin;
    size_t ns = FMC_POW2BND(used + needed);
    char *ptr = (char *)malloc(ns);
    if (!ptr)
      return nullptr;
    auto *front = s->end - s->size;
    s->end = ptr + ns;
    if (used) {
      memcpy(s->end - used, s->begin, used);
      free(front);
    }
    s->begin = s->end - used;
    s->size = ns;
  }
  return s->begin - needed;
}

fm_call_handle_t fm_call_stack_push(fm_call_stack_t *s, fm_call_obj_t *obj,
                                    int depc, fm_call_handle_t deps[]) {
  auto needed = fm_call_item_size_needed(depc, obj);
  if (auto ptr = fm_call_stack_mem_ensure(s, needed)) {
    for (auto i = 0; i < depc; ++i) {
      fm_call_stack_item(s, deps[i])->term = false;
    }
    auto *i = fm_call_item_make(ptr, depc, deps, obj);
    s->begin = (char *)i;
    return fm_call_stack_offset(s, i);
  }
  return 0;
}

struct fm_call_queue
    : fm::unique_pq<fm_call_handle_t, std::vector<fm_call_handle_t>,
                    std::greater<fm_call_handle_t>> {};

fm_call_queue_t *fm_call_queue_new() { return new fm_call_queue(); }

void fm_call_queue_del(fm_call_queue_t *q) { return delete q; }

bool fm_call_queue_empty(fm_call_queue_t *q) { return q->empty(); }

void fm_call_queue_push(fm_call_queue_t *q, fm_call_handle_t o) { q->push(o); }

bool fm_call_stack_exec_one(fm_call_stack_t *s, fm_call_queue_t *q) {
  if (q->empty())
    return false;
  auto *top = fm_call_stack_item(s, q->pop());
  auto *obj = fm_call_item_obj(top);
  if (fm_call_obj_exec(obj)) {
    for (unsigned i = 0; i < top->depc; ++i) {
      // @todo presort dependency offsets to optimize queueing
      q->push(top->deps[i]);
    }
    fm_call_obj_deps_queue(obj);
    return true;
  }
  return false;
}

bool fm_call_stack_exec(fm_call_stack_t *s, fm_call_queue_t *q) {
  bool done = false;
  while (!q->empty()) {
    done |= fm_call_stack_exec_one(s, q);
  };
  return done;
}
