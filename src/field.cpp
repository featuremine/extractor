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
 * @file field.cpp
 * @author Maxim Trokhimtchouk
 * @date Mar 28 2017
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "field.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"

#include <stdlib.h>
#include <string.h>
#include <string>
#include <unordered_map>

using namespace std;

bool fm_comp_field_call_stream_init(fm_frame_t *result, size_t args,
                                    const fm_frame_t *const argv[],
                                    fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto idx = (int64_t)ctx->comp;
  fm_frame_proj_assign(result, argv[0], idx);
  return true;
}

bool fm_comp_field_stream_exec(fm_frame_t *result, size_t,
                               const fm_frame_t *const argv[],
                               fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto idx = (int64_t)ctx->comp;
  fm_frame_proj_assign(result, argv[0], idx);
  return true;
}

fm_call_def *fm_comp_field_stream_call(fm_comp_def_cl comp_cl,
                                       const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_field_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_field_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_field_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                unsigned argc, fm_type_decl_cp argv[],
                                fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    auto *errstr = "expect a field name as a single parameter";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto *name = fm_arg_try_cstring(fm_type_tuple_arg(ptype, 0), &plist);
  auto type = fm_frame_proj_type_get(sys, argv[0], name);

  if (!type) {
    auto *errstr = "parameter must be a field name of the input frame";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  int64_t cl = fm_type_frame_field_idx(argv[0], name);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_closure_set(def, (void *)cl);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_stream_call_set(def, &fm_comp_field_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}
