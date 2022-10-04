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
 * @file constant.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "constant.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include "op_util.hpp"
#include <cassert>
#include <functional>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include <fmc++/strings.hpp>

using namespace std;

template <class T> struct the_constant_field_exec_2_0 : op_field_exec {
  the_constant_field_exec_2_0(fm_field_t field, fm_type_decl_cp arg_type,
                              fm_arg_stack_t &plist)
      : field_(field) {
    if constexpr (is_floating_point_v<T>) {
      double val;
      fmc_runtime_error_unless(fm_arg_try_float64(arg_type, &plist, &val))
          << "could not read a float value";
      val_ = val;
    } else if constexpr (is_signed_v<T>) {
      int64_t val;
      fmc_runtime_error_unless(fm_arg_try_integer(arg_type, &plist, &val))
          << "could not read a signed value";
      fmc_runtime_error_unless(val <= numeric_limits<T>::max() &&
                               val >= numeric_limits<T>::min())
          << "value " << val << " outside of range";
      val_ = val;
    } else if constexpr (is_unsigned_v<T>) {
      if (uint64_t val = 0; fm_arg_try_uinteger(arg_type, &plist, &val)) {
        fmc_runtime_error_unless(val <= numeric_limits<T>::max() &&
                                 val >= numeric_limits<T>::min())
            << "value " << val << " outside of range";
        val_ = val;
        return;
      }
      int64_t val;
      fmc_runtime_error_unless(fm_arg_try_integer(arg_type, &plist, &val))
          << "could not read a unsigned value";
      fmc_runtime_error_unless(val >= 0 && (T)val <= numeric_limits<T>::max())
          << "value " << val << " outside of range";
      val_ = val;
    } else {
      val_ = STACK_POP(plist, T);
    }
  }
  the_constant_field_exec_2_0(fm_field_t field, fm_arg_stack_t &plist)
      : field_(field) {
    val_ = STACK_POP(plist, T);
  }

  void exec(fm_frame_t *result, size_t args,
            const fm_frame_t *const argv[]) override {
    *(T *)fm_frame_get_ptr1(result, field_, 0) = val_;
  }

  fm_field_t field_;
  T val_;
};

template <> struct the_constant_field_exec_2_0<char *> : op_field_exec {
  the_constant_field_exec_2_0(fm_field_t field, size_t str_len,
                              fm_arg_stack_t &plist)
      : field_(field), len_(str_len), val_(STACK_POP(plist, const char *)) {}

  void exec(fm_frame_t *result, size_t args,
            const fm_frame_t *const argv[]) override {
    strncpy((char *)fm_frame_get_ptr1(result, field_, 0), val_.c_str(), len_);
  }

  fm_field_t field_;
  size_t len_;
  string val_;
};

template <> struct the_constant_field_exec_2_0<DECIMAL64> : op_field_exec {
  the_constant_field_exec_2_0(fm_field_t field, fm_type_decl_cp arg_type,
                              fm_arg_stack_t &plist)
      : field_(field) {
    if (fm_type_is_decimal(arg_type)) {
      val_ = STACK_POP(plist, DECIMAL64);
    } else {
      double val;
      fmc_runtime_error_unless(fm_arg_try_float64(arg_type, &plist, &val))
          << "could not read a float value";
      val_ = fm_decimal64_from_double(val);
    }
  }

  void exec(fm_frame_t *result, size_t args,
            const fm_frame_t *const argv[]) override {
    *(DECIMAL64 *)fm_frame_get_ptr1(result, field_, 0) = val_;
  }

  fm_field_t field_;
  DECIMAL64 val_;
};

struct constant_comp_cl {
  ~constant_comp_cl() {
    for (auto *ptr : calls) {
      delete ptr;
    }
  }
  vector<op_field_exec *> calls;
};

bool fm_comp_constant_call_stream_init(fm_frame_t *result, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx,
                                       fm_call_exec_cl *cl) {
  auto &calls = (*(constant_comp_cl *)ctx->comp).calls;
  for (auto &call : calls) {
    call->exec(result, args, argv);
  }
  return true;
}

bool fm_comp_constant_stream_exec(fm_frame_t *result, size_t,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  return true;
}

