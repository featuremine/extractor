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
 * @file comp_def_simp.h
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C declaration of the simplified computation interface.
 *
 * This file contains declarations of the simplified computation interface.
 * @see http://www.featuremine.com
 */

extern "C" {
#include "comp_def_simp.h"
#include "call_ctx.h"
#include "comp_sys.h"
#include "time64.h"
}

fm_call_def *fm_comp_def_simp_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *cdef = (fm_comp_def_simp_t *)comp_cl;
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, cdef->init);
  fm_call_def_destroy_set(def, cdef->destroy);
  fm_call_def_range_set(def, cdef->range);
  fm_call_def_exec_set(def, cdef->stream_exec);
  return def;
}

fm_call_def *fm_comp_def_simp_query_call(fm_comp_def_cl comp_cl,
                                         const fm_ctx_def_cl ctx_cl) {
  auto *cdef = (fm_comp_def_simp_t *)comp_cl;
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, cdef->init);
  fm_call_def_destroy_set(def, cdef->destroy);
  fm_call_def_range_set(def, cdef->range);
  fm_call_def_exec_set(def, cdef->query_exec);
  return def;
}

fm_ctx_def_t *fm_comp_def_simp_generate(fm_comp_sys_t *csys,
                                        fm_comp_def_cl closure, unsigned argc,
                                        fm_type_decl_cp argv[],
                                        fm_type_decl_cp ptype,
                                        fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  auto *cdef = (fm_comp_def_simp_t *)closure;
  auto type = cdef->type(sys, argc, argv);
  if (!type) {
    // @note set error here
    return nullptr;
  }
  fm_ctx_def_cl ctx_cl = nullptr;
  if (cdef->new_cl && !cdef->new_cl(&ctx_cl, sys, ptype, plist)) {
    // @note set error here
    return nullptr;
  }
  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, cdef->inplace);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_def_simp_stream_call);
  fm_ctx_def_query_call_set(def, &fm_comp_def_simp_query_call);
  return def;
}

void fm_comp_def_simp_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *cdef = (fm_comp_def_simp_t *)cl;
  auto *ctx_cl = fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    cdef->del_cl(ctx_cl);
}
