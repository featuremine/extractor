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
 * @file csv_play.cpp
 * @authors Andres Rangel, Leandro Leon
 * @date 21 Aug 2018
 * @brief File contains C++ definitions for the "and" logical operator
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "logical_or.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include "extractor/decimal64.hpp"
#include "extractor/frame.hpp"
#include "op_util.hpp"
#include "extractor/time64.hpp"
#include <fmc++/mpl.hpp>

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

bool fm_comp_logical_or_call_stream_init(fm_frame_t *result, size_t args,
                                         const fm_frame_t *const argv[],
                                         fm_call_ctx_t *ctx,
                                         fm_call_exec_cl *cl) {
  auto fields = *(size_t *)ctx->comp;
  for (size_t field = 0; field < fields; ++field) {
    bool res = false;
    for (size_t i = 0; i < args; ++i) {
      res = res || *(const bool *)fm_frame_get_cptr1(argv[i], field, 0);
      if (res) // for shorcut evaluation of logical_and operator
        break;
    }
    *(bool *)fm_frame_get_ptr1(result, field, 0) = res;
  }
  return true;
}

bool fm_comp_logical_or_stream_exec(fm_frame_t *result, size_t args,
                                    const fm_frame_t *const argv[],
                                    fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto fields = *(size_t *)ctx->comp;
  for (size_t field = 0; field < fields; ++field) {
    bool res = false;
    for (size_t i = 0; i < args; ++i) {
      res = res || *(const bool *)fm_frame_get_cptr1(argv[i], field, 0);
      if (res) // for shorcut evaluation of logical_and operator
        break;
    }
    *(bool *)fm_frame_get_ptr1(result, field, 0) = res;
  }
  return true;
}

fm_call_def *fm_comp_logical_or_stream_call(fm_comp_def_cl comp_cl,
                                            const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_logical_or_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_logical_or_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_logical_or_gen(fm_comp_sys_t *csys,
                                     fm_comp_def_cl closure, unsigned argc,
                                     fm_type_decl_cp argv[],
                                     fm_type_decl_cp ptype,
                                     fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  if (argc < 2) {
    auto *errstr = "expect at least two operator arguments";
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

  auto nf = fm_type_frame_nfields(argv[0]);

  if (nf != 1) {
    for (size_t i = 1; i < argc; ++i) {
      if (!fm_type_equal(argv[0], argv[i])) {
        auto *errstr = "all operator arguments must be the same type or have a "
                       "single field";
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
        return nullptr;
      }
    }
    for (size_t i = 0; i < nf; ++i) {
      if (!fm_type_equal(fm_type_frame_field_type(argv[0], i), bool_param_t)) {
        auto *errstr = "all fields must be of bool type";
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
        return nullptr;
      }
    }
  } else {
    for (size_t i = 0; i < argc; ++i) {
      if (fm_type_frame_nfields(argv[i]) != 1) {
        auto *errstr = "all frames must have the same number of fields";
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
        return nullptr;
      }
      if (!fm_type_equal(fm_type_frame_field_type(argv[i], 0), bool_param_t)) {
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
  fm_ctx_def_stream_call_set(def, &fm_comp_logical_or_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_logical_or_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (size_t *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
