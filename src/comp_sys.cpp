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

#include "extractor/comp_sys.h"
#include "comp.h"
#include "comp_graph.h"
#include "extractor/arg_stack.h"
#include "extractor/frame.h"
#include "extractor/type_sys.h"
#include "stream_ctx.h"

#include "comp_sys.hpp"
#include "fmc++/strings.hpp"
#include "serial_util.hpp"

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
#include "fmc/string.h"

#include <uthash/utarray.h>
#include <uthash/utheap.h>
#include <uthash/utlist.h>

#if defined(FMC_SYS_UNIX)
#define EXTRACTOR_MOD_SEARCHPATH_CUR ""
#define EXTRACTOR_MOD_SEARCHPATH_USRLOCAL ".local/lib/extractor/modules"
#define EXTRACTOR_MOD_SEARCHPATH_SYSLOCAL "/usr/local/lib/extractor/modules"
#define EXTRACTOR_MOD_SEARCHPATH_ENV "EXTRACTORPATH"
#define EXTRACTOR_MOD_SEARCHPATH_ENV_SEP ":"
#if defined(FMC_SYS_LINUX)
#define EXTRACTOR_LIB_SUFFIX ".so"
#elif defined(FMC_SYS_MACH)
#define EXTRACTOR_LIB_SUFFIX ".dylib"
#endif
#else
#define EXTRACTOR_MOD_SEARCHPATH_ENV_SEP ";"
#error "Unsupported operating system"
#endif

#define EXTRACTOR_COMPONENT_INIT_FUNC_PREFIX "ExtractorInit_"

using namespace std;

fm_comp_sys_t *fm_comp_sys_new(char **errmsg) {
  auto *s = new fm_comp_sys();
  s->types = fm_type_sys_new();
  s->modules = NULL;
  s->search_paths = NULL;
  return s;
}

void fm_comp_sys_module_destroy(struct fm_comp_sys_module *mod) {
  if (mod->name)
    free(mod->name);
  if (mod->file)
    free(mod->file);
  if (mod->handle)
    fmc_ext_close(mod->handle);
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

  fm_comp_sys_ext_path_list *phead = s->search_paths;
  fm_comp_sys_ext_path_list *p;
  fm_comp_sys_ext_path_list *ptmp;
  DL_FOREACH_SAFE(phead, p, ptmp) {
    DL_DELETE(phead, p);
    free(p);
  }
  s->search_paths = NULL;

  // destroy modules: also destroys components of the module
  struct fm_comp_sys_module *modhead = s->modules;
  struct fm_comp_sys_module *mod;
  struct fm_comp_sys_module *modtmp;
  DL_FOREACH_SAFE(modhead, mod, modtmp) {
    fm_comp_sys_module_destroy(mod);
    free(mod);
  }
  s->modules = NULL;
}

static struct fm_comp_sys_module *
mod_load(struct fm_comp_sys *sys, const char *dir, const char *modstr,
         const char *mod_lib, const char *mod_func, fmc_error_t **error,
         bool *should_skip) {
  fmc_error_clear(error);
  *should_skip = false;
  fm_comp_sys_module_init_v1 mod_init;
  struct fm_comp_sys_module *m;
  int psz = fmc_path_join(NULL, 0, dir, mod_lib) + 1;
  char lib_path[psz];
  fmc_path_join(lib_path, psz, dir, mod_lib);

  struct fm_comp_sys_module mod;
  memset(&mod, 0, sizeof(mod));

  mod.handle = fmc_ext_open(lib_path, error);
  if (*error) {
    *should_skip = true;
    goto cleanup;
  }

  // Check if init function is available
  mod_init =
      (fm_comp_sys_module_init_v1)fmc_ext_sym(mod.handle, mod_func, error);
  if (*error) {
    *should_skip = true;
    goto cleanup;
  }

  // append the mod to the system
  mod.sys = sys;
  mod.name = fmc_cstr_new(modstr, error);
  if (*error)
    goto cleanup;
  mod.file = fmc_cstr_new(lib_path, error);
  if (*error)
    goto cleanup;

  fmc_error_clear(error);
  mod_init(extractor_api_v1_get(), mod.sys, error);
  if (*error) {
    fmc_error_set(error, "failed to load module %s with error: %s", modstr,
                  fmc_error_msg(*error));
    goto cleanup;
  }

  m = (struct fm_comp_sys_module *)calloc(1, sizeof(mod));
  if (!m) {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    goto cleanup;
  }
  memcpy(m, &mod, sizeof(mod));
  DL_APPEND(sys->modules, m);

  return m;

cleanup:
  fm_comp_sys_module_destroy(&mod);
  return NULL;
}

