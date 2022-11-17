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
 * @file serial_util.hpp
 * @author Maxim Trokhimtchouk
 * @date 12 Jun 2018
 * @brief File contains C declaration of the argument serialization
 *
 * This file contains declarations of the argument serialization
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/serial.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>

using namespace std;

inline string_view fm_read_line(string &buf, fm_reader reader, void *closure,
                                char term = '\n') {
  auto b = buf.size();
  char c = -1;
  while (c != term && reader((void *)&c, 1, closure)) {
    buf.append(&c, 1);
  }
  if (c == term) {
    return string_view(buf.data() + b, buf.size() - b - 1);
  }
  buf.resize(b);
  return string_view();
}

inline string read_str(fm_reader reader, void *closure) {
  string str;
  return string(fm_read_line(str, reader, closure));
}

inline string_view fm_read_buf(string &buf, size_t limit, fm_reader reader,
                               void *closure) {
  auto size = buf.size();
  buf.resize(size + limit);
  if (reader(buf.data() + size, limit, closure)) {
    return string_view(buf.data() + size, limit);
  }
  buf.resize(size);
  return string_view();
}

template <class T>
bool fm_item_read(string &buf, T &x, fm_reader reader, void *closure,
                  char term = '\n') {
  auto view = fm_read_line(buf, reader, closure, term);
  if (view.empty())
    return false;
  std::istringstream stream{string(view)};
  stream >> x;
  return bool(stream);
}

inline bool fm_item_read(string &buf, WCHAR &x, fm_reader reader, void *closure,
                         char term = '\n') {
  auto view = fm_read_line(buf, reader, closure, term);
  if (view.empty())
    return false;
  if (view.size() == sizeof(WCHAR)) {
    x = *(WCHAR *)view.data();
    return true;
  }
  return false;
}

inline bool read_unsigned(unsigned &num, fm_reader reader, void *closure) {
  string buf;
  return fm_item_read(buf, num, reader, closure);
}

inline bool write_char(char c, fm_writer writer, void *closure) {
  return 1 == writer(&c, 1, closure);
}

inline bool write_string(string &buf, fm_writer writer, void *closure) {
  return buf.size() == writer(buf.data(), buf.size(), closure) &&
         write_char('\n', writer, closure);
}

inline bool write_string(const char *str, fm_writer writer, void *closure) {
  auto len = strlen(str);
  return len == writer(str, len, closure) && write_char('\n', writer, closure);
}

template <class T> bool write_number(T x, fm_writer writer, void *closure) {
  auto buf = to_string(x);
  return write_string(buf, writer, closure);
}
