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
 * @file type_space.hpp
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C++ definition of the type space object
 *
 * This file contains definition of the type space object
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

#pragma once

#include <functional>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

extern "C" {
#include "extractor/type_decl.h"
#include "fmc/extension.h"
}

struct fm_type_decl;

namespace fm {
using namespace std;
struct type_space;

enum FM_TYPE_GROUPS {
  FM_TYPE_GROUP_BASE,
  FM_TYPE_GROUP_ARRAY,
  FM_TYPE_GROUP_FRAME,
  FM_TYPE_GROUP_RECORD,
  FM_TYPE_GROUP_TUPLE,
  FM_TYPE_GROUP_CSTRING,
  FM_TYPE_GROUP_MODULE,
  FM_TYPE_GROUP_TYPE,
};

struct base_type_def {
  base_type_def(FM_BASE_TYPE t);
  static size_t hash(FM_BASE_TYPE t);
  bool equal_to(FM_BASE_TYPE t) const;
  string str() const;
  static const fm_type_decl *try_parse(type_space &s, string_view &buf);
  FM_BASE_TYPE type;
};

struct record_type_def {
  record_type_def(const char *name, size_t size);
  static size_t hash(const char *name, size_t size);
  bool equal_to(const char *name, size_t size) const;
  static const fm_type_decl *try_parse(type_space &s, string_view &buf);
  string str() const;
  std::string name;
  size_t size;
};

struct array_type_def {
  array_type_def(fm_type_decl_cp td, size_t s);
  static size_t hash(fm_type_decl_cp td, size_t s);
  bool equal_to(fm_type_decl_cp td, size_t s) const;
  static const fm_type_decl *try_parse(type_space &s, string_view &buf);
  string str() const;
  fm_type_decl_cp type;
  size_t size;
};

struct frame_type_def {
  frame_type_def(unsigned num, const char *names[], fm_type_decl_cp types[],
                 unsigned nd, int dims[]);
  static size_t hash(unsigned num, const char *names[], fm_type_decl_cp types[],
                     unsigned nd, int dims[]);
  bool equal_to(unsigned num, const char *names[], fm_type_decl_cp types[],
                unsigned nd, int dims[]) const;
  static const fm_type_decl *try_parse(type_space &s, string_view &buf);
  string str() const;
  bool has_field(const std::string &name, fm_type_decl_cp type) const;
  using field_t = std::pair<std::string, fm_type_decl_cp>;
  using fields_t = std::vector<field_t>;
  fields_t fields;
  std::vector<int> dims;
};

struct tuple_type_def {
  tuple_type_def(unsigned num, fm_type_decl_cp types[]);
  static size_t hash(unsigned num, fm_type_decl_cp types[]);
  bool equal_to(unsigned num, fm_type_decl_cp types[]) const;
  std::vector<fm_type_decl_cp> items;
  static const fm_type_decl *try_parse(type_space &s, string_view &buf);
  string str() const;
};

struct cstring_type_def {
  cstring_type_def();
  static size_t hash();
  bool equal_to() const;
  static const fm_type_decl *try_parse(type_space &s, string_view &buf);
  string str() const;
};

struct module_type_def {
  module_type_def(size_t ninps, size_t nouts);
  static size_t hash(size_t ninps, size_t nouts);
  bool equal_to(size_t ninps, size_t nouts) const;
  static const fm_type_decl *try_parse(type_space &s, string_view &buf);
  string str() const;
  size_t ninps;
  size_t nouts;
};

struct type_type_def {
  type_type_def();
  static size_t hash();
  bool equal_to() const;
  static const fm_type_decl *try_parse(type_space &s, string_view &buf);
  string str() const;
};

using type_def = std::variant<base_type_def, record_type_def, array_type_def,
                              frame_type_def, tuple_type_def, cstring_type_def,
                              module_type_def, type_type_def>;
} // namespace fm

struct fm_type_decl {
  template <class T> fm_type_decl(size_t i, size_t h, T &&t);
  std::string str() const {
    return visit([](auto &t) { return t.str(); }, def);
  }
  size_t index;
  size_t hash;
  fm::type_def def;
};

inline bool operator==(const fm_type_decl &a, const fm_type_decl &b) {
  return a.index == b.index;
}

inline bool operator!=(const fm_type_decl &a, const fm_type_decl &b) {
  return a.index != b.index;
}

namespace fm {
struct type_space {
  ~type_space();
  fm_type_decl_cp get_base_type(FM_BASE_TYPE t);
  fm_type_decl_cp get_record_type(const char *name, size_t s);
  fm_type_decl_cp get_array_type(fm_type_decl_cp td, unsigned s);
  fm_type_decl_cp get_frame_type(unsigned num, const char *names[],
                                 fm_type_decl_cp types[], unsigned nd,
                                 int dims[]);
  fm_type_decl_cp get_tuple_type(unsigned num, fm_type_decl_cp types[]);
  fm_type_decl_cp get_cstring_type();
  fm_type_decl_cp get_module_type(unsigned ninps, unsigned nouts);
  fm_type_decl_cp get_type_type();
  const fm_type_decl *get_type_from_str(string_view &view);
  template <class T, class... Args>
  fm_type_decl_cp get_type_decl(Args &&... args);
  std::unordered_multimap<size_t, fm_type_decl_cp> decls;
};

template <class T, class... Args>
fm_type_decl_cp type_space::get_type_decl(Args &&... args) {
  auto hash = T::hash(std::forward<Args>(args)...);
  auto range = decls.equal_range(hash);
  for (auto it = range.first; it != range.second; ++it) {
    if (auto *type = std::get_if<T>(&it->second->def)) {
      if (type->equal_to(std::forward<Args>(args)...)) {
        return it->second;
      }
    }
  }
  auto i = decls.size();
  auto *decl = new fm_type_decl(i, hash, T(std::forward<Args>(args)...));
  return decls.emplace(hash, decl)->second;
}
} // namespace fm