struct fm_comp_sys_module *fm_comp_sys_module_get(struct fm_comp_sys *sys,
                                                  const char *mod,
                                                  fmc_error_t **error) {
  fmc_error_clear(error);

  // If the module exists, get it
  struct fm_comp_sys_module *mhead = sys->modules;
  struct fm_comp_sys_module *mitem;
  DL_FOREACH(mhead, mitem) {
    if (!strcmp(mitem->name, mod)) {
      return mitem;
    }
  }

  struct fm_comp_sys_module *ret = NULL;
  char mod_lib[strlen(mod) + strlen(EXTRACTOR_LIB_SUFFIX) + 1];
  snprintf(mod_lib, sizeof(mod_lib), "%s%s", mod, EXTRACTOR_LIB_SUFFIX);

  int pathlen = fmc_path_join(NULL, 0, mod, mod_lib) + 1;
  char mod_lib_2[pathlen];
  fmc_path_join(mod_lib_2, pathlen, mod, mod_lib);

  char mod_func[strlen(EXTRACTOR_COMPONENT_INIT_FUNC_PREFIX) + strlen(mod) + 1];
  snprintf(mod_func, sizeof(mod_func), "%s%s", EXTRACTOR_COMPONENT_INIT_FUNC_PREFIX, mod);
  struct fm_comp_sys_ext_path_list *head = sys->search_paths;
  struct fm_comp_sys_ext_path_list *item;
  bool should_skip = true;
  DL_FOREACH(head, item) {
    ret =
        mod_load(sys, item->path, mod, mod_lib, mod_func, error, &should_skip);
    if (should_skip) {
      ret = mod_load(sys, item->path, mod, mod_lib_2, mod_func, error,
                     &should_skip);
    }
    if (!should_skip) {
      break;
    }
  }
  if (should_skip) {
    fmc_error_set(error, "component module %s was not found", mod);
  }
  return ret;
}

void fm_comp_sys_del(fm_comp_sys_t *s) {
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

void fm_comp_sys_ext_path_list_add(struct fm_comp_sys_ext_path_list **phead,
                                   const char *path, fmc_error_t **error) {
  fm_comp_sys_ext_path_list *p =
      (fm_comp_sys_ext_path_list *)calloc(1, sizeof(*p) + strlen(path) + 1);
  if (p) {
    strcpy(p->path, path);
    DL_APPEND(*phead, p);
  } else {
    fmc_error_set2(error, FMC_ERROR_MEMORY);
    return;
  }
}

void fm_comp_sys_ext_path_list_del(struct fm_comp_sys_ext_path_list **phead) {
  if (!*phead)
    return;
  fm_comp_sys_ext_path_list *p;
  fm_comp_sys_ext_path_list *ptmp;
  DL_FOREACH_SAFE(*phead, p, ptmp) {
    DL_DELETE(*phead, p);
    free(p);
  }
}

void fm_comp_sys_ext_path_list_set(struct fm_comp_sys_ext_path_list **head,
                                   const char **paths, fmc_error_t **error) {
  fmc_error_clear(error);
  struct fm_comp_sys_ext_path_list *tmp = NULL;
  for (unsigned int i = 0; paths && paths[i]; ++i) {
    fm_comp_sys_ext_path_list_add(&tmp, paths[i], error);
    if (*error) {
      fm_comp_sys_ext_path_list_del(&tmp);
      return;
    }
  }
  fm_comp_sys_ext_path_list_del(head);
  *head = tmp;
}

struct fm_comp_sys_ext_path_list *fm_comp_sys_paths_get(fm_comp_sys_t *s) {
  return s->search_paths;
}

void fm_comp_sys_paths_set(struct fm_comp_sys *sys, const char **paths,
                           fmc_error_t **error) {
  fm_comp_sys_ext_path_list_set(&sys->search_paths, paths, error);
}

void fm_comp_sys_paths_add(struct fm_comp_sys *sys, const char *path,
                           fmc_error_t **error) {
  fmc_error_clear(error);
  if (path) {
    fm_comp_sys_ext_path_list_add(&sys->search_paths, path, error);
  }
}

void fm_comp_sys_paths_set_default(struct fm_comp_sys *sys,
                                   fmc_error_t **error) {
  fmc_error_clear(error);
  fm_comp_sys_ext_path_list *tmpls2;
  fm_comp_sys_ext_path_list *tmpls = NULL;

  char *tmp = getenv("HOME");
  int psz = fmc_path_join(NULL, 0, tmp, EXTRACTOR_MOD_SEARCHPATH_USRLOCAL) + 1;
  char home_path[psz];
  fmc_path_join(home_path, psz, tmp, EXTRACTOR_MOD_SEARCHPATH_USRLOCAL);

  const char *defaults[] = {EXTRACTOR_MOD_SEARCHPATH_CUR, home_path,
                            EXTRACTOR_MOD_SEARCHPATH_SYSLOCAL, NULL};

  fm_comp_sys_ext_path_list_set(&tmpls, defaults, error);
  if (*error)
    goto cleanup;

  tmp = getenv(EXTRACTOR_MOD_SEARCHPATH_ENV);
  if (tmp) {
    char ycpaths[strlen(tmp) + 1];
    strcpy(ycpaths, tmp);
    char *found;
    tmp = ycpaths;
    while ((found = strsep(&tmp, EXTRACTOR_MOD_SEARCHPATH_ENV_SEP))) {
      fm_comp_sys_ext_path_list_add(&tmpls, found, error);
      if (*error)
        goto cleanup;
    }
  }
  tmpls2 = sys->search_paths;
  sys->search_paths = tmpls;
  tmpls = tmpls2;
  return;
cleanup:
  fm_comp_sys_ext_path_list_del(&tmpls);
}

bool fm_comp_sys_ext_load(fm_comp_sys_t *s, const char *name) {
  fmc_error_t *error;
  fm_comp_sys_module *mod = fm_comp_sys_module_get(s, name, &error);
  if (error) {
    fm_comp_sys_error_set(s,
                          "[ERROR]\t(comp_sys) failed to load "
                          "extension library %s;\n\t%s",
                          name, fmc_error_msg(error));
    return false;
  }
  DL_APPEND(s->modules, mod);
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

static ytp_sequence_api_v1 *api_v1 = nullptr;

void set_ytp_api_v1(ytp_sequence_api_v1 *api) { api_v1 = api; }

ytp_sequence_api_v1 *get_ytp_api_v1() { return api_v1; }
