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
 * @file comp.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ implementation of the comp object
 *
 * This file contains declarations of the comp
 * @see http://www.featuremine.com
 */

extern "C" {
#include "comp.h"
#include "arg_stack.h"
#include "comp_def.h"
#include "comp_sys.h"
#include "src/arg_serial.h"
}
#include <iostream>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

struct fm_result_ref {
  fm_frame_t *frame = nullptr;
};

struct fm_comp {
  fm_comp_node_t *node = nullptr;
  const fm_comp_def_t *comp_def = nullptr;
  fm_ctx_def_t *ctx_def = nullptr;
  fm_call_def_t *call_def = nullptr;
  fm_call_obj_t *call = nullptr;
  std::vector<fm_comp_clbck_t> clbcks;
  fm_result_ref_t *result_ref = nullptr;
  fm_result_ref result;
  std::string name;
  fm_arg_buffer_t *args = nullptr;
};

const char *fm_comp_type(const fm_comp *obj) { return obj->comp_def->name; }

fm_comp_t *fm_comp_new(const fm_comp_def_t *comp_def, fm_ctx_def_t *ctx_def,
                       const char *name) {
  auto *obj = new fm_comp;
  obj->ctx_def = ctx_def;
  obj->comp_def = comp_def;
  obj->name = name;
  return obj;
}

const fm_ctx_def_t *fm_comp_ctx_def(const fm_comp_t *obj) {
  return obj->ctx_def;
}

const char *fm_comp_name(const fm_comp_t *obj) { return obj->name.c_str(); }

const fm_arg_buffer_t *fm_comp_arg_buffer(const fm_comp *obj) {
  return obj->args;
}

fm_call_obj_t *fm_comp_call(const fm_comp_t *obj) { return obj->call; }

void fm_comp_del(fm_comp_t *obj) {
  fm_comp_call_destroy(obj);
  if (obj->call_def) {
    fm_call_def_del(obj->call_def);
    obj->call_def = nullptr;
  }
  if (obj->ctx_def) {
    if (obj->comp_def->destroy) {
      obj->comp_def->destroy(obj->comp_def->closure, obj->ctx_def);
    }
    fm_ctx_def_del(obj->ctx_def);
    obj->ctx_def = nullptr;
  }
  if (obj->args) {
    fm_arg_buffer_del(obj->args);
  }
  delete obj;
}

void fm_comp_call_destroy(fm_comp_t *obj) {
  if (obj->call) {
    auto *closure = fm_call_obj_exec_cl(obj->call);
    if (closure) {
      auto destroy = fm_call_def_destroy(obj->call_def);
      if (destroy) {
        destroy(closure);
      } else {
        // @todo give some warning or error
        // basically whoever wrote extension
        // forgot to add clean up
      }
    }
    obj->call = nullptr;
  }
}

fm_call_obj_t *fm_stream_call_obj_new(fm_comp_t *comp, fm_exec_ctx_p ctx,
                                      unsigned argc) {
  // @note cannot copy the call here, because the call gets
  // copied to a stack.
  auto *call = fm_call_obj_new(argc);
  fm_call_obj_exec_ctx_set(call, ctx);
  auto *ctx_def = comp->ctx_def;
  auto *comp_def = comp->comp_def;
  fm_call_obj_comp_ctx_set(call, fm_ctx_def_closure(ctx_def));
  fm_call_obj_queuer_set(call, fm_ctx_def_queuer(ctx_def));
  comp->call_def = fm_ctx_def_stream_call(comp_def->closure, ctx_def);
  for (auto &clbck : comp->clbcks)
    fm_call_obj_clbck_set(call, clbck.clbck, clbck.cl);
  return call;
}

bool fm_comp_inplace(const fm_comp_t *obj) {
  return fm_ctx_def_inplace(obj->ctx_def);
}

bool fm_comp_volatile(const fm_comp_t *obj) {
  return fm_ctx_def_volatile(obj->ctx_def);
}

fm_result_ref_t *fm_result_ref_get(fm_comp_t *obj) {
  obj->result_ref = &obj->result;
  return obj->result_ref;
}

fm_frame_t *fm_data_get(fm_result_ref_t *ref) { return ref->frame; }

void fm_comp_result_set(fm_result_ref_t *ref, fm_frame_t *frame) {
  ref->frame = frame;
}

bool fm_comp_data_required(const fm_comp_t *obj) {
  return obj->result_ref != nullptr;
}

fm_type_decl_cp fm_comp_result_type(const fm_comp_t *obj) {
  return fm_ctx_def_type(obj->ctx_def);
}

fm_frame_t *fm_comp_frame_mk(const fm_comp_t *obj, fm_frame_alloc_t *alloc) {
  return fm_frame_from_type(alloc, fm_comp_result_type(obj));
}

bool fm_comp_call_init(fm_comp_t *obj, fm_call_obj_t *call) {
  // @note here we get an actual obj pointer from the stack
  obj->call = call;
  auto *result = fm_call_obj_result(call);
  obj->result.frame = result;
  auto init = fm_call_def_init(obj->call_def);
  fm_call_exec_cl exec_cl = nullptr;
  bool res = true;
  if (init != nullptr) {
    res = init(result, fm_call_obj_argc(call), fm_call_obj_argv(call),
               fm_call_obj_ctx(call), &exec_cl);
  }
  auto exec = fm_call_def_exec(obj->call_def);
  fm_call_obj_exec_set(call, exec, exec_cl);
  return (exec != NULL) && res;
}

fm_comp_node_t *fm_comp_node_ptr(fm_comp_t *obj) { return obj->node; }

const fm_comp_node_t *fm_comp_node_cptr(const fm_comp_t *obj) {
  return obj->node;
}

void fm_comp_node_ptr_set(fm_comp_t *obj, fm_comp_node_t *node) {
  obj->node = node;
}

void fm_comp_clbck_set(fm_comp_t *obj, fm_frame_clbck_p clbck,
                       fm_frame_clbck_cl closure) {

  obj->clbcks.emplace_back(fm_comp_clbck_t{clbck, closure});
}

void fm_comp_set_args(fm_comp_t *comp, fm_type_decl_cp type,
                      fm_arg_stack_t args) {
  if (comp->args) {
    fm_arg_buffer_del(comp->args);
  }
  comp->args = fm_arg_buffer_new(type, args);
}

const fm_comp_def_t *fm_comp_get_def(const fm_comp_t *comp) {
  return comp->comp_def;
}

bool fm_comp_clbck_has(const fm_comp_t *obj) { return !obj->clbcks.empty(); }

fm_comp_clbck_it fm_comp_clbck_begin(fm_comp_t *obj) {
  return obj->clbcks.data();
}

fm_comp_clbck_it fm_comp_clbck_end(fm_comp_t *obj) {
  return obj->clbcks.data() + obj->clbcks.size();
}
