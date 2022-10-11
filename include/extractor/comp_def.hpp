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
 * @file comp_def.hpp
 * @author Maxim Trokhimtchouk
 * @date 22 Nov 2017
 * @brief C++ interface for computation definition
 *
 * Here we define C++ interface for computation definition
 */

#pragma once

extern "C" {
#include "extractor/comp_sys.h"
#include "fmc/time.h"
#include "extractor/type_sys.h"
}

#include "extractor/frame.hpp"
#include "fmc++/time.hpp"
#include "fmc++/mpl.hpp"

#include <tuple>
#include <utility>

namespace fm {
using namespace std;

class abstract_computation {
public:
  void set_handle(fm_call_handle_t handle) { handle_ = handle; }
  fm_call_handle_t handle() { return handle_; }

private:
  fm_call_handle_t handle_;
};

class execution_context {
public:
};

class query_context : public execution_context {
public:
  query_context(fm_exec_ctx_p ctx) {}
};

class stream_context : public execution_context {
public:
  stream_context(fm_exec_ctx_p ctx) : ctx_((fm_stream_ctx_t *)ctx) {}
  void queue(abstract_computation *comp) {
    fm_stream_ctx_queue(ctx_, comp->handle());
  }
  void schedule(abstract_computation *comp, fmc_time64_t time) {
    fm_stream_ctx_schedule(ctx_, comp->handle(), time);
  }
  bool scheduled() { return fm_stream_ctx_scheduled(ctx_); }
  bool idle() { return fm_stream_ctx_idle(ctx_); }
  fmc_time64_t now() { return fm_stream_ctx_now(ctx_); }

private:
  fm_stream_ctx_t *ctx_;
};

template <class Ins, class R, bool Inplace = false>
class computation : public abstract_computation {
public:
  using result_t = R;
  using input_t = Ins;
  result_t &result() { return result_; }
  input_t &input() { return input_; }
  template <class Context> bool init(result_t &res, input_t &in, Context &ctx) {
    return true;
  }
  template <class Context> bool exec(result_t &res, input_t &in, Context &ctx) {
    return true;
  }
  static constexpr bool inplace() { return Inplace; }

private:
  result_t result_;
  input_t input_;
};

template <class A, class R, bool Inplace>
fmc::typify<computation<A, R, Inplace>>
_computation_base(computation<A, R, Inplace> &&) {
  return fmc::typify<computation<A, R, Inplace>>();
}

template <class C>
using computation_base_t =
    typename decltype(_computation_base(declval<C>()))::type;

template <class M> struct function_type;

template <class R, class... Args> struct function_type<R(Args...)> {
  enum { value = true };
  using result = R;
  using type = R(Args...);
  using args = std::tuple<Args...>;
};

template <size_t idx, class Tup>
void init_single_arg(Tup &tup, const fm_frame_t *const *argv) {
  get<idx>(tup) =
      tuple_element_t<idx, Tup>(const_cast<fm_frame_t *>(argv[idx]));
}

template <class Tup, size_t... ind>
void init_all_args(integer_sequence<size_t, ind...>, Tup &tup,
                   const fm_frame_t *const argv[]) {
  (init_single_arg<ind>(tup, argv), ...);
}

template <class I, size_t idx> void check_single_arg(fm_type_decl_cp argv[]) {
  tuple_element_t<idx, I>::check_type(argv[idx]);
}

template <class I, size_t... ind>
void check_all_args(integer_sequence<size_t, ind...>, fm_type_decl_cp argv[]) {
  (check_single_arg<I, ind>(argv), ...);
}

template <class T> bool fm_type_is(fm_type_decl_cp td) {
  return fmc::overloaded{
      [td](fmc::typify<int16_t>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_INT16;
      },
      [td](fmc::typify<int32_t>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_INT32;
      },
      [td](fmc::typify<int64_t>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_INT64;
      },
      [td](fmc::typify<uint16_t>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_UINT16;
      },
      [td](fmc::typify<uint32_t>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_UINT32;
      },
      [td](fmc::typify<uint64_t>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_UINT64;
      },
      [td](fmc::typify<float>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_FLOAT32;
      },
      [td](fmc::typify<double>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_FLOAT64;
      },
      [td](fmc::typify<fmc_time64_t>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_TIME64;
      },
      [td](fmc::typify<char>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_CHAR;
      },
      [td](fmc::typify<wchar_t>) {
        return fm_type_is_base(td) && fm_type_base_enum(td) == FM_TYPE_WCHAR;
      },
      [td](fmc::typify<const char *>) { return fm_type_is_cstring(td); },
  }(fmc::typify<T>());
}

template <class Tup, size_t idx>
void set_indexed_tuple_arg(Tup &tup, fm_type_decl_cp ptype,
                           fm_arg_stack_t &plist) {
  using Type = tuple_element_t<idx, Tup>;
  auto td = fm_type_tuple_arg(ptype, idx);

  // @todo need to add type_decl string
  fmc_runtime_error_unless(fm_type_is<Type>(td))
      << "expecting type " << fmc::type_name<Type>() << "for the " << idx
      << " parameter";

  get<idx>(tup) = STACK_POP(plist, Type);
}

template <class Tup, size_t... ind>
Tup parse_indexed_tuple_args(integer_sequence<size_t, ind...>,
                             fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  Tup tup;
  // @note we need to use comma operator to insure correct
  // order of STACK_POP calls
  (set_indexed_tuple_arg<Tup, ind>(tup, ptype, plist), ...);
  return tup;
}

template <class Tup>
Tup parse_tuple_args(fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  constexpr auto paramc = tuple_size<Tup>::value;
  if (paramc == 0) {
    fmc_runtime_error_unless(fm_args_empty(ptype)) << "expecting no "
                                                      "arguments";
    return Tup();
  }
  fmc_runtime_error_unless(fm_type_is_tuple(ptype))
      << "argument type is expected to be a tuple";

  auto actual_paramc = fm_type_tuple_size(ptype);
  fmc_runtime_error_unless(actual_paramc == paramc)
      << "expected " << paramc << " but got " << actual_paramc;

  using index = make_index_sequence<tuple_size<Tup>::value>;
  return parse_indexed_tuple_args<Tup>(index{}, ptype, plist);
}

template <class C>
using arg_tuple_t = typename function_type<decltype(C::create)>::args;

template <class C> struct comp_arg_init;

template <class R, bool Inplace, class... I>
struct comp_arg_init<computation<tuple<I...>, R, Inplace>> {
  using Comp = computation<tuple<I...>, R, Inplace>;
  void operator()(Comp &comp, size_t argc, const fm_frame_t *const argv[]) {
    constexpr auto expect_argc = tuple_size<typename Comp::input_t>::value;
    using index = make_index_sequence<expect_argc>;
    init_all_args(index{}, comp.input(), argv);
  }
};

template <class I, class R, bool Inplace>
struct comp_arg_init<computation<vector<I>, R, Inplace>> {
  using Comp = computation<vector<I>, R, Inplace>;
  void operator()(Comp &comp, size_t argc, const fm_frame_t *const argv[]) {
    for (unsigned i = 0; i < argc; ++i) {
      comp.input().push_back(const_cast<fm_frame_t *>(argv[i]));
    }
  }
};

template <class C, class Context>
bool fm_cpp_comp_init(fm_frame_t *result, size_t argc,
                      const fm_frame_t *const argv[], fm_call_ctx_t *ctx,
                      fm_call_exec_cl *cl) {
  auto *params = (arg_tuple_t<C> *)ctx->comp;
  C *exec_cl = nullptr;
  try {
    exec_cl = apply(&C::create, *params);
    exec_cl->result() = typename C::result_t(result);

    comp_arg_init<computation_base_t<C>>{}(*exec_cl, argc, argv);

    exec_cl->set_handle(ctx->handle);
    Context context(ctx->exec);
    fmc_runtime_error_unless(exec_cl->init(context)) << "could not "
                                                        "initialize";
    *cl = exec_cl;
    return true;
  } catch (const runtime_error &e) {
    fm_exec_ctx_error_set(ctx->exec, "error initializing computation %s",
                          e.what());
    return false;
  }
}

template <class C, class Context>
bool fm_cpp_comp_exec(fm_frame_t *result, size_t,
                      const fm_frame_t *const argv[], fm_call_ctx_t *ctx,
                      fm_call_exec_cl cl) {
  C *exec_cl = (C *)cl;
  Context context(ctx->exec);
  return exec_cl->exec(context);
}

template <class C> void fm_cpp_comp_destroy(fm_call_exec_cl cl) {
  C *exec_cl = (C *)cl;
  if (exec_cl)
    delete exec_cl;
}

template <class C, class Context>
fm_call_def *fm_cpp_comp_call(fm_comp_def_cl comp_cl,
                              const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, &fm_cpp_comp_init<C, Context>);
  fm_call_def_destroy_set(def, &fm_cpp_comp_destroy<C>);
  fm_call_def_exec_set(def, &fm_cpp_comp_exec<C, Context>);
  return def;
}