fm_call_def *fm_comp_constant_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_constant_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_constant_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_constant_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
                                   fm_type_decl_cp ptype,
                                   fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  auto error = [sys](const char *msg) {
    string errstr = msg;
    errstr.append("\nthe constant feature expects field descriptions as "
                  "the arguments, each field description being a tuple"
                  "\neach field description tuple is expected to have 3 "
                  "elements: (field_name, field_type, field_value)");
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr.c_str());
    return nullptr;
  };

  if (fm_args_empty(ptype)) {
    return error("result frame description of the constant feature must be "
                 "provided.");
  };

  auto size = fm_type_tuple_size(ptype);
  auto ctx_cl = make_unique<constant_comp_cl>();
  auto &calls = ctx_cl->calls;

  vector<const char *> names(size);
  vector<fm_type_decl_cp> types(size);
  int dims[1] = {1};

  auto field_error = [error](size_t field_idx, string str) {
    string errstr = str;
    errstr.append(" for field ");
    errstr.append(to_string(field_idx));
    return error(errstr.c_str());
  };

  for (unsigned i = 0; i < size; ++i) {
    auto tuple_arg = fm_type_tuple_arg(ptype, i);
    if (!fm_type_is_tuple(tuple_arg)) {
      return field_error(i, "invalid field description");
    };

    auto field_tuple_size = fm_type_tuple_size(tuple_arg);
    if (field_tuple_size != 3) {
      string errstr = "invalid field description size ";
      errstr.append(to_string(field_tuple_size));
      errstr.append("; expected 3");
      return field_error(i, errstr);
    };

    if (!fm_type_is_cstring(fm_type_tuple_arg(tuple_arg, 0))) {
      return field_error(i, "first element of field description tuple "
                            "must be the field name");
    };
    names[i] = STACK_POP(plist, const char *);

    if (!fm_type_is_type(fm_type_tuple_arg(tuple_arg, 1))) {
      return field_error(i, "second element of field description tuple "
                            "must be of type type");
    };
    types[i] = STACK_POP(plist, fm_type_decl_cp);

    auto arg_type = fm_type_tuple_arg(tuple_arg, 2);
    if (!fm_type_is_base(arg_type) && !fm_type_is_cstring(arg_type)) {
      return field_error(i, "third element of field description tuple "
                            "must be the field type");
    }

    if (!fm_type_equal(types[i], arg_type) &&
        !(fm_type_is_signed(types[i]) && fm_type_is_signed(arg_type)) &&
        !(fm_type_is_unsigned(types[i]) && fm_type_is_unsigned(arg_type)) &&
        !(fm_type_is_unsigned(types[i]) && fm_type_is_signed(arg_type)) &&
        !(fm_type_is_float(types[i]) && fm_type_is_float(arg_type)) &&
        !(fm_type_is_decimal(types[i]) && fm_type_is_float(arg_type)) &&
        !(fm_type_is_cstring(arg_type) &&
          (fm_type_is_array(types[i]) &&
           fm_type_base_enum(fm_type_array_of(types[i])) == FM_TYPE_CHAR))) {
      return field_error(i, "field type described must correspond to the "
                            "type passed in args");
    }
    if (fm_type_is_base(types[i])) {
      auto em = fm_type_base_enum(types[i]);
      try {
        switch (em) {
        case FM_TYPE_INT8:
          calls.push_back(
              new the_constant_field_exec_2_0<int8_t>(i, arg_type, plist));
          break;
        case FM_TYPE_INT16:
          calls.push_back(
              new the_constant_field_exec_2_0<int16_t>(i, arg_type, plist));
          break;
        case FM_TYPE_INT32:
          calls.push_back(
              new the_constant_field_exec_2_0<int32_t>(i, arg_type, plist));
          break;
        case FM_TYPE_INT64:
          calls.push_back(
              new the_constant_field_exec_2_0<int64_t>(i, arg_type, plist));
          break;
        case FM_TYPE_UINT8:
          calls.push_back(
              new the_constant_field_exec_2_0<uint8_t>(i, arg_type, plist));
          break;
        case FM_TYPE_UINT16:
          calls.push_back(
              new the_constant_field_exec_2_0<uint16_t>(i, arg_type, plist));
          break;
        case FM_TYPE_UINT32:
          calls.push_back(
              new the_constant_field_exec_2_0<uint32_t>(i, arg_type, plist));
          break;
        case FM_TYPE_UINT64:
          calls.push_back(
              new the_constant_field_exec_2_0<uint64_t>(i, arg_type, plist));
          break;
        case FM_TYPE_FLOAT32:
          calls.push_back(
              new the_constant_field_exec_2_0<float>(i, arg_type, plist));
          break;
        case FM_TYPE_FLOAT64:
          calls.push_back(
              new the_constant_field_exec_2_0<double>(i, arg_type, plist));
          break;
        case FM_TYPE_DECIMAL64:
          calls.push_back(
              new the_constant_field_exec_2_0<DECIMAL64>(i, arg_type, plist));
          break;
        case FM_TYPE_TIME64:
          calls.push_back(new the_constant_field_exec_2_0<TIME64>(i, plist));
          break;
        case FM_TYPE_CHAR:
          calls.push_back(new the_constant_field_exec_2_0<char>(i, plist));
          break;
        case FM_TYPE_WCHAR:
          calls.push_back(new the_constant_field_exec_2_0<wchar_t>(i, plist));
          break;
        case FM_TYPE_BOOL:
          calls.push_back(new the_constant_field_exec_2_0<bool>(i, plist));
          break;
        case FM_TYPE_LAST:
          break;
        default: {
          string errstr = "invalid base value type ";
          auto *str = fm_type_to_str(types[i]);
          errstr.append(str);
          free(str);
          return field_error(i, errstr);
        } break;
        }
      } catch (exception &e) {
        return field_error(i, e.what());
      }
    } else if ((fm_type_is_cstring(arg_type) ||
                (fm_type_is_array(arg_type) &&
                 fm_type_base_enum(arg_type) == FM_TYPE_CHAR)) &&
               (fm_type_is_array(types[i]) &&
                fm_type_base_enum(fm_type_array_of(types[i])) ==
                    FM_TYPE_CHAR)) {
      auto val = make_unique<the_constant_field_exec_2_0<char *>>(
          i, fm_type_array_size(types[i]), plist);
      if (val->val_.size() > val->len_) {
        string errstr = "the size of the string provided in field value does "
                        "not match the size specified by the type; expected ";
        errstr.append(to_string(val->len_));
        return field_error(i, errstr);
      };
      calls.push_back(val.release());
    } else {
      string errstr = "invalid value type ";
      auto *str = fm_type_to_str(types[i]);
      errstr.append(str);
      free(str);
      errstr.append(" described by type ");
      auto *desc_str = fm_type_to_str(arg_type);
      errstr.append(desc_str);
      free(desc_str);
      return field_error(i, errstr);
    }
  }

  auto *type =
      fm_frame_type_get1(sys, size, names.data(), types.data(), 1, dims);
  if (!type) {
    const char *errstr = "unable to build frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_constant_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_constant_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (constant_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
