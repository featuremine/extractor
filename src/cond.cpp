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
 * @file cond.cpp
 * @authors Andres Rangel
 * @date 22 Aug 2018
 * @brief File contains C++ definitions for the "cond" logical operator
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "cond.h"
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

bool fm_comp_cond_call_stream_init(fm_frame_t *result, size_t args,
                                   const fm_frame_t *const argv[],
                                   fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  if (*(const bool *)fm_frame_get_cptr1(argv[0], 0, 0))
    fm_frame_assign(result, argv[1]);
  else
    fm_frame_assign(result, argv[2]);
  return true;
}

bool fm_comp_cond_stream_exec(fm_frame_t *result, size_t args,
                              const fm_frame_t *const argv[],
                              fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  if (*(const bool *)fm_frame_get_cptr1(argv[0], 0, 0))
    fm_frame_assign(result, argv[1]);
  else
    fm_frame_assign(result, argv[2]);
  return true;
}

fm_call_def *fm_comp_cond_stream_call(fm_comp_def_cl comp_cl,
                                      const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_cond_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_cond_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_cond_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                               unsigned argc, fm_type_decl_cp argv[],
                               fm_type_decl_cp ptype, fm_arg_stack_t plist) {

  auto *sys = fm_type_sys_get(csys);

  if (argc != 3) {
    auto *errstr = "expect three operator arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect no "
                           "parameters");
    return nullptr;
  }

  if (fm_type_frame_nfields(argv[0]) != 1) {
    auto *errstr = "first argument must have one field";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_type_equal(fm_type_frame_field_type(argv[0], 0),
                     fm_base_type_get(sys, FM_TYPE_BOOL))) {
    auto *errstr = "first argument field type must be bool";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_type_equal(argv[1], argv[2])) {
    auto *errstr =
        "the  second and third operator must be the same type or have a "
        "single field";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, argv[1]);
  fm_ctx_def_closure_set(def, nullptr);
  fm_ctx_def_stream_call_set(def, &fm_comp_cond_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_cond_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {}
