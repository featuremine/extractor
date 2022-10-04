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
 * @file type_space.cpp
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C++ implementation of the type system library
 *
 * This file contains implementation of the type systems library
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

#include <array>
#include <functional>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "type_space.hpp"
#include "fmc++/mpl.hpp"
#include "fmc/alignment.h"

using namespace fm;
using namespace std;

base_type_def::base_type_def(FM_BASE_TYPE t) : type(t) {}

size_t base_type_def::hash(FM_BASE_TYPE t) {
  return fmc_hash_combine(std::hash<FM_TYPE_GROUPS>{}(FM_TYPE_GROUP_BASE),
                          std::hash<FM_BASE_TYPE>{}(t));
}

bool base_type_def::equal_to(FM_BASE_TYPE t) const { return t == type; }

record_type_def::record_type_def(const char *n, size_t s) : name(n), size(s) {}

size_t record_type_def::hash(const char *n, size_t s) {
  auto seed = std::hash<FM_TYPE_GROUPS>{}(FM_TYPE_GROUP_RECORD);
  seed = fmc_hash_combine(seed, std::hash<string_view>{}(string_view(n)));
  return fmc_hash_combine(seed, std::hash<size_t>{}(s));
}

bool record_type_def::equal_to(const char *n, size_t s) const {
  return name == n && size == s;
}

array_type_def::array_type_def(fm_type_decl_cp td, size_t s)
    : type(td), size(s) {}

size_t array_type_def::hash(fm_type_decl_cp td, size_t s) {
  return fmc_hash_combine(
      std::hash<unsigned>{}(s),
      fmc_hash_combine(std::hash<FM_TYPE_GROUPS>{}(FM_TYPE_GROUP_ARRAY),
                       td->hash));
}

bool array_type_def::equal_to(fm_type_decl_cp td, size_t s) const {
  return td->index == type->index && s == size;
}

frame_type_def::frame_type_def(unsigned num, const char *names[],
                               fm_type_decl_cp types[], unsigned nd, int ds[])
    : dims(ds, ds + nd) {
  for (unsigned i = 0; i < num; ++i) {
    fields.emplace_back(names[i], types[i]);
  }
}

size_t frame_type_def::hash(unsigned num, const char *names[],
                            fm_type_decl_cp types[], unsigned nd, int ds[]) {
  auto seed = std::hash<FM_TYPE_GROUPS>{}(FM_TYPE_GROUP_FRAME);
  for (unsigned i = 0; i < num; ++i) {
    seed =
        fmc_hash_combine(seed, std::hash<string_view>{}(string_view(names[i])));
    seed = fmc_hash_combine(seed, types[i]->hash);
  }
  for (unsigned i = 0; i < nd; ++i) {
    seed = fmc_hash_combine(seed, std::hash<int>{}(ds[i]));
  }
  return seed;
}

bool frame_type_def::equal_to(unsigned num, const char *names[],
                              fm_type_decl_cp types[], unsigned nd,
                              int ds[]) const {
  if (num != fields.size() || nd != dims.size()) {
    return false;
  }
  for (unsigned i = 0; i < num; ++i) {
    if (fields[i].first != names[i] || *fields[i].second != *types[i]) {
      return false;
    }
  }
  for (unsigned i = 0; i < nd; ++i) {
    if (dims[i] != ds[i])
      return false;
  }
  return true;
}

tuple_type_def::tuple_type_def(unsigned num, fm_type_decl_cp types[])
    : items(types, types + num) {}

size_t tuple_type_def::hash(unsigned num, fm_type_decl_cp types[]) {
  auto seed = std::hash<FM_TYPE_GROUPS>{}(FM_TYPE_GROUP_TUPLE);
  for (unsigned i = 0; i < num; ++i) {
    seed = fmc_hash_combine(seed, types[i]->hash);
  }
  return seed;
}

bool tuple_type_def::equal_to(unsigned num, fm_type_decl_cp types[]) const {
  if (items.size() != num)
    return false;
  for (unsigned i = 0; i < num; ++i) {
    if (items[i]->index != types[i]->index)
      return false;
  }
  return true;
}

cstring_type_def::cstring_type_def() {}

size_t module_type_def::hash(size_t ninps, size_t nouts) {
  auto seed = std::hash<FM_TYPE_GROUPS>{}(FM_TYPE_GROUP_MODULE);
  seed = fmc_hash_combine(seed, std::hash<unsigned>{}(ninps));
  return fmc_hash_combine(seed, std::hash<unsigned>{}(nouts));
}

bool module_type_def::equal_to(size_t inps, size_t outs) const {
  if (ninps != inps || nouts != outs)
    return false;
  return true;
}

module_type_def::module_type_def(size_t ninps, size_t nouts)
    : ninps(ninps), nouts(nouts) {}

size_t cstring_type_def::hash() {
  return std::hash<FM_TYPE_GROUPS>{}(FM_TYPE_GROUP_CSTRING);
}

