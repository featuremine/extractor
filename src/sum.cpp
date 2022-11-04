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
 * @file sum.cpp
 * @author Maxim Trokhimtchouk
 * @date 17 Dec 2018
 * @brief File contains C definitions of the sum operator
 *
 * This file contains definitions of the sum operator
 * @see http://www.featuremine.com
 */

extern "C" {

#include "sum.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "extractor/comp_def.hpp"
#include "fmc++/rprice.hpp"
#include "extractor/frame.hpp"
#include "extractor/rational64.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/time.hpp"

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

struct sum_field_exec {
  virtual ~sum_field_exec() {}
  virtual void init(fm_frame_t *result, size_t,
                    const fm_frame_t *const argv[]) = 0;
  virtual void exec(fm_frame_t *result, fm_frame_t *o_val,
                    const fm_frame_t *n_val) = 0;
};

template <class T> struct the_sum_field_exec_2_0 : sum_field_exec {
  the_sum_field_exec_2_0(fm_field_t field) : field_(field) {}
  void init(fm_frame_t *result, size_t argc,
            const fm_frame_t *const argv[]) override {
    T val = T();
    for (unsigned i = 0; i < argc; ++i) {
      T curr_val = *(const T *)fm_frame_get_cptr1(argv[i], field_, 0);
      if constexpr (is_floating_point_v<T> || is_same_v<T, fm_rational64_t>) {
        if (!isnan(curr_val))
          val = val + curr_val;
      } else {
        val = val + curr_val;
      }
    }
    *(T *)fm_frame_get_ptr1(result, field_, 0) = val;
  }
  void exec(fm_frame_t *result, fm_frame_t *o_val,
            const fm_frame_t *n_val) override {
    auto val_old = *(const T *)fm_frame_get_cptr1(o_val, field_, 0);
    auto val_new = *(const T *)fm_frame_get_cptr1(n_val, field_, 0);
    auto val0 = *(const T *)fm_frame_get_cptr1(result, field_, 0);
    if constexpr (is_floating_point_v<T> || is_same_v<T, fm_rational64_t>) {
      if (!isnan(val_old))
        val0 = val0 - val_old;
      if (!isnan(val_new))
        val0 = val0 + val_new;
    } else {
      val0 = val0 - val_old;
      val0 = val0 + val_new;
    }
    *(T *)fm_frame_get_ptr1(result, field_, 0) = val0;
    *(T *)fm_frame_get_ptr1(o_val, field_, 0) = val_new;
  }
  fm_field_t field_;
};

template <> struct the_sum_field_exec_2_0<fmc_rprice_t> : sum_field_exec {
  the_sum_field_exec_2_0(fm_field_t field) : field_(field) {}
  void init(fm_frame_t *result, size_t argc,
            const fm_frame_t *const argv[]) override {
    fmc_rprice_t val = fmc_rprice_t();
    for (unsigned i = 0; i < argc; ++i) {
      val =
          val + *(const fmc_rprice_t *)fm_frame_get_cptr1(argv[i], field_, 0);
    }
    *(fmc_rprice_t *)fm_frame_get_ptr1(result, field_, 0) = val;
  }
  void exec(fm_frame_t *result, fm_frame_t *o_val,
            const fm_frame_t *n_val) override {
    auto val_old =
        *(const fmc_rprice_t *)fm_frame_get_cptr1(o_val, field_, 0);
    auto val_new =
        *(const fmc_rprice_t *)fm_frame_get_cptr1(n_val, field_, 0);
    auto val0 = *(const fmc_rprice_t *)fm_frame_get_cptr1(result, field_, 0);
    *(fmc_rprice_t *)fm_frame_get_ptr1(result, field_, 0) =
        val0 - val_old + val_new;
    *(fmc_rprice_t *)fm_frame_get_ptr1(o_val, field_, 0) = val_new;
  }
  fm_field_t field_;
};

template <> struct the_sum_field_exec_2_0<fmc_time64_t> : sum_field_exec {
  the_sum_field_exec_2_0(fm_field_t field) : field_(field) {}
  void init(fm_frame_t *result, size_t argc,
            const fm_frame_t *const argv[]) override {
    fmc_time64_t val = fmc_time64_t();
    for (unsigned i = 0; i < argc; ++i) {
      val = val + *(const fmc_time64_t *)fm_frame_get_cptr1(argv[i], field_, 0);
    }
    *(fmc_time64_t *)fm_frame_get_ptr1(result, field_, 0) = val;
  }
  void exec(fm_frame_t *result, fm_frame_t *o_val,
            const fm_frame_t *n_val) override {
    auto val_old = *(const fmc_time64_t *)fm_frame_get_cptr1(o_val, field_, 0);
    auto val_new = *(const fmc_time64_t *)fm_frame_get_cptr1(n_val, field_, 0);
    auto val0 = *(const fmc_time64_t *)fm_frame_get_cptr1(result, field_, 0);
    *(fmc_time64_t *)fm_frame_get_ptr1(result, field_, 0) =
        val0 - val_old + val_new;
    *(fmc_time64_t *)fm_frame_get_ptr1(o_val, field_, 0) = val_new;
  }
  fm_field_t field_;
};

struct sum_comp_cl {
  ~sum_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<sum_field_exec *> calls;
  vector<pair<const fm_frame_t *, fm_frame_t *>> elems_;
  fm_frame_t *buffer_ = nullptr;
};

bool fm_comp_sum_call_stream_init(fm_frame_t *result, size_t args,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto *info = (sum_comp_cl *)ctx->comp;
  auto *frames = fm_exec_ctx_frames((fm_exec_ctx *)ctx->exec);
  auto type = fm_frame_type(result);
  for (unsigned i = 0; i < args; ++i) {
    auto *frame = fm_frame_from_type(frames, type);
    if (!frame)
      return false;
    fm_frame_assign(frame, argv[i]);
    info->elems_.emplace_back(argv[i], frame);
  }

  for (auto &call : info->calls) {
    call->init(result, args, argv);
  }
  auto *frame = fm_frame_from_type(frames, type);
  if (!frame)
    return false;
  fm_frame_assign(frame, result);
  info->buffer_ = frame;

  return true;
}

bool fm_comp_sum_stream_exec(fm_frame_t *result, size_t args,
                             const fm_frame_t *const argv[], fm_call_ctx_t *ctx,
                             fm_call_exec_cl cl) {
  auto *info = (sum_comp_cl *)ctx->comp;
  fm_frame_assign(result, info->buffer_);
  return true;
}

void fm_comp_sum_queuer(size_t idx, fm_call_ctx_t *ctx) {
  auto *info = (sum_comp_cl *)ctx->comp;
  auto &&[n_val, o_val] = info->elems_[idx];
  for (auto &call : info->calls) {
    call->exec(info->buffer_, o_val, n_val);
  }
}

fm_call_def *fm_comp_sum_stream_call(fm_comp_def_cl comp_cl,
                                     const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_sum_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_sum_stream_exec);
  return def;
}

