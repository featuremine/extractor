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
 * @file min.cpp
 * @authors Leandro leon, Andres Rangel
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "min.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"

#include "extractor/comp_def.hpp"
#include "extractor/frame.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/fxpt128.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/rational64.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/time.hpp"

#include "fmc/time.h"
#include <deque>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>
using namespace std;

struct exec_cl {
  virtual bool exec(fm_frame_t *result, size_t,
                    const fm_frame_t *const argv[]) = 0;

  virtual void reset(fm_frame_t *result, size_t,
                     const fm_frame_t *const argv[]) = 0;

  virtual ~exec_cl(){};
};

template <class T> struct min_exec_cl : public exec_cl {
  min_exec_cl(fm_field_t field) : field_(field) {}

  bool exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[]) {
    T val = *(const T *)fm_frame_get_cptr1(argv[0], field_, 0);

    if (!(val >= curr_min_)) {
      curr_min_ = *(T *)fm_frame_get_ptr1(result, field_, 0) = val;
      return true;
    }
    return false;
  }

  void reset(fm_frame_t *result, size_t, const fm_frame_t *const argv[]) {
    T val = *(const T *)fm_frame_get_cptr1(argv[0], field_, 0);
    curr_min_ = *(T *)fm_frame_get_ptr1(result, field_, 0) = val;
  }

  fm_field_t field_;
  T curr_min_;
};

template <> struct min_exec_cl<double> : public exec_cl {
  min_exec_cl(fm_field_t field) : field_(field) {}

  bool exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[]) {
    double val = *(const double *)fm_frame_get_cptr1(argv[0], field_, 0);

    if (isnan(val))
      return false;

    if (!(val >= curr_min_)) {
      curr_min_ = *(double *)fm_frame_get_ptr1(result, field_, 0) = val;
      return true;
    }
    return false;
  }

  void reset(fm_frame_t *result, size_t, const fm_frame_t *const argv[]) {
    double val = *(const double *)fm_frame_get_cptr1(argv[0], field_, 0);
    curr_min_ = *(double *)fm_frame_get_ptr1(result, field_, 0) = val;
  }

  fm_field_t field_;
  double curr_min_;
};

template <> struct min_exec_cl<float> : public exec_cl {
  min_exec_cl(fm_field_t field) : field_(field) {}

  bool exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[]) {
    float val = *(const float *)fm_frame_get_cptr1(argv[0], field_, 0);

    if (isnan(val))
      return false;

    if (!(val >= curr_min_)) {
      curr_min_ = *(float *)fm_frame_get_ptr1(result, field_, 0) = val;
      return true;
    }
    return false;
  }

  void reset(fm_frame_t *result, size_t, const fm_frame_t *const argv[]) {
    float val = *(const float *)fm_frame_get_cptr1(argv[0], field_, 0);
    curr_min_ = *(float *)fm_frame_get_ptr1(result, field_, 0) = val;
  }

  fm_field_t field_;
  float curr_min_;
};

struct min_comp_cl {
  ~min_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<exec_cl *> calls;
  int64_t t = fmc_time64_end().value;
};

bool fm_comp_min_call_stream_init(fm_frame_t *result, size_t args,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto *closure = (min_comp_cl *)ctx->comp;
  for (auto *call : closure->calls)
    call->reset(result, args, argv);
  return true;
}

bool fm_comp_min_stream_exec(fm_frame_t *result, size_t args,
                             const fm_frame_t *const argv[], fm_call_ctx_t *ctx,
                             fm_call_exec_cl cl) {
  auto now = fm_stream_ctx_now((fm_stream_ctx_t *)ctx->exec);

  auto *closure = (min_comp_cl *)ctx->comp;

  fmc_time64_t scheduled;
  scheduled.value = closure->t;

  if (scheduled == now) {
    for (auto *call : closure->calls)
      call->reset(result, args, argv);
    return true;
  }

  bool res = false;
  for (auto *call : closure->calls)
    res = res || call->exec(result, args, argv);

  return res;
}

fm_call_def *fm_comp_min_stream_call(fm_comp_def_cl comp_cl,
                                     const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_min_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_min_stream_exec);
  return def;
}

void fm_comp_min_queuer(size_t idx, fm_call_ctx_t *ctx) {
  int64_t &value = ((min_comp_cl *)(ctx->comp))->t;
  fmc_time64_t prev;
  prev.value = value;
  auto now = fm_stream_ctx_now((fm_stream_ctx_t *)ctx->exec);
  auto next = idx == 1 ? now : prev;
  value = next.value;
}

template <class... Ts>
exec_cl *get_comp_min_cl(fmc::type_list<Ts...>, fm_type_decl_cp f_type,
                         int idx) {
  exec_cl *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = new min_exec_cl<Tn>(idx);
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}

fm_ctx_def_t *fm_comp_min_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                              unsigned argc, fm_type_decl_cp argv[],
                              fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 2) {
    auto *errstr = "expect two operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect no "
                           "parameters");
    return nullptr;
  }

  using supported_types =
      fmc::type_list<INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64,
                     FLOAT32, FLOAT64, RPRICE, FIXEDPOINT128, DECIMAL128, TIME64, RATIONAL64>;

  auto ctx_cl = make_unique<min_comp_cl>();
  auto &calls = ctx_cl->calls;
  auto inp = argv[0];
  int nf = fm_type_frame_nfields(inp);

  for (int idx = 0; idx < nf; ++idx) {
    auto f_type = fm_type_frame_field_type(inp, idx);
    auto *call = get_comp_min_cl(supported_types(), f_type, idx);
    if (call == nullptr) {
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
  fm_ctx_def_type_set(def, argv[0]);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_queuer_set(def, &fm_comp_min_queuer);
  fm_ctx_def_stream_call_set(def, &fm_comp_min_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);

  return def;
}

void fm_comp_min_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (min_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
