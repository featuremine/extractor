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
 * @file py_play.hpp
 * @author Maxim Trokhimtchouk
 * @date 19 Mar 2019
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
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

struct df_row_parser {
  df_row_parser(fm_type_decl_cp decl) {
    // This better be true;
    assert(fm_type_is_frame(decl));
    auto nfields = fm_type_frame_nfields(decl);
    for (auto i = 0u; i < nfields; ++i) {
      auto type = fm_type_frame_field_type(decl, i);
      auto name = fm_type_frame_field_name(decl, i);
      auto check = get_df_column_check(name, type);
      auto idx = fm_type_frame_field_idx(decl, name);
      auto parse = get_df_column_parse(name, type, idx);
      if (!(check && parse)) {
        auto tp_name = fmc::autofree<char>{fm_type_to_str(type)};
        fmc_runtime_error_unless(false) << "could not obtain parser for column "
                                        << name << " of type " << *tp_name;
      }
      checks_.push_back(check);
      parses_.push_back(parse);
    }
  }
  bool validate(object obj, fm_call_ctx_t *ctx) {
    if (!validate_index(obj["index"], ctx)) {
      return false;
    }
    auto dtypes = obj["dtypes"];
    if (!dtypes) {
      const char *errstr = "unable to obtain dtypes";
      fm_exec_ctx_error_set(ctx->exec, errstr);
      return false;
    }
    for (auto &check : checks_) {
      if (!check(dtypes, ctx))
        return false;
    }
    return true;
  }
  bool validate_index(object index, fm_call_ctx_t *ctx) {
    if (!bool(index)) {
      const char *errstr = "unable to obtain index";
      fm_exec_ctx_error_set(ctx->exec, errstr);
      return false;
    }

    auto *index_type = (PyArray_Descr *)index["dtype"].get_ref();

    auto datetime = datetime::get_pandas_dttz_type();
    if (!datetime) {
      const char *errstr =
          "cannot create pandas.core.dtypes.dtypes.DatetimeTZDtype python "
          "object";
      fm_exec_ctx_error_set(ctx->exec, errstr);
      return false;
    }

    if (PyArray_DescrCheck(index_type)) {
      if (index_type->type_num != NPY_DATETIME) {
        fm_exec_ctx_error_set(ctx->exec,
                              "provided type %s for index is "
                              "not valid, expecting "
                              "datetime64[ns]",
                              index_type->typeobj->tp_name);
        return false;
      }
    } else if (!PyObject_TypeCheck(index_type,
                                   (PyTypeObject *)datetime.get_ref())) {
      const char *errstr = "invalid index type description";
      fm_exec_ctx_error_set(ctx->exec, errstr);
      return false;
    }

    return true;
  }
  bool parse(object row, fm_frame_t *result, fm_call_ctx_t *ctx) {
    for (auto &parser : parses_) {
      if (!parser(row, result, ctx)) {
        return false;
      }
    }
    return true;
  }
  vector<df_column_check> checks_;
  vector<py_field_parse> parses_;
};

struct py_play {
  enum status { ERR = 0, IDLE, DATA, DONE };
  py_play(fm_type_decl_cp type, bool immediate, object iter, fm_time64_t pp)
      : frm_it_(iter), nxt_tm_(fm_time64_end()), parser_(type),
        immediate_(immediate), polling_period_(pp) {}
  status iter_process_next(fm_call_ctx_t *ctx, bool repeat = true) {
    auto py_error_check = [&](status s) {
      if (PyErr_Occurred()) {
        set_python_error(ctx->exec);
        return ERR;
      }
      return s;
    };
    if (!row_it_) {
      if (auto frame = frm_it_.next(); frame) {
        if (frame.is_None()) {
          return IDLE;
        }
        if (!parser_.validate(frame, ctx)) {
          return ERR;
        }
        if (auto it_tp = frame["itertuples"]; it_tp) {
          row_it_ = it_tp();
        } else {
          return py_error_check(ERR);
        }
      } else {
        return py_error_check(DONE);
      }
    }
    row_ob_ = row_it_.next();
    if (!row_ob_) {
      row_it_ = object();
      nxt_tm_ = fm_time64_end();
      if (PyErr_Occurred()) {
        return py_error_check(ERR);
      } else {
        if (repeat) {
          return iter_process_next(ctx, false);
        } else {
          return IDLE;
        }
      }
    }
    auto index = row_ob_[0];
    if (!index) {
      fm_exec_ctx_error_set(ctx->exec, "could not obtain index from "
                                       "pandas DataFrame");
      return ERR;
    }
    nxt_tm_ = set_next_time(index);
    return DATA;
  }

