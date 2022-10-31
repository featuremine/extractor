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
 * @file types.cpp
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C++ implementation of the type system library
 *
 * This file contains implementation of the type systems library
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/type_sys.h"
#include "extractor/module.h"
}

#include "fmc++/mpl.hpp"
#include "type_error.hpp"
#include "type_space.hpp"

#include <algorithm>
#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

using namespace std;

struct fm_type_io {
  using parser =
      std::function<const char *(const char *, const char *, void *)>;
  using fwriter = std::function<bool(FILE *, const void *)>;
  fm_type_io()
      : parse([](const char *a, const char *, void *) { return a; }),
        fwrite([](FILE *, const void *) { return false; }) {}
  fm_type_io(parser &&p, fwriter &&f)
      : parse(forward<parser>(p)), fwrite(forward<fwriter>(f)) {}
  parser parse;
  fwriter fwrite;
};

struct fm_type_sys {
  fm::type_space type_space;
  FM_TYPE_ERROR errnum = FM_TYPE_ERROR_OK;
  std::string errmsg;
  std::unordered_map<size_t, std::unique_ptr<fm_type_io>> parsers_;
};

fm_type_sys_t *fm_type_sys_new() {
  auto *ts = new fm_type_sys();
  return ts;
}

void fm_type_sys_del(fm_type_sys_t *ts) { delete ts; }

FM_TYPE_ERROR fm_type_sys_errno(fm_type_sys_t *ts) { return ts->errnum; }

void fm_type_sys_err_set(fm_type_sys_t *ts, FM_TYPE_ERROR errnum) {
  ts->errnum = errnum;
  ts->errmsg.clear();
}

void fm_type_sys_err_custom(fm_type_sys_t *ts, FM_TYPE_ERROR errnum,
                            const char *errstr) {
  ts->errnum = errnum;
  ts->errmsg = errstr;
}

void fm_type_sys_err_clear(fm_type_sys_t *ts) {
  fm_type_sys_err_set(ts, FM_TYPE_ERROR_OK);
}

const char *fm_type_sys_errmsg(fm_type_sys_t *ts) {
  return ts->errmsg.empty() ? FM_ERROR_MESSAGES[ts->errnum]
                            : ts->errmsg.c_str();
}

bool fm_type_equal(fm_type_decl_cp a, fm_type_decl_cp b) { return *a == *b; }

fm_type_decl_cp fm_base_type_get(fm_type_sys_t *ts, FM_BASE_TYPE t) {
  fm_type_sys_err_clear(ts);
  return ts->type_space.get_base_type(t);
}

fm_type_decl_cp fm_record_type_get(fm_type_sys_t *ts, const char *name,
                                   size_t s) {
  fm_type_sys_err_clear(ts);
  return ts->type_space.get_record_type(name, s);
}

fm_type_decl_cp fm_array_type_get(fm_type_sys_t *ts, fm_type_decl_cp td,
                                  unsigned s) {
  fm_type_sys_err_clear(ts);
  if (fm_type_is_frame(td)) {
    fm_type_sys_err_set(ts, FM_TYPE_ERROR_CHILD);
    return nullptr;
  }
  return ts->type_space.get_array_type(td, s);
}

// @note probably can be improved somewhat to eliminate extra stack use
bool prepare_frame_fields(unsigned n, const char *names[],
                          fm_type_decl_cp types[]) {
  vector<unsigned> idx(n);
  vector<const char *> s_names(n);
  vector<fm_type_decl_cp> s_types(n);
  for (unsigned i = 0; i < n; ++i)
    idx[i] = i;
  std::sort(std::begin(idx), std::end(idx), [&](unsigned a, unsigned b) {
    return strcmp(names[a], names[b]) < 0;
  });
  for (unsigned i = 0; i < n; ++i) {
    auto j = idx[i];
    s_names[i] = names[j];
    s_types[i] = types[j];
  }
  for (unsigned i = 0; i < n; ++i) {
    names[i] = s_names[i];
    types[i] = s_types[i];
  }
  for (unsigned i = 0; i + 1 < n; ++i) {
    if (strcmp(names[i], names[i + 1]) == 0)
      return false;
  }
  return true;
}

