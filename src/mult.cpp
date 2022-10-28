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
 * @file mult.cpp
 * @author Maxim Trokhimtchouk
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "mult.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "extractor/decimal64.hpp"
#include "extractor/frame.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/time.hpp"

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

struct mult_field_exec {
  virtual ~mult_field_exec() {}
  virtual void exec(fm_frame_t *result, size_t,
                    const fm_frame_t *const argv[]) = 0;
};

template <class T> struct the_mult_field_exec_2_0 : mult_field_exec {
  the_mult_field_exec_2_0(fm_field_t field_one, fm_field_t field_two,
                          fm_field_t field_res)
      : field_one_(field_one), field_two_(field_two), field_res_(field_res) {}
  void exec(fm_frame_t *result, size_t,
            const fm_frame_t *const argv[]) override {
    auto val0 = *(const T *)fm_frame_get_cptr1(argv[0], field_one_, 0);
    auto val1 = *(const T *)fm_frame_get_cptr1(argv[1], field_two_, 0);
    *(T *)fm_frame_get_ptr1(result, field_res_, 0) = val1 * val0;
  }
  fm_field_t field_one_;
  fm_field_t field_two_;
  fm_field_t field_res_;
};

struct mult_comp_cl {
  ~mult_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<mult_field_exec *> calls;
};

bool fm_comp_mult_call_stream_init(fm_frame_t *result, size_t args,
                                   const fm_frame_t *const argv[],
                                   fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto &calls = (*(mult_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

bool fm_comp_mult_stream_exec(fm_frame_t *result, size_t args,
                              const fm_frame_t *const argv[],
                              fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &calls = (*(mult_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

fm_call_def *fm_comp_mult_stream_call(fm_comp_def_cl comp_cl,
                                      const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_mult_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_mult_stream_exec);
  return def;
}

template <class... Ts>
mult_field_exec *get_mult_field_exec(fmc::type_list<Ts...>,
                                     fm_type_decl_cp f_type, int idx, int idx2,
                                     int idx3) {
  mult_field_exec *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = new the_mult_field_exec_2_0<Tn>(idx, idx2, idx3);
    }
  };

  (create(fmc::typify<Ts>()), ...);
  return result;
}

fm_ctx_def_t *fm_comp_mult_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
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

  int non_eq = 0;
  fm_type_decl_cp single = argv[1];
  fm_type_decl_cp multi = argv[0];

  if (fm_type_frame_nfields(argv[0]) == 1 ||
      fm_type_frame_nfields(argv[1]) == 1) {
    if (fm_type_frame_nfields(argv[1]) == 1) {
      single = argv[1];
      multi = argv[0];
      non_eq = 1;
    } else {
      single = argv[0];
      multi = argv[1];
      non_eq = -1;
    }
    auto type0 = fm_type_frame_field_type(single, 0);
    for (unsigned i = 0; i < fm_type_frame_nfields(multi); ++i) {
      auto type1 = fm_type_frame_field_type(multi, i);
      if (!fm_type_equal(type0, type1)) {
        auto *errstr = "the fields from the operators have different "
                       "types";
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
        return nullptr;
      }
    }
  } else if (!fm_type_equal(argv[0], argv[1])) {
    auto *errstr = "two operator arguments must be the same type or have a "
                   "single field";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto ctx_cl = make_unique<mult_comp_cl>();
  auto &calls = ctx_cl->calls;

  using supported_types =
      fmc::type_list<INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64,
                     FLOAT32, FLOAT64, DECIMAL128>;

  int nf = fm_type_frame_nfields(multi);
  auto f_type = fm_type_frame_field_type(single, 0);

  for (int idx = 0; idx < nf; ++idx) {

    mult_field_exec *call = nullptr;
    switch (non_eq) {
    case -1:
      call = get_mult_field_exec(supported_types(), f_type, 0, idx, idx);
      break;
    case 0:
      call = get_mult_field_exec(supported_types(), f_type, idx, idx, idx);
      break;
    case 1:
      call = get_mult_field_exec(supported_types(), f_type, idx, 0, idx);
      break;
    }

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
  fm_ctx_def_type_set(def, multi);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_mult_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_mult_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (mult_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
