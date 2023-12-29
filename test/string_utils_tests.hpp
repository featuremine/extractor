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
 * @file string_utils.hpp
 * @authors Maxim Trokhimtchouk
 * @date 14 Oct 2019
 * @brief File contains tests for string utilities
 *
 * @see http://www.featuremine.com
 */

#include "fmc++/gtestwrap.hpp"
#include "fmc++/strings.hpp"

#include <string>

using namespace fmc;

#define test_success_from_string_view1(T, num)                                 \
  {                                                                            \
    auto str = to_string(num);                                                 \
    vector<char> buf_(str.size());                                             \
    memcpy(buf_.data(), str.data(), str.size());                               \
    string_view view(buf_.data(), str.size());                                 \
    auto result = from_string_view<T>(view);                                   \
    EXPECT_EQ(result.first, (T)num);                                           \
    EXPECT_EQ(result.second, view);                                            \
  }

#define test_success_from_string_view2(T, num)                                 \
  {                                                                            \
    auto str = to_string(num) + "-";                                           \
    vector<char> buf_(str.size());                                             \
    memcpy(buf_.data(), str.data(), str.size());                               \
    string_view view(buf_.data(), str.size());                                 \
    auto result = from_string_view<T>(view);                                   \
    EXPECT_EQ(result.first, (T)num);                                           \
    EXPECT_EQ(result.second, view.substr(0, str.size() - 1));                  \
  }

#define test_fail_from_string_view(T, view)                                    \
  {                                                                            \
    auto result = from_string_view<T>(view);                                   \
    EXPECT_EQ((int64_t)result.first, 0);                                       \
    EXPECT_TRUE(result.second.empty());                                        \
  }

#define test_all_numeric_cases(FUNC, num)                                      \
  {                                                                            \
    FUNC(int8_t, num);                                                         \
    FUNC(int16_t, num);                                                        \
    FUNC(int32_t, num);                                                        \
    FUNC(int64_t, num);                                                        \
    FUNC(uint8_t, num);                                                        \
    FUNC(uint16_t, num);                                                       \
    FUNC(uint32_t, num);                                                       \
    FUNC(uint64_t, num);                                                       \
  }

#define test_all_limit_cases(FUNC)                                             \
  {                                                                            \
    FUNC(int8_t, numeric_limits<int8_t>::max());                               \
    FUNC(int16_t, numeric_limits<int16_t>::max());                             \
    FUNC(int32_t, numeric_limits<int32_t>::max());                             \
    FUNC(int64_t, numeric_limits<int64_t>::max());                             \
    FUNC(uint8_t, numeric_limits<uint8_t>::max());                             \
    FUNC(uint16_t, numeric_limits<uint16_t>::max());                           \
    FUNC(uint32_t, numeric_limits<uint32_t>::max());                           \
    FUNC(uint64_t, numeric_limits<uint64_t>::max());                           \
    FUNC(int8_t, numeric_limits<int8_t>::min());                               \
    FUNC(int16_t, numeric_limits<int16_t>::min());                             \
    FUNC(int32_t, numeric_limits<int32_t>::min());                             \
    FUNC(int64_t, numeric_limits<int64_t>::min());                             \
  }

TEST(string_utils, from_string_view) {
  using namespace std;
  test_all_numeric_cases(test_success_from_string_view1, 0);
  test_all_numeric_cases(test_success_from_string_view1, 1);
  test_all_numeric_cases(test_success_from_string_view1, 10);
  test_all_numeric_cases(test_success_from_string_view2, 0);
  test_all_numeric_cases(test_success_from_string_view2, 1);
  test_all_numeric_cases(test_success_from_string_view2, 10);

  test_all_numeric_cases(test_fail_from_string_view, "-");
  test_all_numeric_cases(test_fail_from_string_view, "");
  test_all_numeric_cases(test_fail_from_string_view, " ");
  test_all_numeric_cases(test_fail_from_string_view, "s10strin");

  test_all_limit_cases(test_success_from_string_view1);
  test_all_limit_cases(test_success_from_string_view2);
}

TEST(string_utils, to_string_view_double) {
  double double_val = 100.0;
  char buf[20];
  while (double_val < 100) {
    EXPECT_EQ(to_string_view_double(&buf[0], double_val, 9),
              string_view(std::to_string(double_val)));
    double_val += 0.000001;
  }
  EXPECT_EQ(to_string_view_double(&buf[0], 0.9999999999, 9), string_view("1"));
  EXPECT_EQ(to_string_view_double(&buf[0], 0.999999999, 9),
            string_view("0.999999999"));
  EXPECT_EQ(to_string_view_double(&buf[0], 0.99999999999, 10),
            string_view("1"));
  EXPECT_EQ(to_string_view_double(&buf[0], 0.9999999999, 10),
            string_view("0.9999999999"));
  EXPECT_EQ(to_string_view_double(&buf[0], 0.9999999999999999, 15),
            string_view("1"));
  EXPECT_EQ(to_string_view_double(&buf[0], 0.999999999999999, 15),
            string_view("0.999999999999999"));
  EXPECT_EQ(to_string_view_double(&buf[0], 0.0, 9), string_view("0"));
  EXPECT_EQ(to_string_view_double(&buf[0], 0.00001, 9), string_view("0.00001"));
  EXPECT_EQ(to_string_view_double(&buf[0], 10.0, 9), string_view("10"));
  EXPECT_EQ(to_string_view_double(&buf[0], 1.0, 9), string_view("1"));
}

TEST(string_utils, to_string_view_int) {

  char buf[9] = {0};
  string_view str_buf(buf, 9);

  for (int int_val = -100000; int_val < 100000; ++int_val) {
    EXPECT_EQ(string(to_string_view_signed(&buf[0], int_val)),
              to_string(int_val));

    char otherbuf[20] = {0};
    if (int_val >= 0) {
      snprintf(otherbuf, sizeof(otherbuf), "%.9i", int_val);
    } else {
      snprintf(otherbuf, sizeof(otherbuf), "%.8i", int_val);
    }

    auto res = to_string_view_signed_fixed_length(str_buf, int_val);
    EXPECT_EQ(string(res), string(otherbuf));

    if (int_val >= 0) {
      auto res = to_string_view_unsigned_fixed_length(str_buf, int_val);
      EXPECT_EQ(string(res), string(otherbuf));

      EXPECT_EQ(string(to_string_view_unsigned(&buf[0], (size_t)int_val)),
                to_string(int_val));
    }
  }

  EXPECT_EQ(to_string_view_signed(buf, 0), string_view("0"));
  EXPECT_EQ(to_string_view_signed(buf, 1), string_view("1"));
  EXPECT_EQ(to_string_view_signed(buf, -1), string_view("-1"));
}