fm_type_decl_cp fm_frame_type_get(fm_type_sys_t *ts, unsigned nf, unsigned nd,
                                  ...) {
  vector<const char *> names(nf);
  vector<fm_type_decl_cp> types(nf);
  vector<int> dims(nd);
  va_list args;

  va_start(args, nd);
  for (unsigned i = 0; i < nf; ++i) {
    names[i] = va_arg(args, const char *);
    types[i] = va_arg(args, fm_type_decl_cp);
  }
  for (unsigned i = 0; i < nd; ++i) {
    dims[i] = va_arg(args, int);
  }
  va_end(args);
  return fm_frame_type_get1(ts, nf, names.data(), types.data(), nd,
                            dims.data());
}

fm_type_decl_cp fm_frame_type_get1(fm_type_sys_t *ts, unsigned nf,
                                   const char *fnames[],
                                   fm_type_decl_cp ftypes[], unsigned nd,
                                   int dims[]) {
  fm_type_sys_err_clear(ts);
  FM_TYPE_ERROR errnum = FM_TYPE_ERROR_OK;
  vector<const char *> names(nf);
  vector<fm_type_decl_cp> types(nf);

  for (unsigned i = 0; i < nf; ++i) {
    names[i] = fnames[i];
    types[i] = ftypes[i];
    if (!fm_type_is_simple(types[i])) {
      errnum = FM_TYPE_ERROR_CHILD;
      goto error;
    }
  }
  if (!prepare_frame_fields(nf, names.data(), types.data())) {
    errnum = FM_TYPE_ERROR_DUPLICATE;
    goto error;
  }
  for (unsigned i = 0; i < nd; ++i) {
    if (dims[i] < 0) {
      errnum = FM_TYPE_ERROR_DIM;
      goto error;
    }
  }
  return ts->type_space.get_frame_type(nf, names.data(), types.data(), nd,
                                       dims);

error:
  fm_type_sys_err_set(ts, errnum);
  return nullptr;
}

fm_type_decl_cp fm_tuple_type_get(fm_type_sys_t *ts, unsigned num, ...) {
  vector<fm_type_decl_cp> types(num);
  va_list args;

  va_start(args, num);
  for (unsigned i = 0; i < num; ++i) {
    types[i] = va_arg(args, fm_type_decl_cp);
  }
  return fm_tuple_type_get1(ts, num, types.data());
}

fm_type_decl_cp fm_tuple_type_get1(fm_type_sys_t *ts, unsigned num,
                                   fm_type_decl_cp types[]) {
  return ts->type_space.get_tuple_type(num, types);
}

fm_type_decl_cp fm_cstring_type_get(fm_type_sys_t *ts) {
  return ts->type_space.get_cstring_type();
}

fm_type_decl_cp fm_module_type_get(fm_type_sys_t *ts, unsigned ninps,
                                   unsigned nouts) {
  return ts->type_space.get_module_type(ninps, nouts);
}

fm_type_decl_cp fm_type_type_get(fm_type_sys_t *ts) {
  return ts->type_space.get_type_type();
}

bool fm_type_is_base(fm_type_decl_cp td) {
  return std::holds_alternative<fm::base_type_def>(td->def);
}

bool fm_type_is_record(fm_type_decl_cp td) {
  return std::holds_alternative<fm::record_type_def>(td->def);
}

bool fm_type_is_array(fm_type_decl_cp td) {
  return std::holds_alternative<fm::array_type_def>(td->def);
}

bool fm_type_is_simple(fm_type_decl_cp td) {
  if (fm_type_is_base(td) || fm_type_is_record(td)) {
    return true;
  } else if (fm_type_is_array(td)) {
    return fm_type_is_simple(fm_type_array_of(td));
  }
  return false;
}

