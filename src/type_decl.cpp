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
 * @file type_decl.cpp
 * @author Maxim Trokhimtchouk
 * @date 20 Sept 2017
 * @brief File contains C++ definitions of the type declaration system
 *
 * This file contains definitions of the type declaration system
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/type_decl.h"
#include "fmc/time.h"
}

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <limits>
#include <type_traits>

using namespace std;

template <class T> struct typify { using type = T; };

#define STR_TO_TYPE3(type, func)                                               \
  type str_to_type(const char *str, char **str_end, typify<type>) {            \
    return func(str, str_end, 10);                                             \
  }
#define STR_TO_TYPE2(type, func)                                               \
  type str_to_type(const char *str, char **str_end, typify<type>) {            \
    return func(str, str_end);                                                 \
  }

template <class T> auto str_to_int(const char *str, char **str_end) {
  long val = strtol(str, str_end, 10);
  using limits = std::numeric_limits<T>;
  if (val < limits::lowest()) {
    val = limits::lowest();
    errno = ERANGE;
  }
  if (val > limits::max()) {
    val = limits::max();
    errno = ERANGE;
  }
  return val;
}

STR_TO_TYPE2(int8_t, str_to_int<int8_t>);
STR_TO_TYPE2(int16_t, str_to_int<int16_t>);
STR_TO_TYPE2(int32_t, str_to_int<int32_t>);
STR_TO_TYPE2(uint8_t, str_to_int<uint8_t>);
STR_TO_TYPE2(uint16_t, str_to_int<uint16_t>);
STR_TO_TYPE2(uint32_t, str_to_int<uint32_t>);
STR_TO_TYPE3(long, std::strtol);
STR_TO_TYPE3(long long, std::strtoll);
STR_TO_TYPE3(unsigned long, std::strtoul);
STR_TO_TYPE3(unsigned long long, std::strtoull);
STR_TO_TYPE2(float, std::strtof);
STR_TO_TYPE2(double, std::strtod);
STR_TO_TYPE2(long double, std::strtold);
STR_TO_TYPE2(bool, str_to_int<bool>);

template <class T>
const char *parser_def(const char *begin, const char *end, T *data,
                       const char *fmt) {
  char *e;
  auto val = str_to_type(begin, &e, typify<T>());
  if (e != end || e == begin)
    return begin;
  *data = val;
  return end;
}

const char *nano_parser(const char *begin, const char *end, void *data,
                        const char *fmt) {
  auto &time = *(fmc_time64_t *)data;
  return parser_def(begin, end, &time.value, "");
}

const char *skip(const char *begin, const char *end, void *data,
                 const char *fmt) {
  return end;
}

template <class T>
const char *type_parser(const char *begin, const char *end, void *data,
                        const char *fmt) {
  return parser_def(begin, end, (T *)data, fmt);
}

const char *rational64_parser(const char *begin, const char *end, void *data,
                              const char *fmt) {
  char *e = (char *)end;
  int64_t num = strtol(begin, &e, 10);

  if (e == begin) {
    if (e[0] != '.')
      return e;
    num = 0;
  }

  if (e == end) {
    *(fm_rational64_t *)data = fm_rational64_new2(num, 1);
    return end;
  }

  char *d = (char *)end;
  int64_t den = strtol(e + 1, &d, 10);

  if (d == e + 1 || d != end) {
    return d;
  }

  if (e[0] == '.') {
    int32_t buff = 1;
    for (e = e + 1; e < end; ++e)
      buff *= 10;
    num = num * buff + den;
    den = buff;
  } else if (e[0] != '/') {
    return e;
  }

  *(fm_rational64_t *)data = fm_rational64_new2(num, den);

  return d;
}

const char *decimal64_parser(const char *begin, const char *end, void *data,
                             const char *fmt) {
  double val = 0.0;
  auto ret = type_parser<FLOAT64>(begin, end, &val, "");
  *(fm_decimal64_t *)data = fm_decimal64_from_double(val);
  return ret;
}

const char *decimal128_parser(const char *begin, const char *end, void *data,
                             const char *fmt) {
  fmc_decimal128_from_str((fmc_decimal128_t *)data, begin);
  return end;
}

template <class T>
const char *char_parser(const char *begin, const char *end, void *data,
                        const char *fmt) {
  if (begin + sizeof(T) != end)
    return begin;
  *(T *)data = *(T *)begin;
  return begin + sizeof(T);
}

const char *bool_parser(const char *begin, const char *end, void *data,
                        const char *fmt) {
  int val = 0.0;
  auto ret = type_parser<int32_t>(begin, end, &val, "");
  *(bool *)data = val == 1 ? true : false;
  return ret;
}

