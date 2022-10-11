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
 * @file window_util.hpp
 * @author Andres Rangel
 * @date 24 Dec 2018
 * @brief Window based operator utilities
 *
 * This file defines some window based operator utilities
 */

#pragma once

extern "C" {
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
#include "extractor/type_sys.h"
}

#include "field_util.hpp"
#include "sample.hpp"
#include <deque>
#include <vector>

struct field_exec_cl {
  virtual void init(const fm_frame_t *argv, fm_frame_t *result) = 0;
  virtual void push(const fm_frame_t *argv, fm_frame_t *result) = 0;
  virtual void pop(fm_frame_t *result) = 0;
  virtual ~field_exec_cl(){};
};

struct base_exp_field_exec_cl {
  virtual void init(const fm_frame_t *input, fm_frame_t *result) = 0;
  virtual void set(const fm_frame_t *input, const fmc_time64_t &now) = 0;
  virtual void asof(const fm_frame_t *input, fm_frame_t *result,
                    const fmc_time64_t &now) = 0;
  virtual ~base_exp_field_exec_cl(){};
};

template <template <class> class Comp>
struct fm_comp_tick_window : fm_comp_identity_result {
  fm_comp_tick_window(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
                      fm_type_decl_cp argv[], fm_type_decl_cp ptype,
                      fm_arg_stack_t plist)
      : curr_(0) {
    fmc_runtime_error_unless(argc == 1) << "expect single operator as input";

    fmc_runtime_error_unless(
        fm_arg_try_uinteger(fm_type_tuple_arg(ptype, 0), &plist, &len_))
        << "expect an unsigned integer as window length parameter";

    using supported_types = fmc::type_list<FLOAT32, FLOAT64>;

    int nf = fm_type_frame_nfields(argv[0]);

    for (int idx = 0; idx < nf; ++idx) {
      auto f_type = fm_type_frame_field_type(argv[0], idx);
      auto *call = get_field_exec_cl<field_exec_cl, Comp>(supported_types(),
                                                          f_type, idx);
      auto *str = fm_type_to_str(f_type);
      string type_str = str;
      free(str);
      fmc_runtime_error_unless(call) << "invalid type " << type_str;
      calls.push_back(call);
    }
  }
  ~fm_comp_tick_window() {
    for (auto &&call : calls) {
      delete call;
    }
  }
  bool init(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx) {
    for (auto *call : calls) {
      call->init(argv[0], result);
    }

    return true;
  }
  bool exec(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx, bool interval, bool updated) {
    ++curr_;

    if (curr_ > len_) {
      --curr_;
      for (auto *call : calls) {
        call->pop(result);
      }
    }

    for (auto *call : calls) {
      call->push(argv[0], result);
    }

    return true;
  }
  uint64_t len_;
  uint64_t curr_;
  vector<field_exec_cl *> calls;
};

template <template <class> class Comp>
struct fm_comp_time_window : fm_comp_identity_result {
  fm_comp_time_window(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
                      fm_type_decl_cp argv[], fm_type_decl_cp ptype,
                      fm_arg_stack_t plist) {
    fmc_runtime_error_unless(argc == 1) << "expect single operator as input";

    fmc_runtime_error_unless(
        fm_arg_try_time64(fm_type_tuple_arg(ptype, 0), &plist, &window_size_))
        << "expect a time window length parameter";

    using supported_types = fmc::type_list<FLOAT32, FLOAT64>;

    int nf = fm_type_frame_nfields(argv[0]);

    for (int idx = 0; idx < nf; ++idx) {
      auto f_type = fm_type_frame_field_type(argv[0], idx);
      auto *call = get_field_exec_cl<field_exec_cl, Comp>(supported_types(),
                                                          f_type, idx);
      auto *str = fm_type_to_str(f_type);
      string type_str = str;
      free(str);
      fmc_runtime_error_unless(call) << "invalid type " << type_str;
      calls.push_back(call);
    }
  }
  ~fm_comp_time_window() {
    for (auto &&call : calls) {
      delete call;
    }
  }
  bool init(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx) {
    for (auto *call : calls) {
      call->init(argv[0], result);
    }

    return true;
  }
  bool exec(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx, bool interval, bool updated) {
    const fmc_time64_t now = fm_stream_ctx_now((fm_stream_ctx_t *)ctx->exec);

    while (!queue_.empty() && now >= queue_.front() + window_size_) {
      for (auto *call : calls) {
        call->pop(result);
      }
      queue_.pop_front();
    }

    if (updated) {
      for (auto *call : calls) {
        call->push(argv[0], result);
      }
      queue_.emplace_back(now);
    }

    if (!queue_.empty())
      fm_stream_ctx_schedule((fm_stream_ctx_t *)ctx->exec, ctx->handle,
                             queue_.front() + window_size_);

    return true;
  }
  fmc_time64_t window_size_;
  vector<field_exec_cl *> calls;
  std::deque<fmc_time64_t> queue_;
};

