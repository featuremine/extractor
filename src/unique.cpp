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
 * @file unique.cpp
 * @authors Maxim Trokhimtchouk
 * @date 17 Dec 2018
 * @brief File contains C++ definitions for the "unique" filtering operator
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "unique.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "extractor/frame.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/time.hpp"
#include "op_util.hpp"

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

struct unique_field_exec {
  virtual ~unique_field_exec() {}
  virtual bool exec(const fm_frame_t *result,
                    const fm_frame_t *const argv[]) = 0;
};

template <class T> struct the_unique_field_exec_2_0 : unique_field_exec {
  the_unique_field_exec_2_0(fm_field_t field) : field_(field) {}

  bool exec(const fm_frame_t *result, const fm_frame_t *const argv[]) override {
    return fmc::almost_equal<T>(
        *(const T *)fm_frame_get_cptr1(argv[0], field_, 0),
        *(const T *)fm_frame_get_cptr1(result, field_, 0));
  }

  fm_field_t field_;
};

template <> struct the_unique_field_exec_2_0<char *> : unique_field_exec {
  the_unique_field_exec_2_0(fm_field_t field, size_t len)
      : field_(field), len_(len) {}

  bool exec(const fm_frame_t *result, const fm_frame_t *const argv[]) override {
    return memcmp((const char *)fm_frame_get_cptr1(argv[0], field_, 0),
                  (const char *)fm_frame_get_cptr1(result, field_, 0),
                  len_) == 0;
  }

  fm_field_t field_;
  size_t len_;
};

struct unique_comp_cl {
  ~unique_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<unique_field_exec *> calls;
};

bool fm_comp_unique_call_stream_init(fm_frame_t *result, size_t args,
                                     const fm_frame_t *const argv[],
                                     fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  fm_frame_assign(result, argv[0]);
  return true;
}

bool fm_comp_unique_stream_exec(fm_frame_t *result, size_t args,
                                const fm_frame_t *const argv[],
                                fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &calls = (*(unique_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    if (!call->exec(result, argv)) {
      fm_frame_assign(result, argv[0]);
      return true;
    }
  }
  return false;
}

fm_call_def *fm_comp_unique_stream_call(fm_comp_def_cl comp_cl,
                                        const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_unique_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_unique_stream_exec);
  return def;
}

template <class... Ts>
unique_field_exec *get_unique_field_exec(fmc::type_list<Ts...>,
                                         fm_type_decl_cp f_type, int idx) {
  unique_field_exec *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = new the_unique_field_exec_2_0<Tn>(idx);
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}

fm_ctx_def_t *fm_comp_unique_gen(fm_comp_sys_t *sys, fm_comp_def_cl closure,
                                 unsigned argc, fm_type_decl_cp argv[],
                                 fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *tsys = fm_type_sys_get(sys);
  if (argc != 1) {
    auto *errstr = "expect single operator arguments";
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_PARAMS,
                           "expect no "
                           "parameters");
    return nullptr;
  }

  auto ctx_cl = make_unique<unique_comp_cl>();
  auto &calls = ctx_cl->calls;

  using supported_types =
      fmc::type_list<INT8, INT16, INT32, INT64, UINT8, UINT16, UINT32, UINT64,
                     FLOAT32, FLOAT64, RPRICE, DECIMAL128, TIME64>;

  auto inp = argv[0];

  int nf = fm_type_frame_nfields(inp);

  for (int idx = 0; idx < nf; ++idx) {
    auto f_type = fm_type_frame_field_type(inp, idx);

    if (fm_type_is_base(f_type)) {
      auto *call = get_unique_field_exec(supported_types(), f_type, idx);
      if (!call) {
        ostringstream os;
        auto *str = fm_type_to_str(f_type);
        os << "type " << str << "is not supported in unique feature";
        free(str);
        fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_ARGS, os.str().c_str());
        return nullptr;
      }
      calls.push_back(call);
    } else if (fm_type_is_array(f_type)) {
      auto af_type = fm_type_array_of(f_type);
      if (fm_type_is_base(af_type)) {
        switch (fm_type_base_enum(af_type)) {
        case FM_TYPE_CHAR:
          calls.push_back(new the_unique_field_exec_2_0<char *>(
              idx, fm_type_array_size(f_type)));
          break;
        default:
          ostringstream os;
          auto *str = fm_type_to_str(af_type);
          os << "array of type " << str
             << "is not supported in "
                "unique feature";
          free(str);
          fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_ARGS, os.str().c_str());
          return nullptr;
          break;
        }
      }
    }
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, argv[0]);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_unique_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_unique_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (unique_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
