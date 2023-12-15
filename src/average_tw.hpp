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
 * @file average_tw.cpp
 * @authors Andres Rangel
 * @date 27 Sep 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "sample.hpp"
#include "upcast_util.hpp"

#include <limits>
#include <vector>

#include "fmc++/decimal128.hpp"
#include "fmc++/fxpt128.hpp"

struct exec_cl {
  virtual void exec(fmc_time64_t t_d) = 0;
  virtual void set(fm_frame_t *result) = 0;
  virtual void reset(const fm_frame_t *argv) = 0;
  virtual ~exec_cl(){};
};

template <template <class> class C, class... Ts>
exec_cl *get_comp_cl(fmc::type_list<Ts...>, fm_type_decl_cp f_type, int idx) {
  exec_cl *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = new C<Tn>(idx);
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}

template <template <class> class C, class... Ts>
fm_type_decl_cp get_comp_type(fmc::type_list<Ts...>, fm_type_sys_t *sys,
                              fm_type_decl_cp f_type) {
  fm_type_decl_cp result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = fm::frame_field_type<typename C<Tn>::result>().fm_type(sys);
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}

template <template <class> class Comp> struct fm_comp_tw : fm_comp_sample_2_0 {
  fm_comp_tw(fm_comp_sys_t *csys, fm_comp_def_cl closure, unsigned argc,
             fm_type_decl_cp argv[], fm_type_decl_cp ptype,
             fm_arg_stack_t plist)
      : fm_comp_sample_2_0(csys, closure, argc, argv, ptype, plist),
        last_time_(fmc_time64_start()) {
    using supported_types = fmc::type_list<FLOAT32, FLOAT64, DECIMAL128, fmc::fxpt128>;

    int nf = fm_type_frame_nfields(argv[0]);
    vector<const char *> names(nf);
    vector<fm_type_decl_cp> types(nf);
    int nd = 1;
    int dims[1] = {1};
    auto *sys = fm_type_sys_get(csys);

    for (int idx = 0; idx < nf; ++idx) {
      auto f_type = fm_type_frame_field_type(argv[0], idx);
      auto *call = get_comp_cl<Comp>(supported_types(), f_type, idx);
      types[idx] = get_comp_type<Comp>(supported_types(), sys, f_type);
      names[idx] = fm_type_frame_field_name(argv[0], idx);

      auto *str = fm_type_to_str(f_type);
      string type_str = str;
      free(str);
      fmc_runtime_error_unless(call) << "invalid type " << type_str;
      calls.push_back(call);
    }

    ret_type =
        fm_frame_type_get1(sys, nf, names.data(), types.data(), nd, dims);
  }
  ~fm_comp_tw() {
    for (auto &&call : calls) {
      delete call;
    }
  }
  fm_type_decl_cp result_type(fm_type_sys_t *sys, fm_type_decl_cp arg) {
    return ret_type;
  }
  bool init(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx) {
    for (auto *call : calls)
      call->reset(argv[0]);

    return true;
  }
  bool exec(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx, bool interval, bool updated) {
    auto now = fm_stream_ctx_now((fm_stream_ctx_t *)ctx->exec);
    auto delta =
        last_time_ == fmc_time64_start() ? fmc_time64_end() : now - last_time_;
    if (interval) {
      for (auto *call : calls) {
        call->exec(delta);
        call->set(result);
        call->reset(argv[0]);
      }
      last_time_ = now;
    } else {
      for (auto *call : calls) {
        call->exec(delta);
        call->reset(argv[0]);
      }
      last_time_ = now;
    }

    return interval;
  }
  vector<exec_cl *> calls;
  fmc_time64_t last_time_;
  fm_type_decl_cp ret_type = nullptr;
};