bool cstring_type_def::equal_to() const { return true; }

type_type_def::type_type_def() {}

size_t type_type_def::hash() {
  return std::hash<FM_TYPE_GROUPS>{}(FM_TYPE_GROUP_TYPE);
}

bool type_type_def::equal_to() const { return true; }

string base_type_def::str() const { return fm_base_type_name(type); }

string record_type_def::str() const {
  return "record(" + name + "," + to_string(size) + ")";
}

string array_type_def::str() const {
  return "array(" + type->str() + "," + to_string(size) + ")";
}

string frame_type_def::str() const {
  string s = "frame(";
  bool first = true;
  for (auto field : fields) {
    if (!first)
      s.append(",");
    first = false;
    s.append(field.first);
    s.append(":");
    s.append(field.second->str());
  }
  s.append(")");
  return s;
}

bool frame_type_def::has_field(const std::string &name,
                               fm_type_decl_cp type) const {
  for (auto &field : fields) {
    if (field.first == name && *type == *field.second)
      return true;
  }
  return false;
}

string tuple_type_def::str() const {
  string s = "tuple(";
  bool first = true;
  for (auto *item : items) {
    if (!first)
      s.append(",");
    first = false;
    s.append(item->str());
  }
  s.append(")");
  return s;
}

string cstring_type_def::str() const { return "cstring"; }

string module_type_def::str() const {
  string s = "module(";
  s.append(to_string(ninps));
  s.append(",");
  s.append(to_string(nouts));
  s.append(")");
  return s;
}

string type_type_def::str() const { return "type"; }

pair<string_view, string_view> has_prefix(string_view buf, string_view prefix) {
  return (buf.substr(0, prefix.size()) == prefix)
             ? pair(buf.substr(0, prefix.size()), buf.substr(prefix.size()))
             : pair(string_view(), buf);
}

const fm_type_decl *base_type_def::try_parse(type_space &s, string_view &buf) {
  for (int i = 0; i < FM_TYPE_LAST; ++i) {
    auto em = static_cast<FM_BASE_TYPE>(i);
    auto &&[prefix, rest] = has_prefix(buf, string_view(fm_base_type_name(em)));
    if (!prefix.empty()) {
      buf = rest;
      return s.get_base_type(em);
    }
  }
  return nullptr;
}

static unsigned long find_first_of_parenthesis(std::string_view str,
                                               char expected) {
  auto it = str.begin();
  auto opened = 0u;
  while (it != str.end()) {
    auto c = *it;
    if (c == expected && opened == 0) {
      return it - str.begin();
    } else if (c == '(') {
      ++opened;
    } else if (c == ')') {
      if (opened > 0) {
        --opened;
      } else {
        return std::string_view::npos;
      }
    }
    ++it;
  }
  return std::string_view::npos;
}

const fm_type_decl *record_type_def::try_parse(type_space &s,
                                               string_view &buf) {
  constexpr string_view prefix = "record(";
  auto &&[prf, rest] = has_prefix(buf, prefix);
  if (prf.empty()) {
    return nullptr;
  }
  auto where = find_first_of_parenthesis(rest, ',');
  if (where == string_view::npos || where + 2 > rest.size())
    return nullptr;
  auto name = rest.substr(0, where);
  rest = rest.substr(where + 1);
  char *e = nullptr;
  auto size = strtoul(rest.data(), &e, 10);
  if (e == rest.data() || *e != ')')
    return nullptr;
  buf = rest.substr(e - rest.data() + 1);
  return s.get_record_type(string(name).c_str(), size);
}

const fm_type_decl *array_type_def::try_parse(type_space &s, string_view &buf) {
  constexpr string_view prefix = "array(";
  auto &&[prf, rest] = has_prefix(buf, prefix);
  if (prf.empty()) {
    return nullptr;
  }
  auto td = s.get_type_from_str(rest);
  if (!td || rest.empty() || rest.front() != ',')
    return nullptr;
  rest = rest.substr(1);
  char *e = nullptr;
  auto size = strtoul(rest.data(), &e, 10);
  if (e == rest.data() || *e != ')')
    return nullptr;
  buf = rest.substr(e - rest.data() + 1);
  return s.get_array_type(td, size);
}

const fm_type_decl *frame_type_def::try_parse(type_space &s, string_view &buf) {
  constexpr string_view prefix = "frame(";
  auto &&[prf, rest] = has_prefix(buf, prefix);
  if (prf.empty()) {
    return nullptr;
  }
  std::vector<fm_type_decl_cp> types;
  std::vector<std::string> names_str;
  std::array<int, 1> dims = {1};
  bool done = false;
  bool first = true;
  while (!rest.empty()) {
    if (rest.front() == ')') {
      rest = rest.substr(1);
      done = true;
      break;
    }
    if (!first) {
      if (rest.front() != ',') {
        break;
      }
      rest = rest.substr(1);
    }
    for (auto i = 0u;; ++i) {
      if (i == rest.size()) {
        return nullptr;
      }
      if (rest[i] == ':') {
        names_str.emplace_back(rest.substr(0, i));
        rest = rest.substr(i + 1);
        break;
      }
    }
    first = false;
    auto td = s.get_type_from_str(rest);
    if (!td)
      break;
    types.push_back(td);
  }
  if (!done) {
    return nullptr;
  }
  buf = rest;
  std::vector<const char *> names;
  for (auto &name : names_str) {
    names.push_back(name.c_str());
  }
  return s.get_frame_type(names.size(), names.data(), types.data(), dims.size(),
                          dims.data());
}

