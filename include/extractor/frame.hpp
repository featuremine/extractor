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
 * @file frame.hpp
 * @author Maxim Trokhimtchouk
 * @date 22 Nov 2017
 * @brief C++ interface for frames
 *
 * This is a C++ interface for frames
 */

#pragma once

extern "C" {
#include "extractor/comp_sys.h"
#include "extractor/frame.h"
#include "extractor/type_sys.h"
}

#include <array>
#include <fmc++/mpl.hpp>

namespace fm {
using namespace std;

template <class C> struct mutable_return { using type = C &; };

template <unsigned I, class C> struct mutable_return<C[I]> {
  using type = C *;
};

template <class C> using mutable_return_t = typename mutable_return<C>::type;

template <size_t N> struct frame_items {
  template <class T>
  static T &get(fm_frame_t *frame, fm_field_t off, const array<int, N> &idx);
};

template <> struct frame_items<1> {
  template <class T>
  static T &get(fm_frame_t *frame, fm_field_t off, const array<int, 1> &idx) {
    return *(T *)fm_frame_get_ptr1(frame, off, idx[0]);
  }
};

template <class T> struct frame_field_type;

template <FM_BASE_TYPE TYPE> struct frame_field_base_type {
  fm_type_decl_cp fm_type(fm_type_sys_t *ts) {
    return fm_base_type_get(ts, TYPE);
  }
  bool validate(fm_type_decl_cp td) {
    return fm_type_is_base(td) && fm_type_base_enum(td) == TYPE;
  }
};

#define FRAME_FIELD_TYPE_ENUM(type, val)                                       \
  template <> struct frame_field_type<type> : frame_field_base_type<val> {};

FRAME_FIELD_TYPE_ENUM(uint8_t, FM_TYPE_UINT8);
FRAME_FIELD_TYPE_ENUM(uint16_t, FM_TYPE_UINT16);
FRAME_FIELD_TYPE_ENUM(uint32_t, FM_TYPE_UINT32);
FRAME_FIELD_TYPE_ENUM(uint64_t, FM_TYPE_UINT64);
FRAME_FIELD_TYPE_ENUM(int8_t, FM_TYPE_INT8);
FRAME_FIELD_TYPE_ENUM(int16_t, FM_TYPE_INT16);
FRAME_FIELD_TYPE_ENUM(int32_t, FM_TYPE_INT32);
FRAME_FIELD_TYPE_ENUM(int64_t, FM_TYPE_INT64);
FRAME_FIELD_TYPE_ENUM(double, FM_TYPE_FLOAT64);
FRAME_FIELD_TYPE_ENUM(float, FM_TYPE_FLOAT32);
FRAME_FIELD_TYPE_ENUM(fm_rational64_t, FM_TYPE_RATIONAL64);
FRAME_FIELD_TYPE_ENUM(fm_decimal64_t, FM_TYPE_DECIMAL64);
FRAME_FIELD_TYPE_ENUM(fm_time64_t, FM_TYPE_TIME64);
FRAME_FIELD_TYPE_ENUM(wchar_t, FM_TYPE_WCHAR);
FRAME_FIELD_TYPE_ENUM(char, FM_TYPE_CHAR);
FRAME_FIELD_TYPE_ENUM(bool, FM_TYPE_BOOL);

template <unsigned I, class C> struct frame_field_type<C[I]> {
  fm_type_decl_cp fm_type(fm_type_sys_t *ts) {
    return fm_array_type_get(ts, frame_field_type<C>().fm_type(ts), I);
  }
  bool validate(fm_type_decl_cp td) {
    return fm_type_is_array(td) && fm_type_array_size(td) == I &&
           frame_field_type<C>().validate(fm_type_array_of(td));
  }
};

inline string fm_type_to_string(fm_type_decl_cp type) {
  char *type_cstr = fm_type_to_str(type);
  fmc_runtime_error_unless(type_cstr) << "expecting a proper type name";
  string type_str(type_cstr);
  free(type_cstr);
  return type_str;
}

template <class T>
fm_field_t frame_type_field_get(fm_type_decl_cp td, const char *name) {
  auto off = fm_type_frame_field_idx(td, name);
  fmc_runtime_error_unless(off >= 0) << "no field with name " << name;
  auto type = fm_type_frame_field_type(td, off);
  fmc_runtime_error_unless(frame_field_type<T>().validate(type))
      << "the expected type " << fmc::type_name<T>()
      << " of the field does not match actual " << fm_type_to_string(type);
  return off;
}

template <class T>
fm_field_t frame_field_get(fm_frame_t *frame, const char *name) {
  return frame_type_field_get<T>(fm_frame_type(frame), name);
}

template <class... Fs> struct _fields_t : public Fs... {
  _fields_t() = default;
  _fields_t(fm_frame_t *frame) : Fs(frame)..., frame_(frame) {}
  static void check_type(fm_type_decl_cp type) {
    fmc_runtime_error_unless(fm_type_is_frame(type)) << "expecting a frame";
    (Fs::check_type(type), ...);
  }
  template <int N, class... Ind> void resize(Ind... ind) {
    static_assert(N == tuple_size<tuple<Ind...>>::value);
    fm_frame_reserve(frame_, static_cast<int>(ind)...);
  }
  fm_frame_t *frame_ = nullptr;
};

template <int N, class... Fs> struct _slices_t : public Fs::slice_t... {
  enum { DIMS = N };
  _slices_t() = default;
  _slices_t(array<int, N> idx, _fields_t<Fs...> &fields)
      : fields_(fields), idx_(idx) {}
  static fm_type_decl_cp type_decl(fm_type_sys_t *ts) {
    constexpr auto nf = tuple_size<tuple<Fs...>>::value;
    array<const char *, nf> names{Fs::name()...};
    array<fm_type_decl_cp, nf> types{Fs::type_decl(ts)...};
    array<int, N> dims = {1};
    return fm_frame_type_get1(ts, nf, names.data(), types.data(), N,
                              dims.data());
  }
  _fields_t<Fs...> &fields_;
  array<int, N> idx_;
};

#define FIELD(_field_name_, type)                                              \
  struct _field_name_ {                                                        \
    struct slice_t {                                                           \
      fm::mutable_return_t<type> _field_name_() {                              \
        auto *slices = static_cast<slices_t *>(this);                          \
        auto &fields = slices->fields_;                                        \
        auto *frame = fields.frame_;                                           \
        auto off = slices->fields_._field_name_::offset_;                      \
        return fm::frame_items<slices_t::DIMS>::get<type>(frame, off,          \
                                                          slices->idx_);       \
      }                                                                        \
    };                                                                         \
    _field_name_() = default;                                                  \
    _field_name_(fm_frame_t *f)                                                \
        : offset_(fm::frame_field_get<type>(f, name())) {}                     \
    static fm_type_decl_cp type_decl(fm_type_sys_t *ts) {                      \
      return fm::frame_field_type<type>().fm_type(ts);                         \
    }                                                                          \
    static void check_type(fm_type_decl_cp td) {                               \
      fm::frame_type_field_get<type>(td, name());                              \
    }                                                                          \
    static const char *name() { return #_field_name_; }                        \
    fm_field_t offset_ = -1;                                                   \
  }

#define FIELDS(...)                                                            \
  using slices_t = fm::_slices_t<DIMS, __VA_ARGS__>;                           \
  using fields_t = fm::_fields_t<__VA_ARGS__>;                                 \
                                                                               \
private:                                                                       \
  fields_t fields_;                                                            \
                                                                               \
public:                                                                        \
  template <class... Args> slices_t operator[](Args... args) {                 \
    array<int, DIMS> idx = {args...};                                          \
    return slices_t(idx, fields_);                                             \
  }

#define FRAME(name, dims)                                                      \
  class name {                                                                 \
  public:                                                                      \
    enum { DIMS = dims }

#define END_FRAME(name)                                                        \
  static void check_type(fm_type_decl_cp td) {                                 \
    return fields_t::check_type(td);                                           \
  }                                                                            \
  name() = default;                                                            \
  name(fm_frame_t *frame) : fields_(frame) {}                                  \
  static fm_type_decl_cp type_decl(fm_type_sys_t *ts) {                        \
    return slices_t::type_decl(ts);                                            \
  }                                                                            \
  template <class... Ind> void resize(Ind... ind) {                            \
    fields_.resize<DIMS>(ind...);                                              \
  }                                                                            \
  }

/*
 * @note
 * FRAME(some_frame_type, 1);
 *     FIELD(some_field_name, double);
 *     FIELD(some_other_field_name, double);
 *     FIELDS(some_field_name, some_other_field_name);
 * END_FRAME(some_frame_type);
 *
 * some_frame_type some_frame(frame_ptr);
 *
 * some_frame[0].bidprice();
 */

} // namespace fm
