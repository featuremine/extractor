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
 * @file sample_gen.hpp
 * @author Maxim Trokhimtchouk
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the generic sample
 *
 * This file contains declarations of the generic sample
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
#include "extractor/type_sys.h"
}

#include "extractor/comp_def.hpp"
#include "extractor/time64.hpp"

using namespace std;

template <class C> struct fm_comp_sample_generic {
  struct closure {
    template <class... Args>
    closure(Args... args) : obj_(args...), interval{0} {}
    C obj_;
    bool interval = 0;
    bool updated = 0;
  };

  static bool stream_init(fm_frame_t *result, size_t args,
                          const fm_frame_t *const argv[], fm_call_ctx_t *ctx,
                          fm_call_exec_cl *cl) {
    auto *comp_cl = (closure *)ctx->comp;
    return comp_cl->obj_.init(result, args, argv, ctx);
  }

  static bool stream_exec(fm_frame_t *result, size_t args,
                          const fm_frame_t *const argv[], fm_call_ctx_t *ctx,
                          fm_call_exec_cl cl) {
    auto *comp_cl = (fm_comp_sample_generic<C>::closure *)ctx->comp;
    auto interval = comp_cl->interval;
    auto updated = comp_cl->updated;
    comp_cl->interval = 0;
    comp_cl->updated = 0;
    return comp_cl->obj_.exec(result, args, argv, ctx, interval, updated);
  }

  static fm_call_def *stream_call(fm_comp_def_cl comp_cl,
                                  const fm_ctx_def_cl ctx_cl) {
    auto *def = fm_call_def_new();
    fm_call_def_init_set(def, fm_comp_sample_generic<C>::stream_init);
    fm_call_def_exec_set(def, fm_comp_sample_generic<C>::stream_exec);
    return def;
  }

  static void queuer(size_t idx, fm_call_ctx_t *ctx) {
    auto *comp_cl = (closure *)ctx->comp;
    if (idx == 1) {
      comp_cl->interval = 1;
    } else {
      comp_cl->updated = 1;
    }
  }

  static fm_ctx_def_t *gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                           unsigned argc, fm_type_decl_cp argv[],
                           fm_type_decl_cp ptype, fm_arg_stack_t plist) {
    void *cl = nullptr;
    fm_type_decl_cp ret_type = nullptr;
    auto *sys = fm_type_sys_get(csys);
    try {
      auto *typed_cl = new fm_comp_sample_generic<C>::closure(
          csys, closure, argc, argv, ptype, plist);
      ret_type = typed_cl->obj_.result_type(sys, argv[0]);
      cl = (void *)typed_cl;
    } catch (std::range_error &e) {
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, e.what());
      return nullptr;
    } catch (std::runtime_error &e) {
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, e.what());
      return nullptr;
    } catch (std::exception &e) {
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN, e.what());
      return nullptr;
    } catch (...) {
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN,
                             "unknown "
                             "exception");
      return nullptr;
    }
    if (!ret_type) {
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN,
                             "could not determine return type");
      return nullptr;
    }
    auto *def = fm_ctx_def_new();
    fm_ctx_def_inplace_set(def, false);
    fm_ctx_def_type_set(def, ret_type);
    fm_ctx_def_closure_set(def, cl);
    fm_ctx_def_queuer_set(def, &fm_comp_sample_generic<C>::queuer);
    fm_ctx_def_stream_call_set(def, &fm_comp_sample_generic<C>::stream_call);
    fm_ctx_def_query_call_set(def, nullptr);
    return def;
  }

  static void destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
    if (auto *ctx_cl = (closure *)fm_ctx_def_closure(def)) {
      delete ctx_cl;
    }
  }
};

template <class C>
bool fm_comp_sample_add(fm_comp_sys_t *sys, const char *name) {
  fm_comp_def_t def = {name, &fm_comp_sample_generic<C>::gen,
                       &fm_comp_sample_generic<C>::destroy, NULL};
  return fm_comp_type_add(sys, &def);
}
