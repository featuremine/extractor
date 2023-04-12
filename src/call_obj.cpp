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
 * @file call_obj.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definition of the call object
 *
 * @see http://www.featuremine.com
 */

#include "call_obj.h"
#include "extractor/call_ctx.h"
#include "extractor/handle.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

struct fm_dep_queuer {
  fm_call_queuer_p queuer;
  size_t index;
  fm_call_ctx *ctx;
  fm_dep_queuer *next;
};

struct fm_call_obj {
  std::vector<fm_comp_clbck_t> clbcks;
  fm_call_queuer_p queuer;
  fm_dep_queuer *queuers;
  fm_setup_func_p setup;
  fm_call_exec_p exec;
  fm_call_exec_cl exec_cl;
  fm_frame_t *result;
  fm_call_ctx ctx;
  size_t argc;
  fm_frame_t *argv[];
  fm_call_obj() = default;
  fm_call_obj(const fm_call_obj &obj) {
    queuer = obj.queuer;
    queuers = obj.queuers;
    setup = obj.setup;
    exec = obj.exec;
    exec_cl = obj.exec_cl;
    result = obj.result;
    ctx = obj.ctx;
    argc = obj.argc;
    memcpy(argv, obj.argv, argc * sizeof(fm_frame_t *));
    clbcks = obj.clbcks;
  }
};

size_t fm_call_obj_size_needed(size_t argc) {
  return sizeof(fm_call_obj) + sizeof(fm_frame_t *) * argc;
}

fm_call_obj *fm_call_obj_new(unsigned argc) {
  auto *obj = new (malloc(fm_call_obj_size_needed(argc))) fm_call_obj();
  obj->queuer = nullptr;
  obj->queuers = nullptr;
  obj->setup = nullptr;
  obj->exec = nullptr;
  obj->exec_cl = nullptr;
  obj->result = nullptr;
  obj->ctx.exec = nullptr;
  obj->ctx.comp = nullptr;
  obj->ctx.handle = 0;
  obj->ctx.depc = 0;
  obj->ctx.deps = nullptr;
  obj->argc = argc;
  return obj;
}

void fm_call_obj_cleanup(fm_call_obj_t *obj) {
  fm_dep_queuer *next = obj->queuers;
  while (next) {
    auto *dep_q = next;
    next = next->next;
    delete dep_q;
  }
  obj->~fm_call_obj();
}

void fm_call_obj_del(fm_call_obj_t *obj) {
  fm_call_obj_cleanup(obj);
  free(obj);
}

size_t fm_call_obj_size(fm_call_obj_t *obj) {
  return fm_call_obj_size_needed(obj->argc);
}

size_t fm_call_obj_argc(fm_call_obj_t *call) { return call->argc; }

bool fm_call_obj_exec(fm_call_obj_t *obj) {
  if (fm_exec_ctx_is_error(obj->ctx.exec)) {
    return false;
  }
  if (obj->setup)
    obj->setup(obj);
  bool res =
      obj->exec(obj->result, obj->argc, obj->argv, &obj->ctx, obj->exec_cl);
  if (res) {
    for (auto &clbck : obj->clbcks) {
      if (fm_exec_ctx_is_error(obj->ctx.exec)) {
        break;
      }
      (clbck.clbck)(obj->result, clbck.cl, &obj->ctx);
    }
  }
  return res;
}

fm_call_handle_t fm_call_obj_handle(fm_call_obj_t *obj) {
  return obj->ctx.handle;
}

void fm_call_obj_handle_set(fm_call_obj_t *obj, fm_call_handle_t handle) {
  obj->ctx.handle = handle;
}

void fm_call_obj_depc_set(fm_call_obj_t *obj, size_t depc) {
  obj->ctx.depc = depc;
}

void fm_call_obj_deps_set(fm_call_obj_t *obj, const fm_call_handle_t *deps) {
  obj->ctx.deps = deps;
}

fm_frame_t *fm_call_obj_result(fm_call_obj_t *obj) { return obj->result; }

void fm_call_obj_result_set(fm_call_obj_t *obj, fm_frame_t *frame) {
  obj->result = frame;
}

fm_frame_t *const *fm_call_obj_argv(fm_call_obj_t *obj) { return obj->argv; }

fm_frame_t *fm_call_obj_arg(fm_call_obj_t *obj, size_t argi) {
  return obj->argv[argi];
}

void fm_call_obj_arg_set(fm_call_obj_t *obj, size_t argi, fm_frame_t *f) {
  obj->argv[argi] = f;
}

void fm_call_obj_setup_set(fm_call_obj_t *obj, fm_setup_func_p setfunc) {
  obj->setup = setfunc;
}

fm_call_ctx *fm_call_obj_ctx(fm_call_obj_t *obj) { return &obj->ctx; }

void fm_call_obj_exec_ctx_set(fm_call_obj_t *obj, fm_exec_ctx_p ctx) {
  obj->ctx.exec = ctx;
}

void fm_call_obj_comp_ctx_set(fm_call_obj_t *obj, fm_comp_ctx_p ctx) {
  obj->ctx.comp = ctx;
}

void fm_call_obj_exec_set(fm_call_obj_t *obj, fm_call_exec_p func,
                          fm_call_exec_cl cl) {
  obj->exec = func;
  obj->exec_cl = cl;
}

fm_call_exec_cl fm_call_obj_exec_cl(fm_call_obj_t *obj) { return obj->exec_cl; }

void fm_call_obj_clbck_set(fm_call_obj_t *obj, fm_frame_clbck_p func,
                           fm_frame_clbck_cl cl) {
  obj->clbcks.emplace_back(fm_comp_clbck_t{func, cl});
}

fm_exec_ctx_p fm_call_obj_exec_ctx(fm_call_obj_t *obj) { return obj->ctx.exec; }

void fm_call_obj_queuer_set(fm_call_obj_t *obj, fm_call_queuer_p q) {
  obj->queuer = q;
}

void fm_call_obj_deps_queue(fm_call_obj_t *obj) {
  fm_dep_queuer **next = &obj->queuers;
  while (*next) {
    (*next)->queuer((*next)->index, (*next)->ctx);
    next = &((*next)->next);
  }
}

void fm_call_obj_dep_queuer_add(fm_call_obj_t *in, fm_call_obj_t *out,
                                size_t idx) {
  auto *queuer = out->queuer;
  if (!queuer)
    return;
  fm_dep_queuer **next = &in->queuers;
  for (; *next; next = &((*next)->next)) {
  }
  *next = new fm_dep_queuer{queuer, idx, &out->ctx, nullptr};
}

void fm_call_obj_copy(void *ptr, fm_call_obj_t *obj) {
  ptr = new (ptr) fm_call_obj(*obj);
}
