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
 * @file delta.hpp
 * @author Andres Rangel
 * @date 17 Dec 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "sample.hpp"

struct delta_field_exec {
  virtual ~delta_field_exec() {}
  virtual void init(fm_frame_t *result) = 0;
  virtual void exec(fm_frame_t *result, const fm_frame_t *const prev,
                    const fm_frame_t *const curr) = 0;
};

template <class T> struct the_delta_field_exec_2_0 : delta_field_exec {
  the_delta_field_exec_2_0(fm_field_t field) : field_(field) {}
  void init(fm_frame_t *result) override {
    *(T *)fm_frame_get_ptr1(result, field_, 0) = T(0);
  }
  void exec(fm_frame_t *result, const fm_frame_t *const prev,
            const fm_frame_t *const curr) override {
    auto val_curr = *(const T *)fm_frame_get_cptr1(curr, field_, 0);
    auto val_prev = *(const T *)fm_frame_get_cptr1(prev, field_, 0);
    *(T *)fm_frame_get_ptr1(result, field_, 0) = val_curr - val_prev;
  }
  fm_field_t field_;
};

template <class... Ts>
delta_field_exec *get_delta_field_exec(fmc::type_list<Ts...>,
                                       fm_type_decl_cp f_type, int idx) {
  delta_field_exec *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = new the_delta_field_exec_2_0<Tn>(idx);
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}

struct fm_comp_delta : fm_comp_sample_2_0 {
  fm_comp_delta(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
                fm_type_decl_cp argv[], fm_type_decl_cp ptype,
                fm_arg_stack_t plist)
      : fm_comp_sample_2_0(sys, closure, argc, argv, ptype, plist) {
    using supported_types =
        fmc::type_list<INT8, INT16, INT32, INT64, FLOAT32, FLOAT64>;

    auto inp = argv[0];
    int nf = fm_type_frame_nfields(inp);
    for (int idx = 0; idx < nf; ++idx) {
      auto f_type = fm_type_frame_field_type(inp, idx);
      auto *call = get_delta_field_exec(supported_types(), f_type, idx);
      ostringstream os;
      auto *str = fm_type_to_str(f_type);
      os << "type " << str << "is not supported in delta feature";
      free(str);
      fmc_runtime_error_unless(call) << os.str();
      calls_.push_back(call);
    }
  }
  ~fm_comp_delta() {
    for (auto *ptr : calls_) {
      delete ptr;
    }
  }
  bool init(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx) {
    auto *frames = fm_exec_ctx_frames(ctx->exec);
    auto type = fm_frame_type(argv[0]);
    sample_ = fm_frame_from_type(frames, type);
    fm_frame_reserve(sample_, fm_frame_dim(argv[0], 0));
    fm_frame_assign(sample_, argv[0]);

    for (auto &&call : calls_) {
      call->init(result);
    }

    return true;
  }
  bool exec(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx, bool interval, bool updated) {
    if (interval) {
      for (auto &&call : calls_) {
        call->exec(result, sample_, argv[0]);
      }
      fm_frame_assign(sample_, argv[0]);
    }
    return interval;
  }
  fm_frame_t *sample_ = nullptr;
  vector<delta_field_exec *> calls_;
};

bool fm_comp_delta_add(fm_comp_sys_t *sys) {
  return fm_comp_sample_add<fm_comp_delta>(sys, "delta");
}