bool fm_type_is_unsigned(fm_type_decl_cp td) {
  if (!td)
    return false;
  if (auto pval = std::get_if<fm::base_type_def>(&td->def)) {
    switch (pval->type) {
    case FM_TYPE_INT8:
    case FM_TYPE_INT16:
    case FM_TYPE_INT32:
    case FM_TYPE_INT64:
      return false;
      break;
    case FM_TYPE_UINT8:
    case FM_TYPE_UINT16:
    case FM_TYPE_UINT32:
    case FM_TYPE_UINT64:
      return true;
      break;
    case FM_TYPE_FLOAT32:
    case FM_TYPE_FLOAT64:
      return false;
      break;
    default:
      return false;
      break;
    }
  }
  return false;
}

bool fm_type_is_signed(fm_type_decl_cp td) {
  if (!td)
    return false;
  if (auto pval = std::get_if<fm::base_type_def>(&td->def)) {
    switch (pval->type) {
    case FM_TYPE_INT8:
    case FM_TYPE_INT16:
    case FM_TYPE_INT32:
    case FM_TYPE_INT64:
      return true;
      break;
    case FM_TYPE_UINT8:
    case FM_TYPE_UINT16:
    case FM_TYPE_UINT32:
    case FM_TYPE_UINT64:
      return false;
      break;
    case FM_TYPE_FLOAT32:
    case FM_TYPE_FLOAT64:
      return false;
      break;
    default:
      return false;
      break;
    }
  }
  return false;
}

bool fm_type_is_float(fm_type_decl_cp td) {
  if (!td)
    return false;
  if (auto pval = std::get_if<fm::base_type_def>(&td->def)) {
    switch (pval->type) {
    case FM_TYPE_INT8:
    case FM_TYPE_INT16:
    case FM_TYPE_INT32:
    case FM_TYPE_INT64:
      return false;
      break;
    case FM_TYPE_UINT8:
    case FM_TYPE_UINT16:
    case FM_TYPE_UINT32:
    case FM_TYPE_UINT64:
      return false;
      break;
    case FM_TYPE_FLOAT32:
    case FM_TYPE_FLOAT64:
      return true;
      break;
    default:
      return false;
      break;
    }
  }
  return false;
}

bool fm_type_is_bool(fm_type_decl_cp td) {
  if (!td)
    return false;
  if (auto pval = std::get_if<fm::base_type_def>(&td->def)) {
    switch (pval->type) {
    case FM_TYPE_BOOL:
      return true;
      break;
    default:
      return false;
      break;
    }
  }
  return false;
}

bool fm_type_is_decimal(fm_type_decl_cp td) {
  if (!td)
    return false;
  if (auto pval = std::get_if<fm::base_type_def>(&td->def)) {
    switch (pval->type) {
    case FM_TYPE_INT8:
    case FM_TYPE_INT16:
    case FM_TYPE_INT32:
    case FM_TYPE_INT64:
      return false;
      break;
    case FM_TYPE_UINT8:
    case FM_TYPE_UINT16:
    case FM_TYPE_UINT32:
    case FM_TYPE_UINT64:
      return false;
      break;
    case FM_TYPE_FLOAT32:
    case FM_TYPE_FLOAT64:
      return false;
      break;
    case FM_TYPE_DECIMAL64:
      return true;
      break;
    default:
      return false;
      break;
    }
  }
  return false;
}

bool fm_type_is_decimal128(fm_type_decl_cp td) {
  if (!td)
    return false;
  if (auto pval = std::get_if<fm::base_type_def>(&td->def)) {
    switch (pval->type) {
    case FM_TYPE_INT8:
    case FM_TYPE_INT16:
    case FM_TYPE_INT32:
    case FM_TYPE_INT64:
      return false;
      break;
    case FM_TYPE_UINT8:
    case FM_TYPE_UINT16:
    case FM_TYPE_UINT32:
    case FM_TYPE_UINT64:
      return false;
      break;
    case FM_TYPE_FLOAT32:
    case FM_TYPE_FLOAT64:
      return false;
      break;
    case FM_TYPE_DECIMAL64:
      return false;
      break;
    case FM_TYPE_DECIMAL128:
      return true;
      break;
    default:
      return false;
      break;
    }
  }
  return false;
}

