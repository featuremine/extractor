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
 * @file convert.cpp
 * @author Maxim Trokhimtchouk
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "convert.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "extractor/comp_def.hpp"
#include "extractor/decimal64.hpp"
#include "extractor/frame.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/strings.hpp"
#include "fmc++/time.hpp"

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

struct convert_field_exec {
  virtual ~convert_field_exec() {}
  virtual void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
                    fm_exec_ctx_t *) = 0;
};

template <class F, class T>
struct the_convert_field_exec_2_0 : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto val0 = *(const F *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(T *)fm_frame_get_ptr1(result, field_, 0) = T(val0);
  }
  fm_field_t field_;
};

template <class T>
struct the_convert_field_exec_2_0<fm_decimal64_t, T> : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto val0 = *(const fm_decimal64_t *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(T *)fm_frame_get_ptr1(result, field_, 0) =
        T(fm_decimal64_to_double(val0));
  }
  fm_field_t field_;
};

template <class T>
struct the_convert_field_exec_2_0<fmc_decimal128_t, T> : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto val0 =
        *(const fmc_decimal128_t *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(T *)fm_frame_get_ptr1(result, field_, 0) =
        T(fmc::conversion<fmc_decimal128_t, double>()(val0));
  }
  fm_field_t field_;
};

template <class T>
struct the_convert_field_exec_2_0<T, fm_decimal64_t> : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto &val0 = *(const T *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(fm_decimal64_t *)fm_frame_get_ptr1(result, field_, 0) =
        fm_decimal64_from_double(val0);
  }
  fm_field_t field_;
};

template <class T>
struct the_convert_field_exec_2_0<T, fmc_decimal128_t> : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto &val0 = *(const T *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(fmc_decimal128_t *)fm_frame_get_ptr1(result, field_, 0) =
        fmc::conversion<double, fmc_decimal128_t>()(val0);
  }
  fm_field_t field_;
};

template <class T>
struct the_convert_field_exec_2_0<T, fm_rational64_t> : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto val0 = *(const T *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(fm_rational64_t *)fm_frame_get_ptr1(result, field_, 0) =
        fm_rational64_new2(val0, 1);
  }
  fm_field_t field_;
};

template <>
struct the_convert_field_exec_2_0<fm_decimal64_t, fm_rational64_t>
    : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto val0 = *(const fm_decimal64_t *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(fm_rational64_t *)fm_frame_get_ptr1(result, field_, 0) =
        fm_rational64_from_decimal64(val0);
  }
  fm_field_t field_;
};

template <>
struct the_convert_field_exec_2_0<fmc_decimal128_t, fm_rational64_t>
    : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto val0 =
        *(const fmc_decimal128_t *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(fm_rational64_t *)fm_frame_get_ptr1(result, field_, 0) =
        fm_rational64_from_double(
            fmc::conversion<fmc_decimal128_t, double>()(val0), 32);
  }
  fm_field_t field_;
};

template <class T>
struct the_convert_field_exec_2_0<fm_rational64_t, T> : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto val0 =
        *(const fm_rational64_t *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(T *)fm_frame_get_ptr1(result, field_, 0) =
        T(fm_rational64_to_double(val0));
  }
  fm_field_t field_;
};

template <>
struct the_convert_field_exec_2_0<fm_decimal64_t, fmc_decimal128_t>
    : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {
    fmc_decimal128_from_int(&divisor_, DECIMAL64_FRACTION);
  }
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto &val0 =
        *(const fm_decimal64_t *)fm_frame_get_cptr1(argv[0], field_, 0);
    auto *res = (fmc_decimal128_t *)fm_frame_get_ptr1(result, field_, 0);
    fmc_decimal128_from_int(res, val0.value);
    fmc_decimal128_div(res, res, &divisor_);
  }
  fm_field_t field_;
  fmc_decimal128_t divisor_;
};

