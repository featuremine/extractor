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
 * @file percentile.hpp
 * @author Andres Rangel
 * @date 4 Jan 2019
 * @brief File contains C++ definitions of the percentile operators
 *
 * This file contains declarations of the percentile operators
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
#include "extractor/type_sys.h"
}

#include "field_util.hpp"
#include "sample.hpp"
#include "window_util.hpp"
#include <deque>
#include <vector>

struct percentile_exec_cl {
  virtual void init(const fm_frame_t *argv, fm_frame_t *result) = 0;
  virtual void push(const fm_frame_t *argv) = 0;
  virtual void pop() = 0;
  virtual void eval(const std::vector<uint64_t> &percentiles,
                    fm_frame_t *result) = 0;
  virtual ~percentile_exec_cl(){};
};

template <template <class> class Comp> struct fm_percentile_tick_window {
  fm_percentile_tick_window(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                            unsigned argc, fm_type_decl_cp argv[],
                            fm_type_decl_cp ptype, fm_arg_stack_t plist)
      : curr_(0) {
    fmc_runtime_error_unless(argc == 1) << "expect single operator as input";

    fmc_runtime_error_unless(ptype && fm_type_is_tuple(ptype))
        << "expect tuple of "
           "parameters";

    fmc_runtime_error_unless(
        fm_arg_try_uinteger(fm_type_tuple_arg(ptype, 0), &plist, &len_))
        << "expect an unsigned integer as window length parameter";

    uint64_t nvals_ = fm_type_tuple_size(ptype) - 1;

    fmc_runtime_error_unless(nvals_ >= 1)
        << "expect at least one percentile value in parameters";

    for (uint64_t i = 0; i < nvals_; ++i) {
      uint64_t val;
      fmc_runtime_error_unless(
          fm_arg_try_uinteger(fm_type_tuple_arg(ptype, i + 1), &plist, &val))
          << "expect a an unsigned integer as percentile value";
      vals_.push_back(val);
    }

    int fields = fm_type_frame_nfields(argv[0]);

    int nf = fields * nvals_;

    vector<string> str_names;

    for (int idx = 0; idx < fields; ++idx) {
      string curr_prefix = fm_type_frame_field_name(argv[0], idx);
      for (uint64_t i = 0; i < nvals_; ++i) {
        str_names.emplace_back(curr_prefix + "_" + to_string(vals_[i]));
      }
    }

    vector<const char *> names(nf);
    vector<fm_type_decl_cp> types(nf);
    int nd = 1;
    int dims[1] = {1};
    auto *sys = fm_type_sys_get(csys);

    using supported_types = fmc::type_list<FLOAT32, FLOAT64>;

    for (int idx = 0; idx < fields; ++idx) {
      auto f_type = fm_type_frame_field_type(argv[0], idx);
      fm_type_decl_cp curr_type =
          get_comp_type<Comp>(supported_types(), sys, f_type);
      string curr_prefix = fm_type_frame_field_name(argv[0], idx);
      for (uint64_t i = 0; i < nvals_; ++i) {
        types[idx * nvals_ + i] = curr_type;
        names[idx * nvals_ + i] = str_names[idx * nvals_ + i].data();
      }
    }

    ret_type_ =
        fm_frame_type_get1(sys, nf, names.data(), types.data(), nd, dims);

    for (int idx = 0; idx < fields; ++idx) {
      vector<fm_field_t> fields(nvals_);
      for (uint64_t i = 0; i < nvals_; ++i) {
        fields[i] = fm_type_frame_field_idx(
            ret_type_, str_names[idx * nvals_ + i].c_str());
      }
      auto f_type = fm_type_frame_field_type(argv[0], idx);
      auto *call = get_field_exec_cl<percentile_exec_cl, Comp>(
          supported_types(), f_type, idx, fields);
      auto *str = fm_type_to_str(f_type);
      string type_str = str;
      free(str);
      fmc_runtime_error_unless(call) << "invalid type " << type_str;
      calls.push_back(call);
    }
  }
  ~fm_percentile_tick_window() {
    for (auto &&call : calls) {
      delete call;
    }
  }
  fm_type_decl_cp result_type(fm_type_sys_t *sys, fm_type_decl_cp arg) {
    return ret_type_;
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
        call->pop();
      }
    }

    for (auto *call : calls) {
      call->push(argv[0]);
      call->eval(vals_, result);
    }
    return true;
  }
  uint64_t len_;
  uint64_t curr_;
  vector<percentile_exec_cl *> calls;
  fm_type_decl_cp ret_type_;
  std::vector<uint64_t> vals_;
};

