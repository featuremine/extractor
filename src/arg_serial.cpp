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
 * @file arg_serial.cpp
 * @author Maxim Trokhimtchouk
 * @date 12 Jun 2018
 * @brief File contains C declaration of the argument serialization
 *
 * This file contains declarations of the argument serialization
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

extern "C" {
#include "arg_serial.h"
#include "extractor/arg_stack.h"
#include "extractor/type_decl.h"
#include "extractor/type_sys.h"
}

#include "extractor/comp_def.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/rational64.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/time.hpp"
#include "serial_util.hpp"
#include "type_space.hpp"
#include "upcast_util.hpp"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace fm;
using namespace fmc;

struct fm_arg_buffer {
  string buf;
  vector<string *> strings;
};

bool fm_arg_buffer_build(ostringstream &os, fm_type_decl_cp td,
                         fm_arg_stack_t &args) {
  if (td == nullptr)
    return false;
  return std::visit(overloaded{
                        [&](auto arg) { return false; },
                        [&](const fm::base_type_def &arg) {
                          switch (arg.type) {
                          case FM_TYPE_INT8:
                            os << STACK_POP(args, INT8) << endl;
                            break;
                          case FM_TYPE_INT16:
                            os << STACK_POP(args, INT16) << endl;
                            break;
                          case FM_TYPE_INT32:
                            os << STACK_POP(args, INT32) << endl;
                            break;
                          case FM_TYPE_INT64:
                            os << STACK_POP(args, INT64) << endl;
                            break;
                          case FM_TYPE_UINT8:
                            os << STACK_POP(args, UINT8) << endl;
                            break;
                          case FM_TYPE_UINT16:
                            os << STACK_POP(args, UINT16) << endl;
                            break;
                          case FM_TYPE_UINT32:
                            os << STACK_POP(args, UINT32) << endl;
                            break;
                          case FM_TYPE_UINT64:
                            os << STACK_POP(args, UINT64) << endl;
                            break;
                          case FM_TYPE_FLOAT32:
                            os << STACK_POP(args, FLOAT32) << endl;
                            break;
                          case FM_TYPE_FLOAT64:
                            os << STACK_POP(args, FLOAT64) << endl;
                            break;
                          case FM_TYPE_RATIONAL64:
                            os << STACK_POP(args, RATIONAL64) << endl;
                            break;
                          case FM_TYPE_RPRICE:
                            os << fmc::rprice::upcast(STACK_POP(args, RPRICE)) << endl;
                            break;
                          case FM_TYPE_DECIMAL128:
                            os << STACK_POP(args, DECIMAL128) << endl;
                            break;
                          case FM_TYPE_TIME64:
                            os << STACK_POP(args, TIME64) << endl;
                            break;
                          case FM_TYPE_CHAR:
                            os << STACK_POP(args, CHAR) << endl;
                            break;
                          case FM_TYPE_WCHAR:
                            os << STACK_POP(args, WCHAR) << endl;
                            break;
                          case FM_TYPE_BOOL:
                            os << STACK_POP(args, bool) << endl;
                            break;
                          case FM_TYPE_LAST:
                            return false;
                            break;
                          }
                          return true;
                        },
                        [&](const fm::tuple_type_def &arg) {
                          for (auto *type : arg.items) {
                            if (!fm_arg_buffer_build(os, type, args)) {
                              return false;
                            }
                          }
                          return true;
                        },
                        [&](const fm::cstring_type_def &arg) {
                          const char *str = STACK_POP(args, const char *);
                          os << strlen(str) << '\0' << str << endl;
                          return true;
                        },
                        [&](const fm::type_type_def &arg) {
                          os << STACK_POP(args, fm_type_decl_cp)->str() << endl;
                          return true;
                        },
                    },
                    td->def);
}

fm_arg_buffer_t *fm_arg_buffer_new(fm_type_decl_cp type, fm_arg_stack_t args) {
  ostringstream os;
  if (fm_arg_buffer_build(os, type, args)) {
    auto *x = new fm_arg_buffer();
    x->buf = type->str();
    x->buf.append("\n");
    x->buf.append(os.str());
    return x;
  }
  return nullptr;
}

bool fm_arg_write(const fm_arg_buffer_t *buf, fm_writer writer, void *closure) {
  auto size = buf->buf.size();
  return writer(buf->buf.data(), size, closure) == size;
}

void fm_arg_buffer_del(fm_arg_buffer_t *buf) {
  for (auto *str : buf->strings) {
    delete str;
  }
  delete buf;
}

size_t fm_arg_buffer_dump(fm_arg_buffer_t *buf, const char **where) {
  *where = buf->buf.c_str();
  return buf->buf.size();
}

template <class T>
bool fm_arg_item_read(string &buf, fm_arg_stack_t **s, fm_reader reader,
                      void *closure) {
  using S = typename upcast<T>::type;
  S x;
  if (!fm_item_read(buf, x, reader, closure)) {
    return false;
  }
  return HEAP_STACK_PUSH(*s, x);
}

