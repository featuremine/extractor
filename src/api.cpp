/******************************************************************************

        COPYRIGHT (c) 2022 by Featuremine Corporation.
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
 * @file api.cpp
 * @date 4 Oct 2022
 * @brief File contains implementation of yamal sequence API
 *
 * File contains implementation of yamal sequence API
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/api.h"
#include "comp.h"
#include "extractor/comp_sys.h"
#include "extractor/exec_ctx.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_sys.h"
#include <extractor/comp_sys_capture.h>
#include <extractor/frame.h>
#include "fmc/string.h"
}

#include "comp_sys.hpp"
#include <uthash/utlist.h>

#if defined(FMC_SYS_UNIX)
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

static void module_type_add(struct fm_comp_sys_module *mod,
                               const fm_comp_def_t *def, fmc_error_t **error) {
  fmc_error_clear(error);
  fm_comp_type_add(mod->sys, def);
}

static struct extractor_api_v1 api_v1 {
  fm_comp_sys_cleanup, fm_type_sys_get, fm_type_sys_err_custom,
      fm_comp_graph_get, fm_stream_ctx_get, fm_stream_ctx_recorded,
      fm_stream_ctx_replayed, fm_stream_ctx_queue, fm_stream_ctx_schedule,
      fm_stream_ctx_scheduled, fm_stream_ctx_idle, fm_stream_ctx_proc_one,
      fm_stream_ctx_next_time, fm_stream_ctx_now, fm_comp_sys_error_msg,
      fm_comp_type_add, fm_comp_find, fm_exec_ctx_is_error,
      fm_exec_ctx_error_msg, fm_exec_ctx_error_set, fm_result_ref_get,
      fm_comp_clbck_set, fm_comp_result_type, fm_type_frame_nfields,
      fm_type_frame_field_name, fm_type_is_base, fm_type_base_enum,
      fm_type_is_cstring, fm_type_tuple_arg, fm_args_empty, fm_type_is_tuple,
      fm_type_tuple_size, fm_frame_get_cptr1, fm_frame_get_ptr1, fm_frame_field,
      fm_data_get, fm_call_def_new, fm_call_def_init_set,
      fm_call_def_destroy_set, fm_call_def_exec_set, fm_ctx_def_new,
      fm_ctx_def_inplace_set, fm_ctx_def_type_set, fm_ctx_def_closure_set,
      fm_ctx_def_volatile_set, fm_ctx_def_queuer_set,
      fm_ctx_def_stream_call_set, fm_ctx_def_query_call_set, fm_ctx_def_closure,
      fm_base_type_get, fm_array_type_get, fm_type_is_array, fm_type_array_size,
      fm_type_array_of, fm_type_to_str, fm_type_frame_field_idx, fm_frame_type,
      fm_type_is_frame, fm_frame_reserve, fm_frame_type_get1,
      fm_type_frame_field_type,

      fm_cstring_type_get, fm_tuple_type_get,

      fm_comp_decl, fm_frame_type_get, fm_type_type_get, fm_type_sys_errmsg,
      module_type_add
};

struct extractor_api_v1 *extractor_api_v1_get() {
  return &api_v1;
}


static struct fm_comp_sys_module *
mod_load(struct fm_comp_sys *sys, const char *dir, const char *modstr,
         const char *mod_lib, const char *mod_func, fmc_error_t **error,
         bool *should_skip) {
  fmc_error_clear(error);
  *should_skip = false;
  fm_comp_sys_module_init_func mod_init;
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
      (fm_comp_sys_module_init_func)fmc_ext_sym(mod.handle, mod_func, error);
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
  mod_init(extractor_api_v1_get(), &mod, error);
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
  sprintf(mod_lib, "%s%s", mod, EXTRACTOR_LIB_SUFFIX);

  int pathlen = fmc_path_join(NULL, 0, mod, mod_lib) + 1;
  char mod_lib_2[pathlen];
  fmc_path_join(mod_lib_2, pathlen, mod, mod_lib);

  char mod_func[strlen(EXTRACTOR_COMPONENT_INIT_FUNC_PREFIX) + strlen(mod) + 1];
  sprintf(mod_func, "%s%s", EXTRACTOR_COMPONENT_INIT_FUNC_PREFIX, mod);
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
