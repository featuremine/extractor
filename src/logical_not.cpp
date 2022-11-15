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
 * @file logical_not.cpp
 * @authors Andres Rangel
 * @date 22 Aug 2018
 * @brief File contains C++ definitions for the "logical_not" logical operator
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "logical_not.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "extractor/frame.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/time.hpp"
#include "op_util.hpp"

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

bool fm_comp_logical_not_call_stream_init(fm_frame_t *result, size_t args,
                                          const fm_frame_t *const argv[],
                                          fm_call_ctx_t *ctx,
                                          fm_call_exec_cl *cl) {
  auto fields = *(size_t *)ctx->comp;
  for (size_t field = 0; field < fields; ++field) {
    *(bool *)fm_frame_get_ptr1(result, field, 0) =
        !*(const bool *)fm_frame_get_cptr1(argv[0], field, 0);
  }
  return true;
}

bool fm_comp_logical_not_stream_exec(fm_frame_t *result, size_t args,
                                     const fm_frame_t *const argv[],
                                     fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto fields = *(size_t *)ctx->comp;
  for (size_t field = 0; field < fields; ++field) {
    *(bool *)fm_frame_get_ptr1(result, field, 0) =
        !*(const bool *)fm_frame_get_cptr1(argv[0], field, 0);
  }
  return true;
}

fm_call_def *fm_comp_logical_not_stream_call(fm_comp_def_cl comp_cl,
                                             const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_logical_not_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_logical_not_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_logical_not_gen(fm_comp_sys_t *csys,
                                      fm_comp_def_cl closure, unsigned argc,
                                      fm_type_decl_cp argv[],
                                      fm_type_decl_cp ptype,
                                      fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  if (argc != 1) {
    auto *errstr = "expect one operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect no "
                           "parameters");
    return nullptr;
  }

  auto bool_param_t = fm_base_type_get(sys, FM_TYPE_BOOL);

  auto inp = argv[0];
  size_t nf = fm_type_frame_nfields(inp);

  if (fm_type_frame_nfields(inp) == 1 &&
      !fm_type_equal(fm_type_frame_field_type(inp, 0), bool_param_t)) {
    auto *errstr = "the two fields have different types";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  } else {
    for (size_t i = 0; i < nf; ++i) {
      if (!fm_type_equal(fm_type_frame_field_type(inp, i), bool_param_t)) {
        auto *errstr = "all fields must be of bool type";
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
        return nullptr;
      }
    }
  }

  auto ctx_cl = make_unique<size_t>(fm_type_frame_nfields(argv[0]));

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, argv[0]);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_logical_not_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_logical_not_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (size_t *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