bool fm_type_is_frame(fm_type_decl_cp td) {
  return std::holds_alternative<fm::frame_type_def>(td->def);
}

bool fm_type_is_subframe(fm_type_decl_cp td1, fm_type_decl_cp td2) {
  if (!std::holds_alternative<fm::frame_type_def>(td1->def))
    return false;

  auto fm1 = std::get_if<fm::frame_type_def>(&td1->def);
  auto fm2 = std::get_if<fm::frame_type_def>(&td2->def);
  if (!fm1 || !fm2)
    return false;
  for (auto &field : fm1->fields) {
    if (!fm2->has_field(field.first, field.second))
      return false;
  }
  return true;
}

bool fm_type_is_tuple(fm_type_decl_cp td) {
  return std::holds_alternative<fm::tuple_type_def>(td->def);
}

bool fm_type_is_cstring(fm_type_decl_cp td) {
  return std::holds_alternative<fm::cstring_type_def>(td->def);
}

bool fm_type_is_module(fm_type_decl_cp td) {
  return std::holds_alternative<fm::module_type_def>(td->def);
}

bool fm_type_is_type(fm_type_decl_cp td) {
  return std::holds_alternative<fm::type_type_def>(td->def);
}

FM_BASE_TYPE fm_type_base_enum(fm_type_decl_cp td) {
  if (!td)
    return FM_TYPE_LAST;
  if (auto pval = std::get_if<fm::base_type_def>(&td->def)) {
    return pval->type;
  } else {
    return FM_TYPE_LAST;
  }
}

size_t fm_type_record_size(fm_type_decl_cp td) {
  if (auto pval = std::get_if<fm::record_type_def>(&td->def)) {
    return pval->size;
  } else {
    return 0;
  }
}

size_t fm_type_array_size(fm_type_decl_cp td) {
  if (auto pval = std::get_if<fm::array_type_def>(&td->def)) {
    return pval->size;
  } else {
    return 0;
  }
}

fm_type_decl_cp fm_type_array_of(fm_type_decl_cp td) {
  if (!td)
    return nullptr;
  if (auto pval = std::get_if<fm::array_type_def>(&td->def)) {
    return pval->type;
  } else {
    return nullptr;
  }
}

unsigned fm_type_frame_ndims(fm_type_decl_cp td) {
  if (auto pval = std::get_if<fm::frame_type_def>(&td->def)) {
    return pval->dims.size();
  } else {
    return 0;
  }
}

int fm_type_frame_dim(fm_type_decl_cp td, int i) {
  if (auto pval = std::get_if<fm::frame_type_def>(&td->def)) {
    return pval->dims[i];
  } else {
    return -1;
  }
}

unsigned fm_type_frame_nfields(fm_type_decl_cp td) {
  if (auto pval = std::get_if<fm::frame_type_def>(&td->def)) {
    return pval->fields.size();
  } else {
    return 0;
  }
}

fm_type_decl_cp fm_type_frame_field_type(fm_type_decl_cp td, int i) {
  if (auto pval = std::get_if<fm::frame_type_def>(&td->def)) {
    return pval->fields[i].second;
  } else {
    return nullptr;
  }
}

const char *fm_type_frame_field_name(fm_type_decl_cp td, int i) {
  if (auto pval = std::get_if<fm::frame_type_def>(&td->def)) {
    return pval->fields[i].first.c_str();
  } else {
    return nullptr;
  }
}

int fm_type_frame_field_idx(fm_type_decl_cp td, const char *name) {
  if (auto pval = std::get_if<fm::frame_type_def>(&td->def)) {
    int i = 0;
    for (auto &field : pval->fields) {
      if (field.first == name)
        return i;
      ++i;
    }
  }

  return -1;
}

