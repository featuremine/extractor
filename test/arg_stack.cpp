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
 * @file arg_stack.cpp
 * @author Maxim Trokhimtchouk
 * @date 8 Aug 2017
 * @brief File contains tests for computational graph object
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "arg_stack.h"
}

#include <fmc++/gtestwrap.hpp>

void receive_args(fm_arg_stack_t args) {
  const char *teststr = STACK_POP(args, const char *);
  ASSERT_STREQ(teststr, "hello");

  float num1 = STACK_POP(args, float);
  ASSERT_FLOAT_EQ(num1, 0.1);

  double num2 = STACK_POP(args, double);
  ASSERT_DOUBLE_EQ(num2, 0.2);

  int8_t num3 = STACK_POP(args, int8_t);
  ASSERT_EQ(num3, 3);

  int64_t num4 = STACK_POP(args, int64_t);
  ASSERT_EQ(num4, 4);

  int32_t num5 = STACK_POP(args, int32_t);
  ASSERT_EQ(num5, 5);

  int16_t num6 = STACK_POP(args, int16_t);
  ASSERT_EQ(num6, 6);

  int8_t num7 = STACK_POP(args, int8_t);
  ASSERT_EQ(num7, 7);
}

TEST(arg_stack, fixed) {
  using namespace std;
  STACK(48, s);
  const char *teststr = "hello"; // +8
  float num1 = 0.1;              // +4 +8
  double num2 = 0.2;             // +8 +16
  int8_t num3 = 3;               // +1 +24
  int64_t num4 = 4;              // +8 +32
  int32_t num5 = 5;              // +4 +40
  int16_t num6 = 6;              // +2 +4 +40
  int8_t num7 = 7;               // +1 +6 +40
  int64_t num8 = 8;              // +8 +48

  ASSERT_TRUE(STACK_CHECK(s, teststr));
  STACK_PUSH(s, teststr);

  ASSERT_TRUE(STACK_CHECK(s, num1));
  STACK_PUSH(s, num1);

  ASSERT_TRUE(STACK_CHECK(s, num2));
  STACK_PUSH(s, num2);

  ASSERT_TRUE(STACK_CHECK(s, num3));
  STACK_PUSH(s, num3);

  ASSERT_TRUE(STACK_CHECK(s, num4));
  STACK_PUSH(s, num4);

  ASSERT_TRUE(STACK_CHECK(s, num5));
  STACK_PUSH(s, num5);

  ASSERT_TRUE(STACK_CHECK(s, num6));
  STACK_PUSH(s, num6);

  ASSERT_TRUE(STACK_CHECK(s, num7));
  STACK_PUSH(s, num7);

  ASSERT_FALSE(STACK_CHECK(s, num8));
  receive_args(STACK_ARGS(s));
}

TEST(arg_stack, heap) {
  using namespace std;
  auto *s = fm_arg_stack_alloc(32);
  ASSERT_NE(s, nullptr);
  const char *teststr = "hello"; // +8
  float num1 = 0.1;              // +4 +8
  double num2 = 0.2;             // +8 +16
  int8_t num3 = 3;               // +1 +24
  int64_t num4 = 4;              // +8 +32
  int32_t num5 = 5;              // +4 +40
  int16_t num6 = 6;              // +2 +4 +40
  int8_t num7 = 7;               // +1 +6 +40

  ASSERT_TRUE(HEAP_STACK_PUSH(s, teststr));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, num1));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, num2));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, num3));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, num4));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, num5));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, num6));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, num7));
  auto *copy = fm_arg_stack_copy(s);
  ASSERT_NE(copy, nullptr);
  receive_args(fm_arg_stack_args(s));
  receive_args(fm_arg_stack_args(copy));
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
