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
 * @file nan.cpp
 * @author Andres Rangel
 * @date 2 Oct 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "nan.h"
#include "extractor/comp_sys.h"

#include <vector>

using namespace std;

bool fm_comp_nan_call_stream_init(fm_frame_t *result, size_t args,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto nf = fm_type_frame_nfields(fm_frame_type(result));
  for (unsigned i = 0; i < nf; ++i) {
    *(double *)fm_frame_get_ptr1(result, i, 0) = NAN;
  }
  return true;
}

bool fm_comp_nan_stream_exec(fm_frame_t *result, size_t,
                             const fm_frame_t *const argv[], fm_call_ctx_t *ctx,
                             fm_call_exec_cl cl) {
  return false;
}

fm_call_def *fm_comp_nan_stream_call(fm_comp_def_cl comp_cl,
                                     const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_nan_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_nan_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_nan_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                              unsigned argc, fm_type_decl_cp argv[],
                              fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  if (argc != 1) {
    const char *errstr = "expect one operator as input";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect no "
                           "parameters");
    return nullptr;
  }

  auto nf = fm_type_frame_nfields(argv[0]);
  auto nd = fm_type_frame_ndims(argv[0]);

  std::vector<const char *> names(nf);
  std::vector<fm_type_decl_cp> types(nf);
  int dims[1] = {1};

  auto double_param_t = fm_base_type_get(sys, FM_TYPE_FLOAT64);

  for (unsigned idx = 0; idx < nf; ++idx) {
    names[idx] = fm_type_frame_field_name(argv[0], idx);
    types[idx] = double_param_t;
  }

  auto type = fm_frame_type_get1(sys, nf, names.data(), types.data(), nd, dims);
  if (!type) {
    auto *errstr = "unable to create result frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, nullptr);
  fm_ctx_def_stream_call_set(def, &fm_comp_nan_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}