fm_base_type_parser fm_base_type_parser_get(FM_BASE_TYPE t) {
  switch (t) {
  case FM_TYPE_INT8:
    return &type_parser<INT8>;
    break;
  case FM_TYPE_INT16:
    return &type_parser<INT16>;
    break;
  case FM_TYPE_INT32:
    return &type_parser<INT32>;
    break;
  case FM_TYPE_INT64:
    return &type_parser<INT64>;
    break;
  case FM_TYPE_UINT8:
    return &type_parser<UINT8>;
    break;
  case FM_TYPE_UINT16:
    return &type_parser<UINT16>;
    break;
  case FM_TYPE_UINT32:
    return &type_parser<UINT32>;
    break;
  case FM_TYPE_UINT64:
    return &type_parser<UINT64>;
    break;
  case FM_TYPE_FLOAT32:
    return &type_parser<FLOAT32>;
    break;
  case FM_TYPE_FLOAT64:
    return &type_parser<FLOAT64>;
    break;
  case FM_TYPE_RATIONAL64:
    return &rational64_parser;
    break;
  case FM_TYPE_DECIMAL64:
    return &decimal64_parser;
    break;
  case FM_TYPE_DECIMAL128:
    return &decimal128_parser;
    break;
  case FM_TYPE_TIME64:
    return &nano_parser;
    break;
  case FM_TYPE_CHAR:
    return &char_parser<CHAR>;
    break;
  case FM_TYPE_WCHAR:
    return &char_parser<WCHAR>;
    break;
  case FM_TYPE_BOOL:
    return &bool_parser;
    break;
  case FM_TYPE_LAST:
    return &skip;
    break;
  }
  return &skip;
}

size_t fm_base_type_sizeof(FM_BASE_TYPE t) {
  switch (t) {
  case FM_TYPE_INT8:
    return sizeof(INT8);
    break;
  case FM_TYPE_INT16:
    return sizeof(INT16);
    break;
  case FM_TYPE_INT32:
    return sizeof(INT32);
    break;
  case FM_TYPE_INT64:
    return sizeof(INT64);
    break;
  case FM_TYPE_UINT8:
    return sizeof(UINT8);
    break;
  case FM_TYPE_UINT16:
    return sizeof(UINT16);
    break;
  case FM_TYPE_UINT32:
    return sizeof(UINT32);
    break;
  case FM_TYPE_UINT64:
    return sizeof(UINT64);
    break;
  case FM_TYPE_FLOAT32:
    return sizeof(FLOAT32);
    break;
  case FM_TYPE_FLOAT64:
    return sizeof(FLOAT64);
    break;
  case FM_TYPE_RATIONAL64:
    return sizeof(RATIONAL64);
    break;
  case FM_TYPE_DECIMAL64:
    return sizeof(DECIMAL64);
    break;
  case FM_TYPE_DECIMAL128:
    return sizeof(DECIMAL128);
    break;
  case FM_TYPE_TIME64:
    return sizeof(TIME64);
    break;
  case FM_TYPE_CHAR:
    return sizeof(CHAR);
    break;
  case FM_TYPE_WCHAR:
    return sizeof(WCHAR);
    break;
  case FM_TYPE_BOOL:
    return sizeof(int32_t);
    break;
  case FM_TYPE_LAST:
    return 0;
    break;
  }
  return 0;
}

constexpr const char *format_str(FM_BASE_TYPE type) {
  switch (type) {
  case FM_TYPE_INT8:
    return "%hhi";
    break;
  case FM_TYPE_INT16:
    return "%hi";
    break;
  case FM_TYPE_INT32:
    return "%i";
    break;
  case FM_TYPE_INT64:
    return "%li";
    break;
  case FM_TYPE_UINT8:
    return "%hhu";
    break;
  case FM_TYPE_UINT16:
    return "%hu";
    break;
  case FM_TYPE_UINT32:
    return "%u";
    break;
  case FM_TYPE_UINT64:
    return "%lu";
    break;
  case FM_TYPE_FLOAT32:
    return "%.15f";
    break;
  case FM_TYPE_FLOAT64:
    return "%.15lf";
    break;
  case FM_TYPE_RATIONAL64:
    return "";
    break;
  case FM_TYPE_DECIMAL64:
    return "";
    break;
  case FM_TYPE_DECIMAL128:
    return "";
    break;
  case FM_TYPE_TIME64:
    return "";
    break;
  case FM_TYPE_CHAR:
    return "%c";
    break;
  case FM_TYPE_WCHAR:
    return "%lc";
    break;
  case FM_TYPE_BOOL:
    return "%d";
  case FM_TYPE_LAST:
    return "";
    break;
  }
  return "";
}