template <class... Ts>
sum_field_exec *get_sum_field_exec(fmc::type_list<Ts...>,
                                   fm_type_decl_cp f_type, int idx) {
  sum_field_exec *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = new the_sum_field_exec_2_0<Tn>(idx);
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}

fm_ctx_def_t *fm_comp_sum_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                              unsigned argc, fm_type_decl_cp argv[],
                              fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  if (argc < 1) {
    auto *errstr = "expect one or more operator arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect no "
                           "parameters");
    return nullptr;
  }

  auto error = [&]() {
    auto *errstr = "two operator arguments must be the same type or have a "
                   "single field of same type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  };

  if (fm_type_frame_nfields(argv[0]) == 1) {
    auto type0 = fm_type_frame_field_type(argv[0], 0);
    for (unsigned i = 1; i < argc; ++i) {
      if (fm_type_frame_nfields(argv[i]) != 1 ||
          !fm_type_equal(type0, fm_type_frame_field_type(argv[i], 0))) {
        return error();
      }
    }
  } else {
    for (unsigned i = 1; i < argc; ++i) {
      if (!fm_type_equal(argv[0], argv[i])) {
        return error();
      }
    }
  }

  auto ctx_cl = make_unique<sum_comp_cl>();
  auto &calls = ctx_cl->calls;

  using supported_types =
      fmc::type_list<INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64,
                     FLOAT32, FLOAT64, RPRICE, TIME64, RATIONAL64>;

  auto inp = argv[0];
  int nf = fm_type_frame_nfields(inp);
  for (int idx = 0; idx < nf; ++idx) {
    auto f_type = fm_type_frame_field_type(inp, idx);
    auto *call = get_sum_field_exec(supported_types(), f_type, idx);
    if (!call) {
      ostringstream os;
      auto *str = fm_type_to_str(f_type);
      os << "type " << str << "is not supported in sum feature";
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
  fm_ctx_def_stream_call_set(def, &fm_comp_sum_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  fm_ctx_def_queuer_set(def, &fm_comp_sum_queuer);
  return def;
}

void fm_comp_sum_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (sum_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
