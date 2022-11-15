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
 * @date 8 Aug 2017
 * @brief File contains tests for computational graph object
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "arg_serial.h"
#include "extractor/arg_stack.h"
}

#include "fmc++/gtestwrap.hpp"
#include <string_view>

using namespace std;

static bool string_view_reader(void *data, size_t limit, void *closure) {
  auto *view = (string_view *)closure;
  if (view->size() < limit)
    return false;
  memcpy(data, view->data(), limit);
  *view = view->substr(limit);
  return true;
}

TEST(arg_serial, simple) {
  using namespace std;
  auto *tsys = fm_type_sys_new();

  auto cstring_t = fm_cstring_type_get(tsys);
  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, cstring_t, fm_type_type_get(tsys), cstring_t);

  auto *mp_play_param_t = fm_tuple_type_get(
      tsys, 2, cstring_t,
      fm_tuple_type_get(tsys, 7, row_desc_t, row_desc_t, row_desc_t, row_desc_t,
                        row_desc_t, row_desc_t, row_desc_t));

  auto *chararray16 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16);
  auto *chararray32 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 32);

  auto *s = fm_arg_stack_alloc(32);
  ASSERT_NE(s, nullptr);

  auto *time_t = fm_base_type_get(tsys, FM_TYPE_TIME64);
  auto *decimal_t = fm_base_type_get(tsys, FM_TYPE_RPRICE);
  auto *i32_t = fm_base_type_get(tsys, FM_TYPE_INT32);
  auto *str1 = "../test/sip_quotes_20171018.base.mp";
  auto *str2 = "receive";
  auto *str3 = "";
  auto *str4 = "ticker";
  auto *str5 = "market";
  auto *str6 = "bidprice";
  auto *str7 = "askprice";
  auto *str8 = "bidqty";
  auto *str9 = "askqty";
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str1));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str2));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, time_t));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str3));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str4));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, chararray16));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str3));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str5));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, chararray32));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str3));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str6));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, decimal_t));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str3));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str7));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, decimal_t));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str3));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str8));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, i32_t));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str3));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str9));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, i32_t));
  ASSERT_TRUE(HEAP_STACK_PUSH(s, str3));

  auto *type_str = fm_type_to_str(mp_play_param_t);
  auto *td = fm_type_from_str(tsys, type_str, strlen(type_str));
  ASSERT_TRUE(fm_type_equal(mp_play_param_t, td));

  auto *buf = fm_arg_buffer_new(mp_play_param_t, fm_arg_stack_args(s));
  const char *dump = nullptr;
  auto size = fm_arg_buffer_dump(buf, &dump);
  string_view view(dump, size);
  string view_str = string(view);

  cout << view_str << endl;

  fm_type_decl_cp td_2 = nullptr;
  fm_arg_stack_t *s_2 = nullptr;
  fm_arg_buffer_t *buf_2 =
      fm_arg_read(tsys, &td_2, &s_2, string_view_reader, &view);
  ASSERT_NE(td_2, nullptr);
  ASSERT_TRUE(fm_type_equal(mp_play_param_t, td_2));

  auto *buf_3 = fm_arg_buffer_new(td_2, fm_arg_stack_args(s_2));
  const char *dump_3 = nullptr;
  auto size_3 = fm_arg_buffer_dump(buf_3, &dump_3);
  string_view view_3(dump_3, size_3);
  string view_str_3 = string(view_3);
  cout << view_str_3 << endl;
  ASSERT_EQ(view_str, view_str_3);

  fm_arg_stack_free(s_2);
  fm_arg_buffer_del(buf_3);
  fm_arg_buffer_del(buf_2);
  fm_arg_buffer_del(buf);

  fm_type_sys_del(tsys);
}

struct memory_read {
  const char *begin;
  const char *end;
  const char *ptr;
};

bool memory_reader(void *data, size_t limit, void *closure) {
  auto buf = (memory_read *)closure;
  if (buf->ptr + limit > buf->end)
    return false;
  memcpy(data, buf->ptr, limit);
  buf->ptr += limit;
  return true;
}

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

  auto *tsys = fm_type_sys_new();
  auto cstring_t = fm_cstring_type_get(tsys);
  auto *i8_t = fm_base_type_get(tsys, FM_TYPE_INT8);
  auto *i16_t = fm_base_type_get(tsys, FM_TYPE_INT16);
  auto *i32_t = fm_base_type_get(tsys, FM_TYPE_INT32);
  auto *i64_t = fm_base_type_get(tsys, FM_TYPE_INT64);
  auto *float_t = fm_base_type_get(tsys, FM_TYPE_FLOAT32);
  auto *double_t = fm_base_type_get(tsys, FM_TYPE_FLOAT64);

  auto *td =
      fm_tuple_type_get(tsys, 4, cstring_t, float_t,
                        fm_tuple_type_get(tsys, 3, double_t, i8_t, i64_t),
                        fm_tuple_type_get(tsys, 3, i32_t, i16_t, i8_t));
  ASSERT_NE(td, nullptr);

  auto *buf = fm_arg_buffer_new(td, STACK_ARGS(s));
  const char *dump = nullptr;
  auto size = fm_arg_buffer_dump(buf, &dump);
  cout.write(dump, size);

  memory_read rd = {dump, dump + size, dump};
  fm_type_decl_cp td_2 = nullptr;
  fm_arg_stack_t *s_2 = nullptr;
  fm_arg_buffer_t *buf_2 = fm_arg_read(tsys, &td_2, &s_2, memory_reader, &rd);
  ASSERT_NE(td_2, nullptr);
  ASSERT_TRUE(fm_type_equal(td, td_2));
  receive_args(fm_arg_stack_args(s_2));
  fm_arg_stack_free(s_2);
  fm_arg_buffer_del(buf_2);
  fm_arg_buffer_del(buf);
  fm_type_sys_del(tsys);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