fm_type_decl_cp fm_frame_proj_type_get(fm_type_sys_t *ts, fm_type_decl_cp td,
                                       const char *name) {
  auto idx = fm_type_frame_field_idx(td, name);
  if (idx < 0)
    return nullptr;
  auto ftype = fm_type_frame_field_type(td, idx);
  const char *fnames[1] = {name};
  fm_type_decl_cp ftypes[1] = {ftype};
  auto nd = fm_type_frame_ndims(td);
  vector<int> dims(nd);
  for (unsigned i = 0; i < nd; ++i) {
    dims[i] = fm_type_frame_dim(td, i);
  }
  return fm_frame_type_get1(ts, 1, fnames, ftypes, nd, dims.data());
}

size_t fm_type_sizeof(fm_type_decl_cp td) {
  if (auto pval = std::get_if<fm::base_type_def>(&td->def)) {
    return fm_base_type_sizeof(pval->type);
  } else if (fm_type_is_record(td)) {
    return fm_type_record_size(td);
  } else if (fm_type_is_array(td)) {
    return fm_type_array_size(td) * fm_type_sizeof(fm_type_array_of(td));
  } else {
    return 0;
  }
}

unsigned fm_type_tuple_size(fm_type_decl_cp td) {
  if (!td)
    return 0;
  if (auto pval = std::get_if<fm::tuple_type_def>(&td->def)) {
    return pval->items.size();
  } else {
    return 0;
  }
}

bool fm_args_empty(fm_type_decl_cp td) {
  return td == nullptr || (fm_type_is_tuple(td) && fm_type_tuple_size(td) == 0);
}

fm_type_decl_cp fm_type_tuple_arg(fm_type_decl_cp td, unsigned i) {
  if (!td)
    return nullptr;
  if (auto pval = std::get_if<fm::tuple_type_def>(&td->def)) {
    if (i < 0 || i + 1 > pval->items.size()) {
      return nullptr;
    }
    return pval->items[i];
  } else {
    return nullptr;
  }
}

template <class T, class V>
int fm_stack_push_arg(fm_arg_stack_t *s, va_list *args) {
  T val = va_arg(*args, V);
  return STACK_SAFE_PUSH(*s, val) ? 0 : 1;
}

int fm_arg_stack_build(fm_type_decl_cp td, fm_arg_stack_t *s, va_list *args) {
  if (td == nullptr)
    return 0;
  return std::visit(
      fmc::overloaded{
          [=](auto arg) { return -1; },
          [=](const fm::base_type_def &arg) {
            bool res = false;
            switch (arg.type) {
            case FM_TYPE_INT8:
              return fm_stack_push_arg<INT8, int>(s, args);
              break;
            case FM_TYPE_INT16:
              return fm_stack_push_arg<INT16, int>(s, args);
              break;
            case FM_TYPE_INT32:
              return fm_stack_push_arg<INT32, INT32>(s, args);
              break;
            case FM_TYPE_INT64:
              return fm_stack_push_arg<INT64, INT64>(s, args);
              break;
            case FM_TYPE_UINT8:
              return fm_stack_push_arg<UINT8, int>(s, args);
              break;
            case FM_TYPE_UINT16:
              return fm_stack_push_arg<UINT16, int>(s, args);
              break;
            case FM_TYPE_UINT32:
              return fm_stack_push_arg<UINT32, UINT32>(s, args);
              break;
            case FM_TYPE_UINT64:
              return fm_stack_push_arg<UINT64, UINT64>(s, args);
              break;
            case FM_TYPE_FLOAT32:
              return fm_stack_push_arg<FLOAT32, double>(s, args);
              break;
            case FM_TYPE_FLOAT64:
              return fm_stack_push_arg<FLOAT64, FLOAT64>(s, args);
              break;
            case FM_TYPE_RATIONAL64:
              return fm_stack_push_arg<RATIONAL64, RATIONAL64>(s, args);
              break;
            case FM_TYPE_DECIMAL64:
              return fm_stack_push_arg<DECIMAL64, DECIMAL64>(s, args);
              break;
            case FM_TYPE_DECIMAL128:
              return fm_stack_push_arg<DECIMAL128, DECIMAL128>(s, args);
              break;
            case FM_TYPE_TIME64:
              return fm_stack_push_arg<TIME64, TIME64>(s, args);
              break;
            case FM_TYPE_CHAR:
              return fm_stack_push_arg<CHAR, int>(s, args);
              break;
            case FM_TYPE_WCHAR:
              return fm_stack_push_arg<WCHAR, int>(s, args);
              break;
            case FM_TYPE_BOOL:
              return fm_stack_push_arg<bool, int>(s, args);
              break;
            case FM_TYPE_LAST:
              return -1;
              break;
            }
            return res ? 0 : 1;
          },
          [=](const fm::tuple_type_def &arg) {
            for (auto *type : arg.items) {
              auto res = fm_arg_stack_build(type, s, args);
              if (res != 0)
                return res;
            }
            return 0;
          },
          [=](const fm::cstring_type_def &arg) {
            return fm_stack_push_arg<const char *, const char *>(s, args);
          },
          [=](const fm::module_type_def &arg) {
            return fm_stack_push_arg<fm_module_t *, fm_module_t *>(s, args);
          },
          [=](const fm::type_type_def &arg) {
            return fm_stack_push_arg<fm_type_decl_cp, fm_type_decl_cp>(s, args);
          },
      },
      td->def);
}