  fm_time64_t iter_next_time() { return nxt_tm_; }

  bool iter_has_data() { return bool(row_ob_); }

  bool iter_process_data(fm_frame_t *result, fm_call_ctx_t *ctx) {
    return parser_.parse(row_ob_, result, ctx);
  }

  fm_time64_t set_next_time(object obj) {
    auto dt_ob = obj["value"];
    return fm_time64_from_nanos(PyLong_AsLongLong(dt_ob.get_ref()));
  }

  status process_next(fm_call_ctx_t *ctx, bool done) {
    auto *s_ctx = (fm_stream_ctx *)ctx->exec;
    auto res = iter_process_next(ctx);
    if (res == ERR || res == DONE) {
      return res;
    }
    auto now = fm_stream_ctx_now(s_ctx);
    if (res == IDLE) {
      auto next = now + polling_period_;
      fm_stream_ctx_schedule(s_ctx, ctx->handle, next);
      return IDLE;
    }
    auto n_t = iter_next_time();
    bool immediate = immediate_ || n_t <= now;
    if (!done && immediate) {
      return DATA;
    }
    fm_stream_ctx_schedule(s_ctx, ctx->handle, immediate_ ? now : n_t);
    return IDLE;
  }

  status process_data(fm_frame_t *result, fm_call_ctx_t *ctx) {
    // If we do we need to parse it and write to the result
    if (!iter_process_data(result, ctx)) {
      return ERR;
    }
    process_next(ctx, true);
    return DATA;
  }

  status process_once(fm_frame_t *result, fm_call_ctx_t *ctx) {
    if (!iter_has_data()) {
      if (auto res = process_next(ctx, false); res != DATA) {
        return res;
      }
    }
    return process_data(result, ctx);
  }
  object frm_it_;
  object row_it_;
  object row_ob_;
  fm_time64_t nxt_tm_;
  df_row_parser parser_;
  bool immediate_;
  fm_time64_t polling_period_;
};

bool fm_comp_py_play_stream_init(fm_frame_t *result, size_t args,
                                 const fm_frame_t *const argv[],
                                 fm_call_ctx_t *call_ctx, fm_call_exec_cl *cl) {
  auto *clbl = (py_play *)call_ctx->comp;
  if (clbl->process_once(result, call_ctx) == py_play::ERR) {
    return false;
  }
  return true;
}

bool fm_comp_py_play_stream_exec(fm_frame_t *result, size_t,
                                 const fm_frame_t *const argv[],
                                 fm_call_ctx_t *call_ctx, fm_call_exec_cl cl) {
  auto *clbl = (py_play *)call_ctx->comp;
  if (auto res = clbl->process_once(result, call_ctx); res == py_play::DATA) {
    return true;
  }
  return false;
}

