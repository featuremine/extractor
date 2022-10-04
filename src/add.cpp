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
 * @file add.cpp
 * @author Leandro Leon
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "add.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include "extractor/decimal64.hpp"
#include "extractor/frame.hpp"
#include "extractor/comp_def.hpp"
#include "extractor/rational64.hpp"
#include "extractor/time64.hpp"
#include <fmc++/mpl.hpp>

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

struct add_field_exec {
  virtual ~add_field_exec() {}
  virtual void exec(fm_frame_t *result, size_t,
                    const fm_frame_t *const argv[]) = 0;
};

template <class T> struct the_add_field_exec_2_0 : add_field_exec {
  the_add_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t,
            const fm_frame_t *const argv[]) override {
    auto val0 = *(const T *)fm_frame_get_cptr1(argv[0], field_, 0);
    auto val1 = *(const T *)fm_frame_get_cptr1(argv[1], field_, 0);
    *(T *)fm_frame_get_ptr1(result, field_, 0) = val0 + val1;
  }
  fm_field_t field_;
};

struct add_comp_cl {
  ~add_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<add_field_exec *> calls;
};

bool fm_comp_add_call_stream_init(fm_frame_t *result, size_t args,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto &calls = (*(add_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

bool fm_comp_add_stream_exec(fm_frame_t *result, size_t args,
                             const fm_frame_t *const argv[], fm_call_ctx_t *ctx,
                             fm_call_exec_cl cl) {
  auto &calls = (*(add_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

fm_call_def *fm_comp_add_stream_call(fm_comp_def_cl comp_cl,
                                     const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_add_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_add_stream_exec);
  return def;
}

template <class... Ts>
add_field_exec *get_add_field_exec(fmc::type_list<Ts...>,
                                   fm_type_decl_cp f_type, int idx) {
  add_field_exec *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = new the_add_field_exec_2_0<Tn>(idx);
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}

fm_ctx_def_t *fm_comp_add_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                              unsigned argc, fm_type_decl_cp argv[],
                              fm_type_decl_cp ptype, fm_arg_stack_t plist) {

  auto *sys = fm_type_sys_get(csys);

  if (argc != 2) {
    auto *errstr = "expect two operator arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect no "
                           "parameters");
    return nullptr;
  }

  if (fm_type_frame_nfields(argv[0]) == 1 &&
      fm_type_frame_nfields(argv[1]) == 1) {
    auto type0 = fm_type_frame_field_type(argv[0], 0);
    auto type1 = fm_type_frame_field_type(argv[1], 0);
    if (!fm_type_equal(type0, type1)) {
      auto *errstr = "the two fields have different types";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
      return nullptr;
    }
  } else if (!fm_type_equal(argv[0], argv[1])) {
    auto *errstr = "two operator arguments must be the same type or have a "
                   "single field of same type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto ctx_cl = make_unique<add_comp_cl>();
  auto &calls = ctx_cl->calls;

  using supported_types =
      fmc::type_list<INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64,
                     FLOAT32, FLOAT64, DECIMAL64, TIME64, RATIONAL64>;

  auto inp = argv[0];
  int nf = fm_type_frame_nfields(inp);
  for (int idx = 0; idx < nf; ++idx) {
    auto f_type = fm_type_frame_field_type(inp, idx);
    auto *call = get_add_field_exec(supported_types(), f_type, idx);
    if (!call) {
      ostringstream os;
      auto *str = fm_type_to_str(f_type);
      os << "type " << str << "is not supported in add feature";
      free(str);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, os.str().c_str());
      return nullptr;
    }
    calls.push_back(call);
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, argv[0]);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_add_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_add_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (add_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