template <class C> struct comp_arg_check;

template <class I, class R, bool Inplace>
struct comp_arg_check<computation<vector<I>, R, Inplace>> {
  bool operator()(fm_type_sys_t *sys, unsigned argc, fm_type_decl_cp argv[]) {
    for (unsigned i = 0; i < argc; ++i) {
      try {
        I::check_type(argv[i]);
      } catch (const runtime_error &e) {
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, e.what());
        return false;
      }
    }
    return true;
  }
};

template <class R, class... I, bool Inplace>
struct comp_arg_check<computation<tuple<I...>, R, Inplace>> {
  using Comp = computation<tuple<I...>, R, Inplace>;
  bool operator()(fm_type_sys_t *sys, unsigned argc, fm_type_decl_cp argv[]) {
    constexpr auto expect_argc = tuple_size<typename Comp::input_t>::value;
    if (expect_argc != argc) {
      ostringstream ostr;
      ostr << "expected " << expect_argc << " operator argument, got " << argc;
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, ostr.str().c_str());
      return false;
    }

    try {
      using index = make_index_sequence<expect_argc>;
      check_all_args<typename Comp::input_t>(index{}, argv);
    } catch (const runtime_error &e) {
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, e.what());
      return false;
    }
    return true;
  }
};

template <class C>
fm_ctx_def_t *fm_cpp_comp_generate(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
                                   fm_type_decl_cp ptype,
                                   fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  if (!comp_arg_check<computation_base_t<C>>{}(sys, argc, argv))
    return nullptr;

  arg_tuple_t<C> *ctx = nullptr;
  try {
    auto args = parse_tuple_args<arg_tuple_t<C>>(ptype, plist);
    ctx = new arg_tuple_t<C>(args);
  } catch (const runtime_error &e) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, e.what());
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, C::inplace());
  fm_ctx_def_type_set(def, C::result_t::type_decl(sys));
  fm_ctx_def_closure_set(def, ctx);
  fm_ctx_def_stream_call_set(def, &fm_cpp_comp_call<C, stream_context>);
  fm_ctx_def_query_call_set(def, &fm_cpp_comp_call<C, query_context>);
  return def;
}

template <class C>
void fm_cpp_comp_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx = (arg_tuple_t<C> *)fm_ctx_def_closure(def);
  if (ctx != nullptr)
    delete ctx;
}

template <class C>
bool fm_cpp_comp_type_add(fm_comp_sys_t *sys, const char *name,
                          fm_comp_def_cl closure = NULL) {
  fm_comp_def_t def = {name, &fm_cpp_comp_generate<C>, &fm_cpp_comp_destroy<C>,
                       closure};
  return fm_comp_type_add(sys, &def);
}
} // namespace fm
