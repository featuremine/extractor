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
 * @file sample.hpp
 * @author Maxim Trokhimtchouk
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the sample operators
 *
 * This file contains declarations of the sample operators
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include "extractor/comp_def.hpp"
#include "sample_gen.hpp"
#include "extractor/time64.hpp"
#include "fmc++/mpl.hpp"

#pragma once

using namespace std;

struct fm_comp_identity_result {
  fm_type_decl_cp result_type(fm_type_sys_t *sys, fm_type_decl_cp arg) {
    return arg;
  }
};

struct fm_comp_sample_2_0 : virtual public fm_comp_identity_result {
  fm_comp_sample_2_0(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
                     fm_type_decl_cp argv[], fm_type_decl_cp ptype,
                     fm_arg_stack_t plist) {
    fmc_runtime_error_unless(argc == 2) << "expect sampled operator and the "
                                           "interval indicator as inputs";
    fmc_range_error_unless(fm_args_empty(ptype)) << "expect no parameters";
  }
};

struct fm_comp_sample_frame : virtual public fm_comp_identity_result {
  bool init(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx) {
    fm_frame_assign(result, argv[0]);
    auto *frames = fm_exec_ctx_frames(ctx->exec);
    auto type = fm_frame_type(argv[0]);
    sample = fm_frame_from_type(frames, type);
    fm_frame_reserve(sample, fm_frame_dim(argv[0], 0));
    fm_frame_assign(sample, argv[0]);
    return true;
  }
  fm_type_decl_cp result_type(fm_type_sys_t *sys, fm_type_decl_cp arg) {
    return arg;
  }
  fm_frame_t *sample = nullptr;
};

struct fm_comp_sample_simple_init {
  bool init(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx) {
    fm_frame_assign(result, argv[0]);
    return true;
  }
};

struct fm_comp_asof : fm_comp_sample_2_0, fm_comp_sample_simple_init {
  fm_comp_asof(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
               fm_type_decl_cp argv[], fm_type_decl_cp ptype,
               fm_arg_stack_t plist)
      : fm_comp_sample_2_0(sys, closure, argc, argv, ptype, plist) {}
  bool exec(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx, bool interval, bool updated) {
    if (interval) {
      fm_frame_assign(result, argv[0]);
    }
    return interval;
  }
};

struct fm_comp_asof_prev : fm_comp_sample_2_0, fm_comp_sample_frame {
  fm_comp_asof_prev(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
                    fm_type_decl_cp argv[], fm_type_decl_cp ptype,
                    fm_arg_stack_t plist)
      : fm_comp_sample_2_0(sys, closure, argc, argv, ptype, plist) {}
  bool exec(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx, bool interval, bool updated) {
    if (interval) {
      fm_frame_assign(result, sample);
      fm_frame_assign(sample, argv[0]);
    }
    return interval;
  }
};

struct fm_comp_left_lim : fm_comp_sample_2_0, fm_comp_sample_frame {
  fm_comp_left_lim(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
                   fm_type_decl_cp argv[], fm_type_decl_cp ptype,
                   fm_arg_stack_t plist)
      : fm_comp_sample_2_0(sys, closure, argc, argv, ptype, plist) {}
  bool exec(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx, bool interval, bool updated) {
    if (interval) {
      fm_frame_assign(result, sample);
    }
    if (interval || updated) {
      fm_frame_assign(sample, argv[0]);
    }
    return interval;
  }
};

struct fm_comp_first_after : fm_comp_sample_2_0, fm_comp_sample_simple_init {
  fm_comp_first_after(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
                      fm_type_decl_cp argv[], fm_type_decl_cp ptype,
                      fm_arg_stack_t plist)
      : fm_comp_sample_2_0(sys, closure, argc, argv, ptype, plist) {}
  bool exec(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx, bool interval, bool updated) {
    if (interval) {
      has_update = false;
    }
    auto triggered = !has_update && updated;
    if (triggered) {
      fm_frame_assign(result, argv[0]);
      has_update = true;
    }
    return triggered;
  }
  bool has_update = false;
};

struct fm_comp_last_asof : fm_comp_sample_frame {
  fm_comp_last_asof(fm_comp_sys_t *sys, fm_comp_def_cl closure, unsigned argc,
                    fm_type_decl_cp argv[], fm_type_decl_cp ptype,
                    fm_arg_stack_t plist) {
    fmc_runtime_error_unless(argc == 2 || argc == 3)
        << "expect sampled operator, the "
           "interval indicator and an optional default "
           "value as inputs";
    fmc_range_error_unless(fm_args_empty(ptype)) << "expect no parameters";
  }
  bool init(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx) {
    fm_comp_sample_frame::init(result, args, argv, ctx);
    if (args != 3) {
      auto *frames = fm_exec_ctx_frames(ctx->exec);
      auto type = fm_frame_type(argv[0]);
      empty = fm_frame_from_type(frames, type);
      fm_frame_reserve(empty, fm_frame_dim(argv[0], 0));
      fm_frame_assign(empty, argv[0]);
    }
    return true;
  }
  bool exec(fm_frame_t *result, size_t args, const fm_frame_t *const argv[],
            fm_call_ctx_t *ctx, bool interval, bool updated) {
    if (interval) {
      if (has_update) {
        fm_frame_assign(result, sample);
      } else {
        if (args == 3) {
          fm_frame_assign(result, argv[2]);
        } else {
          fm_frame_assign(result, empty);
        }
      }
      has_update = false;
    }
    if (updated) {
      fm_frame_assign(sample, argv[0]);
      has_update = true;
    }
    return interval;
  }
  bool has_update = false;
  fm_frame_t *empty = nullptr;
};

bool fm_comp_sample_add_all(fm_comp_sys_t *sys) {
  return fm_comp_sample_add<fm_comp_asof>(sys, "asof") &&
         fm_comp_sample_add<fm_comp_asof>(sys, "sample") &&
         fm_comp_sample_add<fm_comp_asof_prev>(sys, "asof_prev") &&
         fm_comp_sample_add<fm_comp_left_lim>(sys, "left_lim") &&
         fm_comp_sample_add<fm_comp_first_after>(sys, "first_after") &&
         fm_comp_sample_add<fm_comp_last_asof>(sys, "last_asof");
}