template <class T, FM_BASE_TYPE type>
bool type_fwrite(FILE *file, const void *val, const char *fmt) {
  return fprintf(file, format_str(type), *(T *)val) > 0;
}

bool nano_fwriter(FILE *file, const void *val, const char *fmt) {
  auto &time = *(fmc_time64_t *)val;
  return fprintf(file, "%li", time.value) > 0;
}

bool rational64_fwriter(FILE *file, const void *val, const char *fmt) {
  auto value = (*(fm_rational64_t *)val);
  return fprintf(file, "%i/%i", value.num, value.den) > 0;
}

bool decimal64_fwriter(FILE *file, const void *val, const char *fmt) {
  auto value = fm_decimal64_to_double(*(fm_decimal64_t *)val);
  return fprintf(file, "%.15lg", value) > 0;
}

bool decimal128_fwriter(FILE *file, const void *val, const char *fmt) {
  return false;
}

bool bool_fwriter(FILE *file, const void *val, const char *fmt) {
  int32_t value = *(bool *)val ? 1 : 0;
  return fprintf(file, "%d", value) > 0;
}

fm_base_type_fwriter fm_base_type_fwriter_get(FM_BASE_TYPE t) {
  switch (t) {
  case FM_TYPE_INT8:
    return &type_fwrite<INT8, FM_TYPE_INT8>;
    break;
  case FM_TYPE_INT16:
    return &type_fwrite<INT16, FM_TYPE_INT16>;
    break;
  case FM_TYPE_INT32:
    return &type_fwrite<INT32, FM_TYPE_INT32>;
    break;
  case FM_TYPE_INT64:
    return &type_fwrite<INT64, FM_TYPE_INT64>;
    break;
  case FM_TYPE_UINT8:
    return &type_fwrite<UINT8, FM_TYPE_UINT8>;
    break;
  case FM_TYPE_UINT16:
    return &type_fwrite<UINT16, FM_TYPE_UINT16>;
    break;
  case FM_TYPE_UINT32:
    return &type_fwrite<UINT32, FM_TYPE_UINT32>;
    break;
  case FM_TYPE_UINT64:
    return &type_fwrite<UINT64, FM_TYPE_UINT64>;
    break;
  case FM_TYPE_FLOAT32:
    return &type_fwrite<FLOAT32, FM_TYPE_FLOAT32>;
    break;
  case FM_TYPE_FLOAT64:
    return &type_fwrite<FLOAT64, FM_TYPE_FLOAT64>;
    break;
  case FM_TYPE_RATIONAL64:
    return &rational64_fwriter;
    break;
  case FM_TYPE_DECIMAL64:
    return &decimal64_fwriter;
    break;
  case FM_TYPE_DECIMAL128:
    return &decimal128_fwriter;
    break;
  case FM_TYPE_TIME64:
    return &nano_fwriter;
    break;
  case FM_TYPE_CHAR:
    return &type_fwrite<CHAR, FM_TYPE_CHAR>;
    break;
  case FM_TYPE_WCHAR:
    return &type_fwrite<WCHAR, FM_TYPE_WCHAR>;
    break;
  case FM_TYPE_BOOL:
    return &bool_fwriter;
    break;
  case FM_TYPE_LAST:
    return nullptr;
    break;
  }
  return nullptr;
}

const char *fm_base_type_name(FM_BASE_TYPE t) {
  switch (t) {
  case FM_TYPE_INT8:
    return "INT8";
  case FM_TYPE_INT16:
    return "INT16";
  case FM_TYPE_INT32:
    return "INT32";
  case FM_TYPE_INT64:
    return "INT64";
  case FM_TYPE_UINT8:
    return "UINT8";
  case FM_TYPE_UINT16:
    return "UINT16";
  case FM_TYPE_UINT32:
    return "UINT32";
  case FM_TYPE_UINT64:
    return "UINT64";
  case FM_TYPE_FLOAT32:
    return "FLOAT32";
  case FM_TYPE_FLOAT64:
    return "FLOAT64";
  case FM_TYPE_RATIONAL64:
    return "RATIONAL64";
  case FM_TYPE_DECIMAL64:
    return "DECIMAL64";
  case FM_TYPE_DECIMAL128:
    return "DECIMAL128";
  case FM_TYPE_TIME64:
    return "TIME64";
  case FM_TYPE_CHAR:
    return "CHAR";
  case FM_TYPE_WCHAR:
    return "WCHAR";
  case FM_TYPE_BOOL:
    return "BOOL";
  case FM_TYPE_LAST:
    return "LAST";
  }
  return nullptr;
}
