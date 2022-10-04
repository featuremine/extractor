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
 * @file comp_sys.cpp
 * @author Maxim Trokhimtchouk
 * @date 1 Aug 2017
 * @brief File contains C++ implementation of the computational system
 *
 * This file contains implementation of the computational system
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/comp_sys.h"
#include "extractor/arg_stack.h"
#include "extractor/frame.h"
#include "comp.h"
#include "comp_graph.h"
#include "stream_ctx.h"
#include "extractor/type_sys.h"
}

#include "comp_sys.hpp"
#include "serial_util.hpp"
#include "fmc++/strings.hpp"

#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <functional>
#include <mutex>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

#include "fmc/extension.h"
#include "fmc/platform.h"
#ifdef FMC_LICENSE
#include <license.h>
#include <license.hpp>
#endif

using namespace std;

fm_comp_sys_t *fm_comp_sys_new(const char *license_file, char **errmsg) {

  static mutex m;

  lock_guard<mutex> guard(m);

#ifdef FMC_LICENSE
  RLM_HANDLE rh;
  RLM_LICENSE lic = (RLM_LICENSE)NULL;
  if (!fm::rlm_checkin(&lic, &rh, license_file, errmsg)) {
    return nullptr;
  }
#endif
  auto *s = new fm_comp_sys();
  s->types = fm_type_sys_new();
#ifdef FMC_LICENSE
  s->rh = rh;
  s->lic = lic;
#endif
  return s;
}

void fm_comp_sys_cleanup(fm_comp_sys_t *s) {
  // @note the order is important, the graph need to clean up
  // some of the closures in the call
  for (auto *g : s->graphs)
    fm_comp_graph_del(g);
  s->graphs.clear();
  for (auto &destroy : s->destructors)
    destroy();
  s->destructors.clear();
}

void fm_comp_sys_del(fm_comp_sys_t *s) {
#ifdef FMC_LICENSE
  fm::rlm_checkout(s->lic, s->rh);
#endif
  fm_type_sys_del(s->types);
  fm_comp_sys_cleanup(s);
  delete s;
}

struct fm_result_ref {
  const fm_frame_t *data = nullptr;
};

bool fm_comp_type_add(fm_comp_sys_t *sys, const fm_comp_def_t *def) {
  // @todo copy the definition, this way we can create definitions on the fly
  string name = def->name;
  auto where = sys->defs.find(name);

  if (where != sys->defs.end()) {
    fm_comp_sys_error_set(sys, "a computation with name %s already exists",
                          def->name);
    return false;
  }
  sys->defs.emplace(name, *def);
  return true;
}

fm_comp_graph_t *fm_comp_graph_get(fm_comp_sys_t *sys) {
  auto *g = fm_comp_graph_new();
  sys->graphs.push_back(g);
  return g;
}

void fm_comp_graph_remove(fm_comp_sys_t *sys, fm_comp_graph_t *graph) {
  auto &vec = sys->graphs;
  for (auto it = vec.begin(); it != vec.end();) {
    if (*it == graph) {
      fm_comp_graph_del(graph);
      it = vec.erase(it);
    } else {
      ++it;
    }
  }
}

fm_type_sys_t *fm_type_sys_get(fm_comp_sys_t *sys) { return sys->types; }

fm_comp_t *fm_comp_decl(fm_comp_sys_t *csys, fm_comp_graph *graph,
                        const char *comp, unsigned nargs, fm_type_decl_cp type,
                        ...) {
  va_list args;
  va_start(args, type);
  vector<fm_comp_t *> inputs(nargs);
  for (unsigned i = 0; i < nargs; ++i) {
    inputs[i] = va_arg(args, fm_comp_t *);
  }
  fm_comp_t *inst = nullptr;
  STACK(4096, stack);
  auto res = fm_arg_stack_build(type, STACK_FWD(stack), &args);
  if (res == 0) {
    return fm_comp_decl4(csys, graph, comp, NULL, nargs, inputs.data(), type,
                         STACK_ARGS(stack));
  }
  va_end(args);
  return inst;
}

fm_comp_t *fm_comp_decl2(fm_comp_sys_t *csys, fm_comp_graph *graph,
                         const char *comp, const char *name, unsigned nargs,
                         fm_type_decl_cp type, ...) {
  va_list args;
  va_start(args, type);
  vector<fm_comp_t *> inputs(nargs);
  for (unsigned i = 0; i < nargs; ++i) {
    inputs[i] = va_arg(args, fm_comp_t *);
  }
  fm_comp_t *inst = nullptr;
  STACK(4096, stack);
  auto res = fm_arg_stack_build(type, STACK_FWD(stack), &args);
  if (res == 0) {
    return fm_comp_decl4(csys, graph, comp, name, nargs, inputs.data(), type,
                         STACK_ARGS(stack));
  }
  va_end(args);
  return inst;
}

