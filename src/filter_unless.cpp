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
 * @file filter_unless.cpp
 * @authors Andres Rangel
 * @date 22 Aug 2018
 * @brief File contains C++ definitions for the "filter_unless" logical operator
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "filter_unless.h"
#include "arg_stack.h"
#include "comp_def.h"
#include "comp_sys.h"
#include "stream_ctx.h"
#include "time64.h"
}

#include "decimal64.hpp"
#include "frame.hpp"
#include "time64.hpp"
#include <fmc++/mpl.hpp>

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

struct op_field_exec {
  virtual ~op_field_exec() {}
  virtual bool exec(fm_frame_t *result, size_t,
                    const fm_frame_t *const argv[]) = 0;
};

struct the_filter_unless_field_exec_2_0 : op_field_exec {
  the_filter_unless_field_exec_2_0(fm_field_t field) : field_(field) {}

  bool exec(fm_frame_t *result, size_t args,
            const fm_frame_t *const argv[]) override {
    return !*(const bool *)fm_frame_get_cptr1(argv[0], field_, 0);
  }

  fm_field_t field_;
};

struct filter_unless_comp_cl {
  ~filter_unless_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<op_field_exec *> calls;
  bool updated = false;
};

bool fm_comp_filter_unless_call_stream_init(fm_frame_t *result, size_t args,
                                            const fm_frame_t *const argv[],
                                            fm_call_ctx_t *ctx,
                                            fm_call_exec_cl *cl) {
  fm_frame_assign(result, argv[args - 1]);
  return true;
}

bool fm_comp_filter_unless_stream_exec(fm_frame_t *result, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *comp_cl = (filter_unless_comp_cl *)ctx->comp;
  auto &calls = comp_cl->calls;
  for (auto &call : calls) {
    if (!call->exec(result, args, argv))
      return false;
  }
  if (comp_cl->updated) {
    fm_frame_assign(result, argv[args - 1]);
    comp_cl->updated = false;
    return true;
  }
  return false;
}

fm_call_def *fm_comp_filter_unless_stream_call(fm_comp_def_cl comp_cl,
                                               const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_filter_unless_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_filter_unless_stream_exec);
  return def;
}

void fm_comp_filter_unless_queuer(size_t idx, fm_call_ctx_t *ctx) {
  auto *comp_cl = (filter_unless_comp_cl *)ctx->comp;
  if (idx == 1) {
    comp_cl->updated = true;
  }
}

fm_ctx_def_t *fm_comp_filter_unless_gen(fm_comp_sys_t *csys,
                                        fm_comp_def_cl closure, unsigned argc,
                                        fm_type_decl_cp argv[],
                                        fm_type_decl_cp ptype,
                                        fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  if (argc != 2) {
    auto *errstr = "expect one or two operator arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect no "
                           "parameters");
    return nullptr;
  }

  auto ctx_cl = make_unique<filter_unless_comp_cl>();
  auto &calls = ctx_cl->calls;

  auto bool_param_t = fm_base_type_get(sys, FM_TYPE_BOOL);

  auto inp = argv[0];
  size_t nf = fm_type_frame_nfields(inp);

  for (size_t i = 0; i < nf; ++i) {
    if (!fm_type_equal(fm_type_frame_field_type(inp, i), bool_param_t)) {
      auto *errstr = "all fields must be of bool type";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
      return nullptr;
    }
  }

  for (size_t idx = 0; idx < nf; ++idx) {
    auto f_type = fm_type_frame_field_type(inp, idx);
    auto *call = new the_filter_unless_field_exec_2_0(idx);
    if (!call) {
      ostringstream os;
      auto *str = fm_type_to_str(f_type);
      os << "invalid type " << str;
      free(str);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, os.str().c_str());
      return nullptr;
    }
    calls.push_back(call);
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, argv[argc - 1]);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_queuer_set(def, &fm_comp_filter_unless_queuer);
  fm_ctx_def_stream_call_set(def, &fm_comp_filter_unless_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_filter_unless_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (filter_unless_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
