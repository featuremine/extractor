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
 * @file ar.hpp
 * @author Andres Rangel
 * @date 2 Jan 2019
 * @brief File contains C++ definitions of the ar operators
 *
 * This file contains declarations of the ar operators
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "extractor/time64.h"
}

#include "field_util.hpp"
#include "sample.hpp"
#include <vector>

struct base_ar_field_exec_cl {
  virtual void init(const fm_frame_t *input, fm_frame_t *result) = 0;
  virtual void exec(const fm_frame_t *const input[], fm_frame_t *result) = 0;
  virtual ~base_ar_field_exec_cl(){};
};

template <class T> struct ar_field_exec_cl : base_ar_field_exec_cl {
  ar_field_exec_cl(fm_field_t field) : field_(field) {}
  void init(const fm_frame_t *input, fm_frame_t *result) {
    prev_ = *(T *)fm_frame_get_ptr1(result, field_, 0) =
        *(const T *)fm_frame_get_cptr1(input, field_, 0);
  };
  void exec(const fm_frame_t *const input[], fm_frame_t *result) {
    const T &val = *(const T *)fm_frame_get_cptr1(input[0], field_, 0);

    if (isnan(val))
      return;

    if (isnan(prev_)) {
      *(T *)fm_frame_get_ptr1(result, field_, 0) = prev_ = val;
      return;
    }

    *(T *)fm_frame_get_ptr1(result, field_, 0) = prev_ =
        *(const T *)fm_frame_get_cptr1(input[1], 0, 0) * prev_ +
        *(const T *)fm_frame_get_cptr1(input[2], 0, 0) *
            *(const T *)fm_frame_get_cptr1(input[0], field_, 0);
  };
  fm_field_t field_;
  T prev_;
};

struct fm_comp_ar : fm_comp_identity_result {
  fm_comp_ar(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
             fm_type_decl_cp argv[], fm_type_decl_cp ptype,
             fm_arg_stack_t plist) {
    fmc_runtime_error_unless(argc == 3) << "expect input operator and the "
                                           "interval indicator as inputs";
    fmc_range_error_unless(fm_args_empty(ptype)) << "expect no parameters";

    using supported_types = fmc::type_list<FLOAT32, FLOAT64>;

    int nf = fm_type_frame_nfields(argv[0]);
    auto base_type = fm_type_frame_field_type(argv[0], 0);

    fmc_runtime_error_unless(fm_type_frame_nfields(argv[1]) == 1)
        << "second input must have one field";
    fmc_runtime_error_unless(fm_type_frame_field_type(argv[1], 0) == base_type)
        << "type missmatch";

    fmc_runtime_error_unless(fm_type_frame_nfields(argv[2]) == 1)
        << "second input must have one field";
    fmc_runtime_error_unless(fm_type_frame_field_type(argv[2], 0) == base_type)
        << "type missmatch";

    for (int idx = 0; idx < nf; ++idx) {
      auto f_type = fm_type_frame_field_type(argv[0], idx);
      fmc_runtime_error_unless(f_type == base_type) << "type missmatch";
      auto *call = get_field_exec_cl<base_ar_field_exec_cl, ar_field_exec_cl>(
          supported_types(), base_type, idx);
      auto *str = fm_type_to_str(base_type);
      string type_str = str;
      free(str);
      fmc_runtime_error_unless(call) << "invalid type " << type_str;
      calls.push_back(call);
    }
  }
  ~fm_comp_ar() {
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
    if (updated || interval) {
      for (auto *call : calls) {
        call->exec(argv, result);
      }
    }

    return interval;
  }
  std::vector<base_ar_field_exec_cl *> calls;
};

bool fm_comp_ar_add(fm_comp_sys_t *sys) {
  return fm_comp_sample_add<fm_comp_ar>(sys, "ar");
}
