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
 * @file round.cpp
 * @author Andres Rangel
 * @date 10 May 2019
 * @brief File contains C++ definitions of the round computation
 *
 * This file contains definitions of the round computation
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
#include "roundop.h"
}

#include "extractor/comp_def.hpp"
#include "extractor/frame.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/time.hpp"

#include <cmath>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

struct round_field_exec {
  virtual ~round_field_exec() {}
  virtual void exec(fm_frame_t *result, size_t,
                    const fm_frame_t *const argv[]) = 0;
};

template <class D, class T> struct the_round_field_exec_2_0;

template <class T>
struct the_round_field_exec_2_0<fmc_rprice_t, T> : round_field_exec {
  the_round_field_exec_2_0(fm_field_t field, int64_t divisor)
      : field_(field), divisor_(divisor),
        factor_(FMC_RPRICE_FRACTION / divisor_) {}
  void exec(fm_frame_t *result, size_t,
            const fm_frame_t *const argv[]) override {
    const T &val0 = *(const T *)fm_frame_get_cptr1(argv[0], field_, 0);
    fmc_rprice_from_raw((fmc_rprice_t *)fm_frame_get_ptr1(result, field_, 0),
                        llround(val0 * divisor_) * factor_);
  }
  fm_field_t field_;
  int64_t divisor_;
  int64_t factor_;
};

template <class T>
struct the_round_field_exec_2_0<fmc_decimal128_t, T> : round_field_exec {
  the_round_field_exec_2_0(fm_field_t field, int64_t divisor)
  : field_(field), divisor_(divisor),
    factor_(1 / fmc::decimal128(divisor_)) {
  }
  void exec(fm_frame_t *result, size_t,
            const fm_frame_t *const argv[]) override {
    const T &val0 = *(const T *)fm_frame_get_cptr1(argv[0], field_, 0);
    fmc_decimal128_t &res = *(fmc_decimal128_t *)fm_frame_get_ptr1(result, field_, 0);
    fmc_decimal128_from_int(&res, llround(val0 * divisor_));
    res = res * factor_;
  }
  fm_field_t field_;
  int64_t divisor_;
  fmc_decimal128_t factor_;
};

