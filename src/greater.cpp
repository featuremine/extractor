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
 * @file greater.cpp
 * @authors Andres Rangel
 * @date 22 Aug 2018
 * @brief File contains C++ definitions for the "greater" logical operator
 *
 * @see http://www.featuremine.com
 */

#include "greater.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"

#include "extractor/comp_def.hpp"
#include "extractor/frame.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/rational64.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/time.hpp"
#include "op_util.hpp"

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

template <class T> struct the_greater_field_exec_2_0 : op_field_exec {
  the_greater_field_exec_2_0(fm_field_t field) : field_(field) {}

  void exec(fm_frame_t *result, size_t args,
            const fm_frame_t *const argv[]) override {
    bool res = *(const T *)fm_frame_get_cptr1(argv[0], field_, 0) >
               *(const T *)fm_frame_get_cptr1(argv[1], field_, 0);

    *(bool *)fm_frame_get_ptr1(result, field_, 0) = res;
  }

  fm_field_t field_;
};

template <> struct the_greater_field_exec_2_0<char *> : op_field_exec {
  the_greater_field_exec_2_0(fm_field_t field) : field_(field) {}

  void exec(fm_frame_t *result, size_t args,
            const fm_frame_t *const argv[]) override {
    bool res = memcmp((const char *)fm_frame_get_cptr1(argv[0], field_, 0),
                      (const char *)fm_frame_get_cptr1(argv[1], field_, 0),
                      fm_type_array_size(fm_type_frame_field_type(
                          fm_frame_type(argv[0]), field_))) > 0;

    *(bool *)fm_frame_get_ptr1(result, field_, 0) = res;
  }

  fm_field_t field_;
};

struct greater_comp_cl {
  ~greater_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<op_field_exec *> calls;
};

bool fm_comp_greater_call_stream_init(fm_frame_t *result, size_t args,
                                      const fm_frame_t *const argv[],
                                      fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto &calls = (*(greater_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

bool fm_comp_greater_stream_exec(fm_frame_t *result, size_t args,
                                 const fm_frame_t *const argv[],
                                 fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &calls = (*(greater_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

fm_call_def *fm_comp_greater_stream_call(fm_comp_def_cl comp_cl,
                                         const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_greater_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_greater_stream_exec);
  return def;
}

template <class... Ts>
op_field_exec *get_greater_field_exec(fmc::type_list<Ts...>,
                                      fm_type_decl_cp f_type, int idx) {
  op_field_exec *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = new the_greater_field_exec_2_0<Tn>(idx);
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}

fm_ctx_def_t *fm_comp_greater_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                  unsigned argc, fm_type_decl_cp argv[],
                                  fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  if (argc != 2) {
    auto *errstr = "expect two operator arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect no "
                           "parameters");
    return nullptr;
  }

  if (fm_type_frame_nfields(argv[0]) == 1 &&
      fm_type_frame_nfields(argv[1]) == 1) {
    auto type0 = fm_type_frame_field_type(argv[0], 0);
    auto type1 = fm_type_frame_field_type(argv[1], 0);
    if (!fm_type_equal(type0, type1)) {
      auto *errstr = "the two fields have different types";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
      return nullptr;
    }
  } else if (!fm_type_equal(argv[0], argv[1])) {
    auto *errstr = "all operator arguments must be the same type or have a "
                   "single field of same type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto ctx_cl = make_unique<greater_comp_cl>();
  auto &calls = ctx_cl->calls;

  using supported_types =
      fmc::type_list<INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64,
                     FLOAT32, FLOAT64, RPRICE, TIME64, RATIONAL64>;

  auto inp = argv[0];

  int nf = fm_type_frame_nfields(inp);
  int nd = fm_type_frame_ndims(inp);
  vector<const char *> names(nf);
  vector<fm_type_decl_cp> types(nf);

  vector<int> dims(nd);

  for (int idx = 0; idx < nf; ++idx) {
    dims[idx] = fm_type_frame_dim(inp, idx);
  }

  auto bool_param_t = fm_base_type_get(sys, FM_TYPE_BOOL);

  for (int idx = 0; idx < nf; ++idx) {
    names[idx] = fm_type_frame_field_name(argv[0], idx);

    auto f_type = fm_type_frame_field_type(inp, idx);
    types[idx] = bool_param_t;

    if (fm_type_is_base(f_type)) {
      auto *call = get_greater_field_exec(supported_types(), f_type, idx);
      if (!call) {
        ostringstream os;
        auto *str = fm_type_to_str(f_type);
        os << "type " << str << "is not supported in greater feature";
        free(str);
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, os.str().c_str());
        return nullptr;
      }
      calls.push_back(call);
    } else if (fm_type_is_array(f_type)) {
      auto af_type = fm_type_array_of(f_type);
      if (fm_type_is_base(af_type)) {
        switch (fm_type_base_enum(af_type)) {
        case FM_TYPE_CHAR:
          calls.push_back(new the_greater_field_exec_2_0<char *>(idx));
          break;
        default:
          ostringstream os;
          auto *str = fm_type_to_str(af_type);
          os << "array of type " << str
             << "is not supported in "
                "greater feature";
          free(str);
          fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, os.str().c_str());
          return nullptr;
          break;
        }
      }
    }
  }

  auto type =
      fm_frame_type_get1(sys, nf, names.data(), types.data(), nd, dims.data());
  if (!type) {
    auto *errstr = "unable to create result frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_greater_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_greater_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (greater_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