bool fm_arg_get_uint64(fm_type_decl_cp td, fm_arg_stack_t *arg, uint64_t *i) {
  auto en = fm_type_base_enum(td);
  switch (en) {
  case FM_TYPE_UINT8:
    *i = STACK_POP(*arg, UINT8);
    return true;
    break;
  case FM_TYPE_UINT16:
    *i = STACK_POP(*arg, UINT16);
    return true;
    break;
  case FM_TYPE_UINT32:
    *i = STACK_POP(*arg, UINT32);
    return true;
    break;
  case FM_TYPE_UINT64:
    *i = STACK_POP(*arg, UINT64);
    return true;
    break;
  default:
    return false;
  }
}

bool fm_arg_get_int64(fm_type_decl_cp td, fm_arg_stack_t *arg, int64_t *i) {
  auto en = fm_type_base_enum(td);
  switch (en) {
  case FM_TYPE_INT8:
    *i = STACK_POP(*arg, INT8);
    return true;
    break;
  case FM_TYPE_INT16:
    *i = STACK_POP(*arg, INT16);
    return true;
    break;
  case FM_TYPE_INT32:
    *i = STACK_POP(*arg, INT32);
    return true;
    break;
  case FM_TYPE_INT64:
    *i = STACK_POP(*arg, INT64);
    return true;
    break;
  default:
    return false;
  }
}

bool fm_arg_get_float64(fm_type_decl_cp td, fm_arg_stack_t *arg, double *i) {
  auto en = fm_type_base_enum(td);
  switch (en) {
  case FM_TYPE_FLOAT32:
    *i = STACK_POP(*arg, FLOAT32);
    return true;
    break;
  case FM_TYPE_FLOAT64:
    *i = STACK_POP(*arg, FLOAT64);
    return true;
    break;
  default:
    return false;
  }
}

bool fm_arg_try_uinteger(fm_type_decl_cp td, fm_arg_stack_t *arg, uint64_t *i) {
  if (fm_arg_get_uint64(td, arg, i)) {
    return true;
  }
  int64_t tmp_i;
  fm_arg_stack_t tmp_arg = *arg;
  if (fm_arg_get_int64(td, &tmp_arg, &tmp_i)) {
    if (tmp_i >= 0) {
      *i = tmp_i;
      *arg = tmp_arg;
      return true;
    }
  }
  return false;
}

bool fm_arg_try_integer(fm_type_decl_cp td, fm_arg_stack_t *arg, int64_t *i) {
  return fm_arg_get_int64(td, arg, i);
}