fm_comp_t *fm_comp_decl4(fm_comp_sys_t *csys, fm_comp_graph *graph,
                         const char *comp, const char *name, unsigned nargs,
                         fm_comp_t **inputs, fm_type_decl_cp type,
                         fm_arg_stack_t args) {
  auto where = csys->defs.find(string(comp));
  if (where == csys->defs.end()) {
    fm_comp_sys_error_set(
        csys, "[ERROR]\t(comp_sys) count not find operator %s", comp);
    return nullptr;
  }
  auto *def = &where->second;

  if (name && fm_comp_node_name_find(graph, name)) {
    fm_comp_sys_error_set(csys,
                          "[ERROR]\t(comp_sys) computation with name "
                          "%s already exists",
                          name);
    return nullptr;
  }
  string namestr;
  if (name) {
    namestr = name;
  } else {
    char *name_ptr = fm_comp_node_uniq_name_gen(graph, comp);
    namestr = name_ptr;
    free(name_ptr);
  }

  vector<fm_comp_node_t *> input_nodes(nargs);
  vector<fm_type_decl_cp> input_types(nargs);
  for (unsigned i = 0; i < nargs; ++i) {
    auto *arg = inputs[i];
    auto *node = fm_comp_node_ptr(arg);
    input_nodes[i] = node;
    input_types[i] = fm_comp_result_type(arg);
  }

  auto *ctx =
      def->generate(csys, def->closure, nargs, input_types.data(), type, args);

  if (!ctx)
    return nullptr;

  auto *inst = fm_comp_new(def, ctx, namestr.c_str());
  fm_comp_set_args(inst, type, args);
  auto *node = fm_comp_graph_add(graph, inst, nargs, input_nodes.data());
  fm_comp_node_ptr_set(inst, node);
  fm_comp_node_name_add(graph, namestr.c_str(), node);
  return inst;
}

fm_comp_t *fm_comp_find(fm_comp_graph_t *g, const char *name) {
  auto *node = fm_comp_node_name_find(g, name);
  if (node) {
    return fm_comp_node_obj(node);
  }
  return nullptr;
}

fm_stream_ctx_t *fm_stream_ctx_get(fm_comp_sys_t *s, fm_comp_graph_t *g) {
  if (!fm_comp_graph_stable_top_sort(g)) {
    fm_comp_sys_error_set(s, "[ERROR]\t(comp_sys) graph has circular "
                             "dependencies");
    return nullptr;
  }
  auto *ctx = fm_stream_ctx_new(g);
  if (fm_exec_ctx_is_error((fm_exec_ctx_t *)ctx)) {
    fm_comp_sys_error_set(s,
                          "[ERROR]\t(comp_sys) failed to create "
                          "stream_ctx;\n\t%s",
                          fm_exec_ctx_error_msg((fm_exec_ctx_t *)ctx));
    fm_stream_ctx_del(ctx);
    return nullptr;
  }

  s->destructors.emplace_back([ctx]() { fm_stream_ctx_del(ctx); });
  return ctx;
}

bool fm_comp_sys_is_error(fm_comp_sys_t *s) { return !s->errmsg.empty(); }

const char *fm_comp_sys_error_msg(fm_comp_sys_t *s) {
  return s->errmsg.c_str();
}

void fm_comp_sys_error_set(fm_comp_sys_t *s, const char *fmt, ...) {
  va_list args1;
  va_start(args1, fmt);
  va_list args2;
  va_copy(args2, args1);
  auto &buf = s->errmsg;
  auto size = vsnprintf(NULL, 0, fmt, args1) + 1;
  vector<char> errmsg(size);
  va_end(args1);
  vsnprintf(errmsg.data(), size, fmt, args2);
  va_end(args2);
  buf.clear();
  buf.append(errmsg.data(), size);
}

bool fm_comp_sys_ext_load(fm_comp_sys_t *s, const char *name,
                          const char *path) {
  fmc_error_t *error;
  string mainfunc_sym = string("FmInit_") + name;
  // TODO: Review this code, the old fmc_ext load would cause
  // a leak because we never call dlclose
  auto handle = fmc_ext_open(path, &error);
  if (error) {
    fm_comp_sys_error_set(s,
                          "[ERROR]\t(comp_sys) failed to load "
                          "extension library %s from %s;\n\t%s",
                          name, path, fmc_error_msg(error));
    return false;
  }
  auto *handler = fmc_ext_sym(handle, mainfunc_sym.c_str(), &error);
  if (error) {
    fm_comp_sys_error_set(s,
                          "[ERROR]\t(comp_sys) failed to load "
                          "extension library %s from %s;\n\t%s",
                          name, path, fmc_error_msg(error));
    return false;
  }
  auto mainfunc = (void (*)(fm_comp_sys_t *))handler;
  mainfunc(s);
  return true;
}

bool fm_comp_sys_sample_value(fm_comp_sys_t *sys, const char *sample_name,
                              double *value) {
  auto where = sys->samples_.find(std::string_view(sample_name));
  if (where != sys->samples_.end()) {
    *value = (*where).second.value();
    return true;
  }
  return false;
}