template <>
struct the_convert_field_exec_2_0<fmc_decimal128_t, fm_decimal64_t>
    : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field) : field_(field) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto &val0 =
        *(const fmc_decimal128_t *)fm_frame_get_cptr1(argv[0], field_, 0);
    *(fm_decimal64_t *)fm_frame_get_ptr1(result, field_, 0) =
        fm_decimal64_from_double(
            fmc::conversion<fmc_decimal128_t, double>()(val0));
  }
  fm_field_t field_;
};

template <class T>
struct the_convert_field_exec_2_0<char *, T> : convert_field_exec {
  the_convert_field_exec_2_0(fm_field_t field, size_t strlen)
      : field_(field), strlen_(strlen) {}
  void exec(fm_frame_t *result, size_t, const fm_frame_t *const argv[],
            fm_exec_ctx_t *ctx) override {
    auto val0 = (const char *)fm_frame_get_cptr1(argv[0], field_, 0);
    auto inp = string_view(val0, strlen_);
    T &res = *(T *)fm_frame_get_ptr1(result, field_, 0);
    auto [r, p] = fmc::from_string_view<T>(inp);
    if (strnlen(val0, strlen_) != p.size()) {
      fm_exec_ctx_error_set(ctx, "Unable to parse value in field %d", field_);
    } else {
      res = r;
    }
  }
  fm_field_t field_;
  size_t strlen_;
};

struct convert_comp_cl {
  ~convert_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<convert_field_exec *> calls;
};

bool fm_comp_convert_call_stream_init(fm_frame_t *result, size_t args,
                                      const fm_frame_t *const argv[],
                                      fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto &calls = (*(convert_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv, ctx->exec);
  }
  return true;
}

bool fm_comp_convert_stream_exec(fm_frame_t *result, size_t args,
                                 const fm_frame_t *const argv[],
                                 fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &calls = (*(convert_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv, ctx->exec);
  }
  return true;
}

fm_call_def *fm_comp_convert_stream_call(fm_comp_def_cl comp_cl,
                                         const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_convert_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_convert_stream_exec);
  return def;
}