bool fm_arg_try_float64(fm_type_decl_cp td, fm_arg_stack_t *arg, double *i) {
  auto en = fm_type_base_enum(td);
  switch (en) {
  case FM_TYPE_FLOAT32:
    *i = STACK_POP(*arg, FLOAT32);
    return true;
    break;
  case FM_TYPE_FLOAT64:
    *i = STACK_POP(*arg, FLOAT64);
    return true;
    break;
  default:
    return false;
  }
}

bool fm_arg_try_time64(fm_type_decl_cp td, fm_arg_stack_t *arg,
                       fmc_time64_t *i) {
  auto en = fm_type_base_enum(td);
  if (en != FM_TYPE_TIME64) {
    return false;
  }
  *i = STACK_POP(*arg, TIME64);
  return true;
}

fm_type_decl_cp fm_arg_try_type_decl(fm_type_decl_cp ptype,
                                     fm_arg_stack_t *plist) {
  if (!ptype)
    return nullptr;
  if (!fm_type_is_type(ptype)) {
    return nullptr;
  }
  return STACK_POP(*plist, fm_type_decl_cp);
}

const char *fm_arg_try_cstring(fm_type_decl_cp ptype, fm_arg_stack_t *plist) {
  if (!ptype)
    return nullptr;
  if (!fm_type_is_cstring(ptype)) {
    return nullptr;
  }
  return STACK_POP(*plist, const char *);
}

char *fm_type_to_str(fm_type_decl_cp decl) {
  std::string res = decl->str();
  auto size = res.size() + 1;
  char *str = (char *)malloc(size);
  strncpy(str, res.c_str(), size);
  return str;
}

fm_type_decl_cp fm_type_from_str(fm_type_sys_t *ts, const char *c, size_t len) {
  string_view view(c, len);
  auto *td = ts->type_space.get_type_from_str(view);
  if (!view.empty())
    return nullptr;
  return td;
}

fm_type_io fm_type_io_gen(fm_type_decl_cp decl) {
  if (fm_type_is_base(decl)) {
    auto em = fm_type_base_enum(decl);
    auto base_parse = fm_base_type_parser_get(em);
    auto base_fwrite = fm_base_type_fwriter_get(em);
    return {
        fm_type_io::parser([base_parse](const char *b, const char *e, void *d) {
          auto *r = base_parse(b, e, d, "");
          return r == b ? nullptr : r;
        }),
        fm_type_io::fwriter([base_fwrite](FILE *f, const void *d) {
          return base_fwrite(f, d, "");
        })};
  } else if (fm_type_is_array(decl)) {
    auto type = fm_type_array_of(decl);
    auto size = fm_type_array_size(decl);
    if (fm_type_is_base(type)) {
      switch (fm_type_base_enum(type)) {
      case FM_TYPE_CHAR:
        return {
            fm_type_io::parser([size](const char *b, const char *e, void *d) {
              memset((void *)d, 0, size);
              if (e < b || size_t(e - b) > size)
                return b;
              memcpy(d, b, e - b);
              return e;
            }),
            fm_type_io::fwriter([size](FILE *f, const void *d) {
              auto *ptr = (const char *)d;
              for (size_t i = size; i > 0; --i, ++ptr) {
                if (*ptr == 0)
                  return true;
                if (fputc(*ptr, f) == EOF)
                  return false;
              }
              return true;
            })};
        break;
      default:
        break;
      }
    }
  }
  return fm_type_io();
}

fm_type_io_t *fm_type_io_get(fm_type_sys_t *ts, fm_type_decl_cp decl) {
  if (auto where = ts->parsers_.find(decl->index);
      where == ts->parsers_.end()) {
    auto *io = new fm_type_io(fm_type_io_gen(decl));
    return ts->parsers_.emplace(decl->index, std::unique_ptr<fm_type_io>(io))
        .first->second.get();
  } else {
    return where->second.get();
  }
}

const char *fm_type_io_parse(fm_type_io_t *io, const char *b, const char *e,
                             void *d) {
  return io->parse(b, e, d);
}

bool fm_type_io_fwrite(fm_type_io_t *io, FILE *f, const void *d) {
  return io->fwrite(f, d);
}
