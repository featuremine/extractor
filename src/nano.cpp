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
 * @file nano.cpp
 * @author Maxim Trokhimtchouk
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "nano.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"

#include "extractor/frame.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/time.hpp"

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

struct nano_field_exec {
  virtual void exec(fm_frame_t *result, size_t,
                    const fm_frame_t *const argv[]) = 0;
  virtual ~nano_field_exec(){};
};

template <bool time_to_nanos>
struct nano_field_exec_2_0 : public nano_field_exec {
  nano_field_exec_2_0(fm_field_t field) : field_(field) {}

  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[]) {
    if constexpr (time_to_nanos) {
      auto val0 = *(const fmc_time64_t *)fm_frame_get_cptr1(argv[0], field_, 0);
      *(int64_t *)fm_frame_get_ptr1(result, field_, 0) =
          fmc_time64_to_nanos(val0);
    } else {
      auto val0 = *(const int64_t *)fm_frame_get_cptr1(argv[0], field_, 0);
      *(fmc_time64_t *)fm_frame_get_ptr1(result, field_, 0) =
          fmc_time64_from_nanos(val0);
    }
  }
  fm_field_t field_;
};

struct nano_comp_cl {
  ~nano_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<nano_field_exec *> calls;
};

bool fm_comp_nano_call_stream_init(fm_frame_t *result, size_t args,
                                   const fm_frame_t *const argv[],
                                   fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto &calls = (*(nano_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

bool fm_comp_nano_stream_exec(fm_frame_t *result, size_t args,
                              const fm_frame_t *const argv[],
                              fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &calls = (*(nano_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

fm_call_def *fm_comp_nano_stream_call(fm_comp_def_cl comp_cl,
                                      const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_nano_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_nano_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_nano_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                               unsigned argc, fm_type_decl_cp argv[],
                               fm_type_decl_cp ptype, fm_arg_stack_t plist) {
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

  auto ctx_cl = make_unique<nano_comp_cl>();
  auto &calls = ctx_cl->calls;

  auto time64_type = fm_base_type_get(sys, FM_TYPE_TIME64);
  auto int64_type = fm_base_type_get(sys, FM_TYPE_INT64);

  auto inp = argv[0];
  int nf = fm_type_frame_nfields(inp);
  int nd = fm_type_frame_ndims(inp);
  vector<const char *> names(nf);
  vector<fm_type_decl_cp> types(nf);
  vector<int> dims(nd);

  for (int idx = 0; idx < nd; ++idx) {
    dims[idx] = fm_type_frame_dim(inp, idx);
  }

  for (int idx = 0; idx < nf; ++idx) {
    names[idx] = fm_type_frame_field_name(argv[0], idx);
    auto f_type = fm_type_frame_field_type(inp, idx);
    if (fm_type_equal(f_type, time64_type)) {
      calls.push_back(new nano_field_exec_2_0<true>(idx));
      types[idx] = int64_type;
    } else if (fm_type_equal(f_type, int64_type)) {
      calls.push_back(new nano_field_exec_2_0<false>(idx));
      types[idx] = time64_type;
    } else {
      ostringstream os;
      auto *str = fm_type_to_str(f_type);
      os << "type " << str << "is not supported in nano feature";
      free(str);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, os.str().c_str());
      return nullptr;
    }
  }

  auto type =
      fm_frame_type_get1(sys, nf, names.data(), types.data(), nd, dims.data());
  if (!type) {
    auto *errstr = "unable to create result frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_nano_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_nano_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (nano_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