fm_call_def *fm_comp_py_play_stream_call(fm_comp_def_cl comp_cl,
                                         const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_py_play_stream_init);
  fm_call_def_exec_set(def, fm_comp_py_play_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_base_py_play_gen(bool immediate, fm_comp_sys_t *csys,
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
    const char *errstr = "expect a python iterator, "
                         "a tuple describing result frame type, "
                         "and a polling period";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!ptype)
    return error();

  if (!fm_type_is_tuple(ptype))
    return error();
  if (fm_type_tuple_size(ptype) != 3)
    return error();

  auto rec_t = fm_record_type_get(sys, "PyObject*", sizeof(PyObject *));

  auto *param1 = fm_type_tuple_arg(ptype, 0);
  if (!fm_type_is_record(param1) || !fm_type_equal(rec_t, param1))
    return error();

  auto clbl = object::from_borrowed(STACK_POP(plist, PyObject *));

  if (!PyIter_Check(clbl.get_ref()))
    return error();

  auto *row_descs = fm_type_tuple_arg(ptype, 1);
  if (!fm_type_is_tuple(row_descs))
    return error();
  auto size = fm_type_tuple_size(row_descs);

  vector<const char *> names(size);
  vector<fm_type_decl_cp> types(size);
  int dims[1] = {1};

  auto field_error = [sys, error](size_t field_idx, const char *str) {
    string errstr = str;
    errstr.append(" for field ");
    errstr.append(to_string(field_idx));
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr.c_str());
    return nullptr;
  };

  for (unsigned i = 0; i < size; ++i) {
    auto *row_desc = fm_type_tuple_arg(row_descs, i);
    auto field_tuple_size = fm_type_tuple_size(row_desc);
    if (field_tuple_size != 2) {
      string errstr = "invalid field description size ";
      errstr.append(to_string(field_tuple_size));
      errstr.append("; expected 2");
      return field_error(i, errstr.c_str());
    };

    if (!fm_type_is_cstring(fm_type_tuple_arg(row_desc, 0))) {
      return field_error(i, "first element of field description tuple "
                            "must be the field name");
    };
    names[i] = STACK_POP(plist, const char *);

    if (!fm_type_is_type(fm_type_tuple_arg(row_desc, 1))) {
      return field_error(i, "second element of field description tuple "
                            "must be of type type");
    };
    types[i] = STACK_POP(plist, fm_type_decl_cp);

    if (!fm_type_is_simple(types[i])) {
      auto *typestr = fm_type_to_str(types[i]);
      auto errstr = string("expect simple type, got: ") + typestr;
      free(typestr);
      return field_error(i, errstr.c_str());
    }
  }

  auto *type =
      fm_frame_type_get1(sys, size, names.data(), types.data(), 1, dims);
  if (!type) {
    const char *errstr = "unable to generate type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  fm_time64_t polling_period{0};
  if (!fm_arg_try_time64(fm_type_tuple_arg(ptype, 2), &plist,
                         &polling_period)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect third "
                           "parameter to be a "
                           "polling period");
    return nullptr;
  }

  try {
    auto *cl = new py_play(type, immediate, clbl, polling_period);
    auto *def = fm_ctx_def_new();
    fm_ctx_def_inplace_set(def, false);
    fm_ctx_def_type_set(def, type);
    fm_ctx_def_closure_set(def, cl);
    fm_ctx_def_stream_call_set(def, &fm_comp_py_play_stream_call);
    fm_ctx_def_query_call_set(def, nullptr);
    return def;
  } catch (std::exception &e) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, e.what());
    return nullptr;
  }
}

fm_ctx_def_t *
fm_comp_scheduled_py_play_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                              unsigned argc, fm_type_decl_cp argv[],
                              fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  return fm_comp_base_py_play_gen(false, csys, closure, argc, argv, ptype,
                                  plist);
}

fm_ctx_def_t *
fm_comp_immediate_py_play_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                              unsigned argc, fm_type_decl_cp argv[],
                              fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  return fm_comp_base_py_play_gen(true, csys, closure, argc, argv, ptype,
                                  plist);
}

void fm_comp_py_play_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (py_play *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}

const fm_comp_def_t fm_comp_scheduled_play = {"scheduled_play",
                                              &fm_comp_scheduled_py_play_gen,
                                              &fm_comp_py_play_destroy, NULL};

const fm_comp_def_t fm_comp_immediate_play = {"immediate_play",
                                              &fm_comp_immediate_py_play_gen,
                                              &fm_comp_py_play_destroy, NULL};
