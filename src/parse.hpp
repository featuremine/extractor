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
 * @file parse.hpp
 * @author Maxim Trokhimtchouk
 * @date 18 Sept 2017
 * @brief File contains C declaration of the execution context
 *
 * This file contains declarations of the execution context
 * @see http://www.featuremine.com
 */

#pragma once

#include <functional>
#include <stdlib.h>
#include <string>
#include <vector>

namespace fm {
class parse_result {
public:
  enum STATUS { OK = 0, FAIL = 1, ERROR = 2 };
  parse_result() = default;
  parse_result(STATUS s) : val(s) {}
  operator bool() { return val != OK; }
  bool operator==(const parse_result &a) { return a.val == val; }
  void set_ok() { val = OK; }
  void set_fail() { val = FAIL; }
  void set_error() { val = ERROR; }
  static parse_result ok() { return parse_result(OK); }
  static parse_result fail() { return parse_result(FAIL); }
  static parse_result error() { return parse_result(ERROR); }

private:
  STATUS val = OK;
};

struct parse_pos {
  int line = 0;
  int column = 0;
};

class parse_buf {
public:
  parse_buf(FILE *f) : file_(f) {}
  ~parse_buf() {
    if (file_)
      fclose(file_);
  }
  parse_result parse_char() {
    if (auto c = fgetc(file_); c == EOF) {
      return set_file_state();
    } else {
      buf_.push_back(c);
      state_ = parse_result::ok();
      return state_;
    }
  }
  const parse_pos &pos() const { return pos_; }
  void drop_last() {
    if (!buf_.empty())
      buf_.pop_back();
  }
  char last() const { return buf_.empty() ? 0 : buf_.back(); }
  void push(char c) { buf_.push_back(c); }
  void unwind() {
    ungetc(last(), file_);
    drop_last();
  }
  void clear() { buf_.clear(); }
  const std::string &string() const { return buf_; }
  void set_error(const std::string e) { error_ = e; }
  void clear_error() { error_.clear(); }
  const std::string &error() { return error_; }
  parse_result status() const { return state_; }

private:
  parse_result set_file_state() {
    if (ferror(file_)) {
      state_ = parse_result::error();
    } else if (feof(file_)) {
      state_ = parse_result::fail();
    } else {
      state_ = parse_result::ok();
    }
    return state_;
  }
  FILE *file_;
  parse_pos pos_;
  parse_result state_;
  std::string buf_;
  std::string error_;
};

} // namespace fm