template <class T>
struct the_round_field_exec_2_0<int64_t, T> : round_field_exec {
  the_round_field_exec_2_0(fm_field_t field, int64_t divisor) : field_(field) {}
  void exec(fm_frame_t *result, size_t,
            const fm_frame_t *const argv[]) override {
    const T &val0 = *(const T *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(int64_t *)fm_frame_get_ptr1(result, field_, 0) =
        static_cast<int64_t>(llround(val0));
  }
  fm_field_t field_;
};

template <>
struct the_round_field_exec_2_0<int64_t, fmc_rprice_t> : round_field_exec {
  the_round_field_exec_2_0(fm_field_t field, int64_t divisor) : field_(field) {}
  void exec(fm_frame_t *result, size_t,
            const fm_frame_t *const argv[]) override {
    fmc_rprice_round(
        (int64_t *)fm_frame_get_ptr1(result, field_, 0),
        (const fmc_rprice_t *)fm_frame_get_cptr1(argv[0], field_, 0));
  }
  fm_field_t field_;
};

template <>
struct the_round_field_exec_2_0<int64_t, fmc_decimal128_t> : round_field_exec {
  the_round_field_exec_2_0(fm_field_t field, int64_t divisor) : field_(field) {}
  void exec(fm_frame_t *result, size_t,
            const fm_frame_t *const argv[]) override {
    const fmc_decimal128_t &val0 =
        *(const fmc_decimal128_t *)fm_frame_get_cptr1(argv[0], field_, 0);
    // Do we need to explicitly round?
    fmc_decimal128_t ret;
    fmc_decimal128_round(&ret, &val0);
    fmc_error_t *err;
    fmc_decimal128_to_int((int64_t *)fm_frame_get_ptr1(result, field_, 0), &ret,
                          &err);
  }
  fm_field_t field_;
};

struct round_comp_cl {
  ~round_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<round_field_exec *> calls;
};

bool fm_comp_round_call_stream_init(fm_frame_t *result, size_t args,
                                    const fm_frame_t *const argv[],
                                    fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto &calls = (*(round_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

bool fm_comp_round_stream_exec(fm_frame_t *result, size_t args,
                               const fm_frame_t *const argv[],
                               fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &calls = (*(round_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

fm_call_def *fm_comp_round_stream_call(fm_comp_def_cl comp_cl,
                                       const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_round_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_round_stream_exec);
  return def;
}

template <class D, class... Ts>
round_field_exec *get_round_field_exec(fmc::type_list<Ts...>,
                                       fm_type_decl_cp f_type, int idx,
                                       int64_t divisor) {
  round_field_exec *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = new the_round_field_exec_2_0<D, Tn>(idx, divisor);
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}

fm_ctx_def_t *fm_comp_round_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                unsigned argc, fm_type_decl_cp argv[],
                                fm_type_decl_cp ptype, fm_arg_stack_t plist) {

  auto *sys = fm_type_sys_get(csys);

  if (argc != 1) {
    auto *errstr = "expecting one operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto error = [&]() {
    const char *errstr = "expecting either no agruments or the desired divisor "
                         "as an integer argument and optionally the result type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!ptype)
    return error();

  if (!fm_type_is_tuple(ptype))
    return error();

  if (fm_type_tuple_size(ptype) == 1 || fm_type_tuple_size(ptype) == 2) {
    uint64_t divisor;

    if (!fm_arg_try_uinteger(fm_type_tuple_arg(ptype, 0), &plist, &divisor))
      return error();

    if (divisor == 0) {
      auto errstr = string("provided divisor must be a positive number");
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr.c_str());
      return nullptr;
    }
    
    bool is64 = true;

    if (fm_type_tuple_size(ptype) == 2) {
      auto *restype = fm_arg_try_type_decl(fm_type_tuple_arg(ptype, 1), &plist);
      if (!restype)
        return error();
      is64 = fm_type_equal(restype, fm_base_type_get(sys, FM_TYPE_RPRICE));
      if (!is64 && !fm_type_equal(restype, fm_base_type_get(sys, FM_TYPE_DECIMAL128))) {
        auto *errstr = "only Decimal64 and Decimal128 types are supported";
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
        return nullptr;
      }
    }

    if (is64 && FMC_RPRICE_FRACTION % divisor != 0) {
      auto errstr = string("provided divisor must be a divisor for ") +
                    to_string(FMC_RPRICE_FRACTION);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr.c_str());
      return nullptr;
    }

    auto ctx_cl = make_unique<round_comp_cl>();
    auto &calls = ctx_cl->calls;

    using supported_types = fmc::type_list<FLOAT32, FLOAT64>;

    auto inp = argv[0];
    int nf = fm_type_frame_nfields(inp);
    int nd = fm_type_frame_ndims(inp);

    vector<const char *> names(nf);
    vector<fm_type_decl_cp> types(nf);
    vector<int> dims(nd);

    for (int idx = 0; idx < nf; ++idx) {
      dims[idx] = fm_type_frame_dim(inp, idx);
    }

    auto decimal_param_t = is64 ? fm_base_type_get(sys, FM_TYPE_RPRICE)
                                : fm_base_type_get(sys, FM_TYPE_DECIMAL128);

    for (int idx = 0; idx < nf; ++idx) {
      names[idx] = fm_type_frame_field_name(argv[0], idx);
      types[idx] = decimal_param_t;

      auto f_type = fm_type_frame_field_type(inp, idx);
      round_field_exec *call = is64
        ? get_round_field_exec<fmc_rprice_t>(supported_types(), f_type, idx, divisor)
        : get_round_field_exec<fmc_decimal128_t>(supported_types(), f_type, idx, divisor);
      if (!call) {
        ostringstream os;
        auto *str = fm_type_to_str(f_type);
        os << "type " << str << "is not supported in round feature";
        free(str);
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, os.str().c_str());
        return nullptr;
      }
      calls.push_back(call);
    }

    auto type = fm_frame_type_get1(sys, nf, names.data(), types.data(), nd,
                                   dims.data());
    if (!type) {
      auto *errstr = "unable to create result frame type";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
      return nullptr;
    }

    auto *def = fm_ctx_def_new();
    fm_ctx_def_inplace_set(def, false);
    fm_ctx_def_type_set(def, type);
    fm_ctx_def_closure_set(def, ctx_cl.release());
    fm_ctx_def_stream_call_set(def, &fm_comp_round_stream_call);
    fm_ctx_def_query_call_set(def, nullptr);
    return def;
  } else if (fm_type_tuple_size(ptype) == 0) {
    auto ctx_cl = make_unique<round_comp_cl>();
    auto &calls = ctx_cl->calls;

    using supported_types =
        fmc::type_list<FLOAT32, FLOAT64, RPRICE, DECIMAL128>;

    auto inp = argv[0];
    int nf = fm_type_frame_nfields(inp);
    int nd = fm_type_frame_ndims(inp);

    vector<const char *> names(nf);
    vector<fm_type_decl_cp> types(nf);
    vector<int> dims(nd);

    for (int idx = 0; idx < nf; ++idx) {
      dims[idx] = fm_type_frame_dim(inp, idx);
    }

    auto int64_param_t = fm_base_type_get(sys, FM_TYPE_INT64);

    for (int idx = 0; idx < nf; ++idx) {
      names[idx] = fm_type_frame_field_name(argv[0], idx);
      types[idx] = int64_param_t;

      auto f_type = fm_type_frame_field_type(inp, idx);
      uint64_t divisor = 0;
      round_field_exec *call = get_round_field_exec<int64_t>(
          supported_types(), f_type, idx, divisor);
      if (!call) {
        ostringstream os;
        auto *str = fm_type_to_str(f_type);
        os << "type " << str << "is not supported in round feature";
        free(str);
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, os.str().c_str());
        return nullptr;
      }
      calls.push_back(call);
    }

    auto type = fm_frame_type_get1(sys, nf, names.data(), types.data(), nd,
                                   dims.data());
    if (!type) {
      auto *errstr = "unable to create result frame type";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
      return nullptr;
    }

    auto *def = fm_ctx_def_new();
    fm_ctx_def_inplace_set(def, false);
    fm_ctx_def_type_set(def, type);
    fm_ctx_def_closure_set(def, ctx_cl.release());
    fm_ctx_def_stream_call_set(def, &fm_comp_round_stream_call);
    fm_ctx_def_query_call_set(def, nullptr);
    return def;
  } else {
    return error();
  }
}

void fm_comp_round_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (round_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