template <template <class> class Comp>
struct fm_comp_exp_window : fm_comp_identity_result {
  fm_comp_exp_window(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
                     fm_type_decl_cp argv[], fm_type_decl_cp ptype,
                     fm_arg_stack_t plist) {
    fmc_runtime_error_unless(argc == 2) << "expect two operators as input";

    fmc_time64_t window_size;

    fmc_runtime_error_unless(
        fm_arg_try_time64(fm_type_tuple_arg(ptype, 0), &plist, &window_size))
        << "expect a time window length parameter";

    using supported_types = fmc::type_list<FLOAT32, FLOAT64>;

    int nf = fm_type_frame_nfields(argv[0]);

    for (int idx = 0; idx < nf; ++idx) {
      auto f_type = fm_type_frame_field_type(argv[0], idx);
      auto *call = get_field_exec_cl<base_exp_field_exec_cl, Comp>(
          supported_types(), f_type, idx, window_size);
      auto *str = fm_type_to_str(f_type);
      string type_str = str;
      free(str);
      fmc_runtime_error_unless(call) << "invalid type " << type_str;
      calls.push_back(call);
    }
  }
  ~fm_comp_exp_window() {
    for (auto &&call : calls) {
      delete call;
    }
  }
  bool init(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx) {
    for (auto *call : calls) {
      call->init(argv[0], result);
    }

    return true;
  }
  bool exec(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx, bool interval, bool updated) {
    const fmc_time64_t now = fm_stream_ctx_now((fm_stream_ctx_t *)ctx->exec);

    if (updated) {
      for (auto *call : calls) {
        call->set(argv[0], now);
      }
    }

    if (interval) {
      for (auto *call : calls) {
        call->asof(argv[0], result, now);
      }
    }
    return interval;
  }
  vector<base_exp_field_exec_cl *> calls;
};

template <class T, template <class> class S>
struct queued_field_exec_cl : field_exec_cl {
  queued_field_exec_cl(fm_field_t field) : field_(field), count_(0) {}
  void init(const fm_frame_t *argv, fm_frame_t *result) {
    const T &val = *(const T *)fm_frame_get_cptr1(argv, field_, 0);
    *(T *)fm_frame_get_ptr1(result, field_, 0) = obj_.init(val);
  }
  void push(const fm_frame_t *argv, fm_frame_t *result) {
    const T &val = *(const T *)fm_frame_get_cptr1(argv, field_, 0);
    queue_.emplace_back(val);

    if (isnan(val))
      return;

    ++count_;

    *(T *)fm_frame_get_ptr1(result, field_, 0) = obj_.push(val, count_);
  }
  void pop(fm_frame_t *result) {
    const T val = queue_.front();
    queue_.pop_front();

    if (isnan(val))
      return;

    --count_;

    *(T *)fm_frame_get_ptr1(result, field_, 0) = obj_.pop(val, count_);
  }
  fm_field_t field_;
  uint64_t count_;
  std::deque<T> queue_;
  S<T> obj_;
};

template <class T, template <class> class S>
struct exp_field_exec_cl : base_exp_field_exec_cl {
  exp_field_exec_cl(fm_field_t field, fmc_time64_t size)
      : field_(field), prev_(fmc_time64_start()), obj_(size) {}
  void init(const fm_frame_t *input, fm_frame_t *result) {
    const T &val = *(const T *)fm_frame_get_cptr1(input, field_, 0);
    *(T *)fm_frame_get_ptr1(result, field_, 0) = obj_.init(val);
  };
  void set(const fm_frame_t *input, const fmc_time64_t &now) {
    const T &val = *(const T *)fm_frame_get_cptr1(input, field_, 0);

    if (isnan(val))
      return;

    obj_.set(val, prev_, now);
    prev_ = now;
  };
  void asof(const fm_frame_t *input, fm_frame_t *result,
            const fmc_time64_t &now) {
    const T &val = *(const T *)fm_frame_get_cptr1(input, field_, 0);
    *(T *)fm_frame_get_ptr1(result, field_, 0) = obj_.asof(val, prev_, now);
  };
  fm_field_t field_;
  fmc_time64_t prev_;
  S<T> obj_;
};