template <template <class> class Comp> struct fm_percentile_time_window {
  fm_percentile_time_window(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                            unsigned argc, fm_type_decl_cp argv[],
                            fm_type_decl_cp ptype, fm_arg_stack_t plist) {

    fmc_runtime_error_unless(argc == 1) << "expect single operator as input";

    fmc_runtime_error_unless(ptype && fm_type_is_tuple(ptype))
        << "expect tuple of "
           "parameters";

    fmc_runtime_error_unless(
        fm_arg_try_time64(fm_type_tuple_arg(ptype, 0), &plist, &window_size_))
        << "expect a time window length parameter";

    uint64_t nvals_ = fm_type_tuple_size(ptype) - 1;

    fmc_runtime_error_unless(nvals_ >= 1)
        << "expect at least one percentile value in parameters";

    for (uint64_t i = 0; i < nvals_; ++i) {
      uint64_t val;
      fmc_runtime_error_unless(
          fm_arg_try_uinteger(fm_type_tuple_arg(ptype, i + 1), &plist, &val))
          << "expect a an unsigned integer as percentile value";
      vals_.push_back(val);
    }

    int fields = fm_type_frame_nfields(argv[0]);

    int nf = fields * nvals_;

    vector<string> str_names;

    for (int idx = 0; idx < fields; ++idx) {
      string curr_prefix = fm_type_frame_field_name(argv[0], idx);
      for (uint64_t i = 0; i < nvals_; ++i) {
        str_names.emplace_back(curr_prefix + "_" + to_string(vals_[i]));
      }
    }

    vector<const char *> names(nf);
    vector<fm_type_decl_cp> types(nf);
    int nd = 1;
    int dims[1] = {1};
    auto *sys = fm_type_sys_get(csys);

    using supported_types = fmc::type_list<FLOAT32, FLOAT64>;

    for (int idx = 0; idx < fields; ++idx) {
      auto f_type = fm_type_frame_field_type(argv[0], idx);
      fm_type_decl_cp curr_type =
          get_comp_type<Comp>(supported_types(), sys, f_type);
      string curr_prefix = fm_type_frame_field_name(argv[0], idx);
      for (uint64_t i = 0; i < nvals_; ++i) {
        types[idx * nvals_ + i] = curr_type;
        names[idx * nvals_ + i] = str_names[idx * nvals_ + i].data();
      }
    }

    ret_type_ =
        fm_frame_type_get1(sys, nf, names.data(), types.data(), nd, dims);

    for (int idx = 0; idx < fields; ++idx) {
      vector<fm_field_t> fields(nvals_);
      for (uint64_t i = 0; i < nvals_; ++i) {
        fields[i] = fm_type_frame_field_idx(
            ret_type_, str_names[idx * nvals_ + i].c_str());
      }
      auto f_type = fm_type_frame_field_type(argv[0], idx);
      auto *call = get_field_exec_cl<percentile_exec_cl, Comp>(
          supported_types(), f_type, idx, fields);
      auto *str = fm_type_to_str(f_type);
      string type_str = str;
      free(str);
      fmc_runtime_error_unless(call) << "invalid type " << type_str;
      calls.push_back(call);
    }
  }
  ~fm_percentile_time_window() {
    for (auto &&call : calls) {
      delete call;
    }
  }
  fm_type_decl_cp result_type(fm_type_sys_t *sys, fm_type_decl_cp arg) {
    return ret_type_;
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
        call->pop();
      }
      queue_.pop_front();
    }

    if (updated) {
      for (auto *call : calls) {
        call->push(argv[0]);
      }
      queue_.emplace_back(now);
    }

    for (auto *call : calls) {
      call->eval(vals_, result);
    }

    if (!queue_.empty()) {
      fm_stream_ctx_schedule((fm_stream_ctx_t *)ctx->exec, ctx->handle,
                             queue_.front() + window_size_);
    }

    return true;
  }
  fmc_time64_t window_size_;
  vector<percentile_exec_cl *> calls;
  std::deque<fmc_time64_t> queue_;
  fm_type_decl_cp ret_type_;
  std::vector<uint64_t> vals_;
};

template <class T> struct percentile_field_exec_cl : percentile_exec_cl {
  using result = T;
  percentile_field_exec_cl(fm_field_t field, std::vector<fm_field_t> fields)
      : field_(field), fields_(fields) {}
  void init(const fm_frame_t *argv, fm_frame_t *result) {
    const T &val = *(const T *)fm_frame_get_cptr1(argv, field_, 0);

    for (auto &&field : fields_) {
      *(T *)fm_frame_get_ptr1(result, field, 0) = val;
    }
  }
  void push(const fm_frame_t *argv) {
    const T &val = *(const T *)fm_frame_get_cptr1(argv, field_, 0);
    queue_.push_back(val);

    if (isnan(val)) {
      return;
    }

    vals_.insert(std::lower_bound(vals_.begin(), vals_.end(), val), val);
  }
  void pop() {
    const T val = queue_.front();
    queue_.pop_front();

    if (isnan(val))
      return;

    vals_.erase(std::lower_bound(vals_.begin(), vals_.end(), val));
  }
  void eval(const std::vector<uint64_t> &percentiles, fm_frame_t *result) {
    uint64_t sz = vals_.size();

    if (sz) {
      for (uint64_t i = 0; i < fields_.size(); ++i) {
        auto idx = min(percentiles[i] * sz / 100, sz - 1UL);
        *(T *)fm_frame_get_ptr1(result, fields_[i], 0) = vals_[idx];
      }
    } else {
      for (auto &&field : fields_) {
        *(T *)fm_frame_get_ptr1(result, field, 0) =
            std::numeric_limits<T>::quiet_NaN();
      }
    }
  }
  fm_field_t field_;
  std::vector<fm_field_t> fields_;
  std::deque<T> queue_;
  std::vector<T> vals_;
};

bool fm_comp_percentile_add(fm_comp_sys_t *sys) {
  return fm_comp_sample_add<
             fm_percentile_tick_window<percentile_field_exec_cl>>(
             sys, "percentile_tick_mw") &&
         fm_comp_sample_add<
             fm_percentile_time_window<percentile_field_exec_cl>>(
             sys, "percentile_time_mw");
}