bool fm_arg_stack_read(fm_arg_buffer_t *arg_buf, fm_type_sys_t *ts,
                       fm_type_decl_cp td, fm_arg_stack_t **s, fm_reader reader,
                       void *closure) {
  string &buf = arg_buf->buf;
  return std::visit(
      overloaded{
          [&](auto arg) { return false; },
          [&](const fm::base_type_def &arg) {
            switch (arg.type) {
            case FM_TYPE_INT8:
              return fm_arg_item_read<INT8>(buf, s, reader, closure);
              break;
            case FM_TYPE_INT16:
              return fm_arg_item_read<INT16>(buf, s, reader, closure);
              break;
            case FM_TYPE_INT32:
              return fm_arg_item_read<INT32>(buf, s, reader, closure);
              break;
            case FM_TYPE_INT64:
              return fm_arg_item_read<INT64>(buf, s, reader, closure);
              break;
            case FM_TYPE_UINT8:
              return fm_arg_item_read<UINT8>(buf, s, reader, closure);
              break;
            case FM_TYPE_UINT16:
              return fm_arg_item_read<UINT16>(buf, s, reader, closure);
              break;
            case FM_TYPE_UINT32:
              return fm_arg_item_read<UINT32>(buf, s, reader, closure);
              break;
            case FM_TYPE_UINT64:
              return fm_arg_item_read<UINT64>(buf, s, reader, closure);
              break;
            case FM_TYPE_FLOAT32:
              return fm_arg_item_read<FLOAT32>(buf, s, reader, closure);
              break;
            case FM_TYPE_FLOAT64:
              return fm_arg_item_read<FLOAT64>(buf, s, reader, closure);
              break;
            case FM_TYPE_RATIONAL64:
              return fm_arg_item_read<RATIONAL64>(buf, s, reader, closure);
              break;
            case FM_TYPE_RPRICE:
              return fm_arg_item_read<RPRICE>(buf, s, reader, closure);
              break;
            case FM_TYPE_DECIMAL128:
              return fm_arg_item_read<DECIMAL128>(buf, s, reader, closure);
              break;
            case FM_TYPE_TIME64:
              return fm_arg_item_read<TIME64>(buf, s, reader, closure);
              break;
            case FM_TYPE_CHAR:
              return fm_arg_item_read<CHAR>(buf, s, reader, closure);
              break;
            case FM_TYPE_WCHAR:
              return fm_arg_item_read<WCHAR>(buf, s, reader, closure);
              break;
            case FM_TYPE_BOOL:
              return fm_arg_item_read<bool>(buf, s, reader, closure);
              break;
            case FM_TYPE_LAST:
              return false;
              break;
            }
            return true;
          },
          [&](const fm::tuple_type_def &arg) {
            for (auto *type : arg.items) {
              if (!fm_arg_stack_read(arg_buf, ts, type, s, reader, closure)) {
                return false;
              }
            }
            return true;
          },
          [&](const fm::cstring_type_def &arg) {
            size_t len;
            if (!fm_item_read(buf, len, reader, closure, '\0'))
              return false;
            auto view = fm_read_buf(buf, len + 1, reader, closure);
            if (view.empty())
              return false;
            if (view.back() != '\n')
              return false;
            arg_buf->strings.push_back(
                new string(view.substr(0, view.size() - 1)));
            auto *chr = arg_buf->strings.back()->c_str();
            return HEAP_STACK_PUSH(*s, chr);
          },
          [&](const fm::type_type_def &arg) {
            auto view = fm_read_line(buf, reader, closure);
            auto td = fm_type_from_str(ts, view.data(), view.size());
            if (!td)
              return false;
            return HEAP_STACK_PUSH(*s, td);
            ;
          },
      },
      td->def);
}

bool string_view_reader(void *data, size_t limit, void *closure) {
  auto *view = (string_view *)closure;
  if (view->size() < limit)
    return false;
  memcpy(data, view->data(), limit);
  *view = view->substr(limit);
  return true;
}

fm_arg_buffer_t *fm_arg_stack_from_buffer(fm_type_sys_t *ts,
                                          fm_arg_buffer_t *args,
                                          fm_type_decl_cp *type,
                                          fm_arg_stack_t **s) {
  string_view cl(args->buf.data(), args->buf.length());
  return fm_arg_read(ts, type, s, &string_view_reader, &cl);
}

fm_arg_buffer_t *fm_arg_read(fm_type_sys_t *ts, fm_type_decl_cp *td,
                             fm_arg_stack_t **s, fm_reader reader,
                             void *closure) {
  auto *buf = new fm_arg_buffer();
  auto view = fm_read_line(buf->buf, reader, closure);
  *s = fm_arg_stack_alloc(1024);
  if (view.empty()) {
    fm_arg_buffer_del(buf);
    return nullptr;
  }
  *td = fm_type_from_str(ts, view.data(), view.size());
  if (*td && fm_arg_stack_read(buf, ts, *td, s, reader, closure)) {
    return buf;
  }
  fm_arg_stack_free(*s);
  fm_arg_buffer_del(buf);
  return nullptr;
}