template <class... Ts>
convert_field_exec *get_convert_field_exec(fmc::type_list<Ts...>,
                                           fm_type_decl_cp from_type,
                                           fm_type_decl_cp to_type, int idx) {
  convert_field_exec *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    using From = typename Tn::first_type;
    using To = typename Tn::second_type;
    auto obj_to = fm::frame_field_type<To>();
    if constexpr (std::is_same<From, char *>::value) {
      if (!result && fm_type_is_array(from_type) &&
          fm_type_base_enum(fm_type_array_of(from_type)) == FM_TYPE_CHAR &&
          obj_to.validate(to_type)) {
        result = new the_convert_field_exec_2_0<From, To>(
            idx, fm_type_array_size(from_type));
      }
    } else {
      auto obj_from = fm::frame_field_type<From>();
      if (!result && obj_from.validate(from_type) && obj_to.validate(to_type)) {
        result = new the_convert_field_exec_2_0<From, To>(idx);
      }
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}

fm_ctx_def_t *fm_comp_convert_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                  unsigned argc, fm_type_decl_cp argv[],
                                  fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 1) {
    auto *errstr = "expect one operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto *to_type = fm_arg_try_type_decl(fm_type_tuple_arg(ptype, 0), &plist);
  if (!to_type) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expect a type "
                           "parameter");
    return nullptr;
  }

  auto ctx_cl = make_unique<convert_comp_cl>();
  auto &calls = ctx_cl->calls;

  using supported_conversions = fmc::type_list<
      pair<INT8, INT64>, pair<INT16, INT64>, pair<INT32, INT64>,
      pair<INT64, INT8>, pair<INT64, INT16>, pair<INT64, INT32>,
      pair<INT64, INT64>, pair<UINT8, UINT64>, pair<UINT16, UINT64>,
      pair<UINT32, UINT64>, pair<UINT64, UINT64>, pair<INT8, FLOAT64>,
      pair<INT16, FLOAT64>, pair<INT32, FLOAT64>, pair<INT64, FLOAT64>,
      pair<UINT8, FLOAT64>, pair<UINT16, FLOAT64>, pair<UINT32, FLOAT64>,
      pair<UINT64, FLOAT64>, pair<FLOAT32, FLOAT64>, pair<FLOAT64, FLOAT64>,
      pair<INT8, bool>, pair<INT16, bool>, pair<INT32, bool>, pair<INT64, bool>,
      pair<UINT8, bool>, pair<UINT16, bool>, pair<UINT32, bool>,
      pair<UINT64, bool>, pair<FLOAT32, bool>, pair<FLOAT64, bool>,
      pair<bool, INT8>, pair<bool, INT16>, pair<bool, INT32>, pair<bool, INT64>,
      pair<bool, UINT8>, pair<bool, UINT16>, pair<bool, UINT32>,
      pair<bool, UINT64>, pair<bool, FLOAT32>, pair<bool, FLOAT64>,
      pair<DECIMAL64, FLOAT32>, pair<DECIMAL64, FLOAT64>,
      pair<FLOAT32, DECIMAL64>, pair<FLOAT64, DECIMAL64>, pair<INT8, DECIMAL64>,
      pair<INT16, DECIMAL64>, pair<INT32, DECIMAL64>, pair<INT64, DECIMAL64>,
      pair<DECIMAL64, RATIONAL64>, pair<RATIONAL64, FLOAT32>,
      pair<DECIMAL128, FLOAT32>, pair<DECIMAL128, FLOAT64>,
      pair<FLOAT32, DECIMAL128>, pair<FLOAT64, DECIMAL128>,
      pair<INT8, DECIMAL128>, pair<INT16, DECIMAL128>, pair<INT32, DECIMAL128>,
      pair<INT64, DECIMAL128>, pair<DECIMAL128, RATIONAL64>,
      pair<DECIMAL64, DECIMAL128>, pair<DECIMAL128, DECIMAL64>,
      pair<DECIMAL128, INT32>, pair<RATIONAL64, FLOAT64>,
      pair<INT8, RATIONAL64>, pair<INT16, RATIONAL64>, pair<INT32, RATIONAL64>,
      pair<INT64, RATIONAL64>, pair<UINT8, RATIONAL64>,
      pair<UINT16, RATIONAL64>, pair<UINT32, RATIONAL64>,
      pair<UINT64, RATIONAL64>, pair<FLOAT32, RATIONAL64>,
      pair<FLOAT64, RATIONAL64>, pair<char *, INT64>, pair<char *, INT32>,
      pair<char *, INT16>, pair<char *, INT8>, pair<char *, UINT64>,
      pair<char *, UINT32>, pair<char *, UINT16>, pair<char *, UINT8>>;

  auto inp = argv[0];
  int nf = fm_type_frame_nfields(inp);
  vector<const char *> names(nf);
  vector<fm_type_decl_cp> types(nf);
  for (int idx = 0; idx < nf; ++idx) {
    auto from_type = fm_type_frame_field_type(inp, idx);
    names[idx] = fm_type_frame_field_name(inp, idx);
    types[idx] = to_type;
    auto *call = get_convert_field_exec(supported_conversions(), from_type,
                                        to_type, idx);
    if (!call) {
      ostringstream os;
      auto *str_from = fm_type_to_str(from_type);
      auto *str_to = fm_type_to_str(to_type);
      os << "cannot convert from " << str_from << " to " << str_to;
      free(str_from);
      free(str_to);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, os.str().c_str());
      return nullptr;
    }
    calls.push_back(call);
  }

  int nd = fm_type_frame_ndims(inp);
  vector<int> dims(nd);
  for (int idx = 0; idx < nd; ++idx) {
    dims[idx] = fm_type_frame_dim(inp, idx);
  }
  auto type =
      fm_frame_type_get1(sys, nf, names.data(), types.data(), nd, dims.data());
  if (!type) {
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_convert_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_convert_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (convert_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