template <class T> struct average_tw_exec_cl : public exec_cl {
  using result = T;
  using S = typename upcast<T>::type;
  average_tw_exec_cl(fm_field_t field)
      : field_(field), last_val_(), num_(), denom_() {}
  void exec(fmc_time64_t t_d) override {
    if (t_d == fmc_time64_end()) {
      if (!isnan(last_val_)) {
        denom_ = t_d;
      }
      return;
    }

    if (!isnan(last_val_) && denom_ != fmc_time64_end()) {
      num_ += last_val_ * S(fmc_time64_raw(t_d));
      denom_ += t_d;
    }
  }
  void set(fm_frame_t *result) override {
    if (denom_ == fmc_time64_from_raw(0) || denom_ == fmc_time64_end()) {
      *(T *)fm_frame_get_ptr1(result, field_, 0) = last_val_;
    } else {
      *(T *)fm_frame_get_ptr1(result, field_, 0) =
          num_ / S(fmc_time64_raw(denom_));
    }
    num_ = S(0);
    denom_ = fmc_time64_from_raw(0);
  }
  void reset(const fm_frame_t *argv) override {
    if (denom_ != fmc_time64_end()) {
      last_val_ = *(const T *)fm_frame_get_cptr1(argv, field_, 0);
    }
  }
  fm_field_t field_;
  S last_val_;
  S num_;
  fmc_time64_t denom_;
};

template <class T> struct elapsed_exec_cl : public exec_cl {
  using result = fmc_time64_t;
  using S = typename upcast<T>::type;
  elapsed_exec_cl(fm_field_t field)
      : field_(field), last_val_(), denom_() {}
  void exec(fmc_time64_t t_d) override {
    if (t_d == fmc_time64_end()) {
      if (!isnan(last_val_)) {
        denom_ = t_d;
      }
      return;
    }

    if (!isnan(last_val_) && denom_ != fmc_time64_end()) {
      denom_ += t_d;
    }
  }
  void set(fm_frame_t *result) override {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, field_, 0) = denom_;
    denom_ = fmc_time64_from_raw(0);
  }
  void reset(const fm_frame_t *argv) override {
    if (denom_ != fmc_time64_end()) {
      last_val_ = *(const T *)fm_frame_get_cptr1(argv, field_, 0);
    }
  }
  fm_field_t field_;
  S last_val_;
  fmc_time64_t denom_;
};

template <class T> struct sum_tw_exec_cl : public exec_cl {
  using result = T;
  using S = typename upcast<T>::type;
  sum_tw_exec_cl(fm_field_t field) : field_(field), last_val_(), num_() {}
  void exec(fmc_time64_t t_d) override {
    if (t_d == fmc_time64_end()) {
      if (!isnan(last_val_)) {
        if (last_val_ > std::numeric_limits<S>::epsilon()) {
          num_ = numeric_limits<S>::infinity();
        } else if (last_val_ < -std::numeric_limits<S>::epsilon()) {
          num_ = -numeric_limits<S>::infinity();
        } else {
          num_ = S();
        }
      }
      return;
    }

    if (!isnan(last_val_) && isfinite(num_)) {
      num_ += last_val_ * S(fmc_time64_to_fseconds(t_d));
    }
  }
  void set(fm_frame_t *result) override {
    *(T *)fm_frame_get_ptr1(result, field_, 0) = num_;
    num_ = S();
  }
  void reset(const fm_frame_t *argv) override {
    last_val_ = *(const T *)fm_frame_get_cptr1(argv, field_, 0);
  }
  fm_field_t field_;
  S last_val_;
  S num_;
};

bool fm_comp_average_tw_add(fm_comp_sys_t *sys) {
  return fm_comp_sample_add<fm_comp_tw<average_tw_exec_cl>>(sys, "average_"
                                                                 "tw") &&
         fm_comp_sample_add<fm_comp_tw<elapsed_exec_cl>>(sys, "elapsed") &&
         fm_comp_sample_add<fm_comp_tw<sum_tw_exec_cl>>(sys, "sum_tw");
}