const fm_type_decl *tuple_type_def::try_parse(type_space &s, string_view &buf) {
  constexpr string_view prefix = "tuple(";
  auto &&[prf, rest] = has_prefix(buf, prefix);
  if (prf.empty()) {
    return nullptr;
  }
  std::vector<fm_type_decl_cp> types;
  bool done = false;
  bool first = true;
  while (!rest.empty()) {
    if (rest.front() == ')') {
      rest = rest.substr(1);
      done = true;
      break;
    }
    if (!first) {
      if (rest.front() != ',') {
        break;
      }
      rest = rest.substr(1);
    }
    first = false;
    auto td = s.get_type_from_str(rest);
    if (!td)
      break;
    types.push_back(td);
  }
  if (!done) {
    return nullptr;
  }
  buf = rest;
  return s.get_tuple_type(types.size(), types.data());
}

const fm_type_decl *cstring_type_def::try_parse(type_space &s,
                                                string_view &buf) {
  constexpr string_view prefix = "cstring";
  auto &&[prf, rest] = has_prefix(buf, prefix);
  if (prf.empty()) {
    return nullptr;
  }
  buf = rest;
  return s.get_cstring_type();
}

const fm_type_decl *module_type_def::try_parse(type_space &s,
                                               string_view &buf) {
  constexpr string_view prefix = "module(";
  auto &&[prf, rest] = has_prefix(buf, prefix);
  if (prf.empty()) {
    return nullptr;
  }
  char *e = nullptr;
  auto ninps = strtoul(rest.data(), &e, 10);
  if (e == rest.data() || *e != ',')
    return nullptr;
  rest = rest.substr(e - rest.data() + 1);
  auto nouts = strtoul(rest.data(), &e, 10);
  if (e == rest.data() || *e != ')')
    return nullptr;
  buf = rest.substr(e - rest.data() + 1);
  return s.get_module_type(ninps, nouts);
}

const fm_type_decl *type_type_def::try_parse(type_space &s, string_view &buf) {
  constexpr string_view prefix = "type";
  auto &&[prf, rest] = has_prefix(buf, prefix);
  if (prf.empty()) {
    return nullptr;
  }
  buf = rest;
  return s.get_type_type();
}

template <class T>
fm_type_decl::fm_type_decl(size_t i, size_t h, T &&t)
    : index(i), hash(h), def(std::forward<T>(t)) {}

fm_type_decl_cp type_space::get_base_type(FM_BASE_TYPE t) {
  return get_type_decl<base_type_def>(t);
}

fm_type_decl_cp type_space::get_record_type(const char *name, size_t s) {
  return get_type_decl<record_type_def>(name, s);
}

fm_type_decl_cp type_space::get_array_type(fm_type_decl_cp td, unsigned s) {
  return get_type_decl<array_type_def>(td, s);
}

fm_type_decl_cp type_space::get_frame_type(unsigned nf, const char *names[],
                                           fm_type_decl_cp types[], unsigned nd,
                                           int dims[]) {
  return get_type_decl<frame_type_def>(nf, names, types, nd, dims);
}

fm_type_decl_cp type_space::get_tuple_type(unsigned num,
                                           fm_type_decl_cp types[]) {
  return get_type_decl<tuple_type_def>(num, types);
}

fm_type_decl_cp type_space::get_cstring_type() {
  return get_type_decl<cstring_type_def>();
}

fm_type_decl_cp type_space::get_module_type(unsigned ninps, unsigned nouts) {
  return get_type_decl<module_type_def>(ninps, nouts);
}

fm_type_decl_cp type_space::get_type_type() {
  return get_type_decl<type_type_def>();
}

template <class... Types>
const fm_type_decl *try_parse(type_space &ts, string_view &view,
                              fmc::typify<variant<Types...>>) {
  const fm_type_decl *res = nullptr;
  auto do_parse = [&res, &ts, &view](auto tf) {
    using T = typename decltype(tf)::type;
    if (res)
      return;
    res = T::try_parse(ts, view);
  };
  (do_parse(fmc::typify<Types>()), ...);
  return res;
}

const fm_type_decl *type_space::get_type_from_str(string_view &view) {
  return try_parse(*this, view, fmc::typify<fm::type_def>());
}

type_space::~type_space() {
  for (auto it = decls.begin(); it != decls.end(); ++it) {
    delete it->second;
  }
}
