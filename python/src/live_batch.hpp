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
 * @file live_batch.hpp
 * @author Maxim Trokhimtchouk
 * @date 2 Apr 2020
 * @brief File contains C++ definitions of the live_batch operator
 *
 * The live_batch operator is designed to replay named tuples in
 * real live mode.
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "extractor/type_sys.h"
#include "fmc++/mpl.hpp"
#include "py_utils.hpp"
#include "py_wrapper.hpp"

#include <cassert>
#include <errno.h>
#include <functional>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include "fmc++/strings.hpp"
#include <numpy/arrayobject.h>

using namespace fm;
using namespace python;
using namespace std;

struct live_batch {
  enum status { ERR = 0, IDLE, DATA, DONE };
  live_batch(object iter, fmc_time64_t pp)
      : frm_it_(iter), polling_period_(pp) {}

  void iter_process_data(fm_frame_t *result, fm_call_ctx_t *ctx) {
    auto &obj = *(PyObject **)fm_frame_get_ptr1(result, 0, 0);
    Py_XDECREF(obj);
    obj = object(row_ob_).get_ref();
    Py_XINCREF(obj);
  }

  status process_once(fm_frame_t *result, fm_call_ctx_t *ctx) {
    auto py_error_check = [&](status s) {
      if (PyErr_Occurred()) {
        set_python_error(ctx->exec);
        return ERR;
      }
      return s;
    };

    auto *s_ctx = (fm_stream_ctx *)ctx->exec;
    auto now = fm_stream_ctx_now(s_ctx);

    if (!row_it_) {
      if (auto frame = frm_it_.next(); frame) {
        // we expect returned object to be a list
        if (PyList_Check(frame.get_ref())) {
          row_it_ = frame.iter();
          if (!row_it_) {
            return py_error_check(ERR);
          }
        } else {
          fm_exec_ctx_error_set(ctx->exec,
                                "expecting either a list "
                                "of tuples, instead got %s",
                                frame.str().c_str());
          return ERR;
        }
      } else {
        return py_error_check(DONE);
      }
    }

    // read the next item
    row_ob_ = row_it_.next();
    if (!row_ob_) {
      row_it_ = object();
      if (PyErr_Occurred()) {
        set_python_error(ctx->exec);
        return ERR;
      } else {
        auto next = now + polling_period_;
        fm_stream_ctx_schedule(s_ctx, ctx->handle, next);
        return IDLE;
      }
    }
    if (!PyTuple_Check(row_ob_.get_ref())) {
      fm_exec_ctx_error_set(ctx->exec,
                            "expecting either a tuple or list "
                            "of tuples, instead got %s",
                            row_ob_.str().c_str());
      return ERR;
    }
    iter_process_data(result, ctx);
    fm_stream_ctx_schedule(s_ctx, ctx->handle, now);
    return DATA;
  }
  object frm_it_;
  object row_it_;
  object row_ob_;
  fmc_time64_t polling_period_;
};

bool fm_comp_live_batch_stream_init(fm_frame_t *result, size_t args,
                                    const fm_frame_t *const argv[],
                                    fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto *s_ctx = (fm_stream_ctx *)ctx->exec;
  fm_stream_ctx_queue(s_ctx, ctx->handle);
  return true;
}

bool fm_comp_live_batch_stream_exec(fm_frame_t *result, size_t,
                                    const fm_frame_t *const argv[],
                                    fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *clbl = (live_batch *)ctx->comp;
  if (auto res = clbl->process_once(result, ctx); res == live_batch::DATA) {
    return true;
  }
  return false;
}

fm_call_def *fm_comp_live_batch_stream_call(fm_comp_def_cl comp_cl,
                                            const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_live_batch_stream_init);
  fm_call_def_exec_set(def, fm_comp_live_batch_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_live_batch_gen(fm_comp_sys_t *csys,
                                     fm_comp_def_cl closure, unsigned argc,
                                     fm_type_decl_cp argv[],
                                     fm_type_decl_cp ptype,
                                     fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 0) {
    const char *errstr = "no input features should be provided.";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto error = [&]() {
    const char *errstr = "expect a python iterator and a polling period";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 2)
    return error();

  auto rec_t = fm_record_type_get(sys, "PyObject*", sizeof(PyObject *));
  auto *param0 = fm_type_tuple_arg(ptype, 0);
  if (!fm_type_is_record(param0) || !fm_type_equal(rec_t, param0))
    return error();
  auto clbl = object::from_borrowed(STACK_POP(plist, PyObject *));
  if (!PyIter_Check(clbl.get_ref()))
    return error();

  fmc_time64_t polling_period{0};
  if (!fm_arg_try_time64(fm_type_tuple_arg(ptype, 1), &plist,
                         &polling_period)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect second "
                           "parameter to be a "
                           "polling period");
    return nullptr;
  }

  auto type = fm_frame_type_get(sys, 1, 1, "update", rec_t, 1);
  if (!type) {
    return nullptr;
  }

  try {
    auto *cl = new live_batch(clbl, polling_period);
    auto *def = fm_ctx_def_new();
    fm_ctx_def_inplace_set(def, false);
    fm_ctx_def_type_set(def, type);
    fm_ctx_def_closure_set(def, cl);
    fm_ctx_def_stream_call_set(def, &fm_comp_live_batch_stream_call);
    fm_ctx_def_query_call_set(def, nullptr);
    return def;
  } catch (std::exception &e) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, e.what());
    return nullptr;
  }
}

void fm_comp_live_batch_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (live_batch *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}

const fm_comp_def_t fm_comp_live_batch = {"live_batch", &fm_comp_live_batch_gen,
                                          &fm_comp_live_batch_destroy, NULL};
