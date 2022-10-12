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
 * @file type_sys.cpp
 * @author Maxim Trokhimtchouk
 * @date 29 Aug 2017
 * @brief File contains tests for type system
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/type_sys.h"
}

#include "fmc++/gtestwrap.hpp"

#include <iostream>

using namespace std;

TEST(type_sys, create) {
  auto *ts = fm_type_sys_new();

  auto uint32_t1 = fm_base_type_get(ts, FM_TYPE_UINT32);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto uint32_t2 = fm_base_type_get(ts, FM_TYPE_UINT32);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto time64_t2 = fm_base_type_get(ts, FM_TYPE_TIME64);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto bool_t1 = fm_base_type_get(ts, FM_TYPE_BOOL);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);

  ASSERT_NE(uint32_t1, nullptr);
  ASSERT_NE(uint32_t2, nullptr);
  ASSERT_NE(time64_t2, nullptr);
  ASSERT_NE(bool_t1, nullptr);

  EXPECT_TRUE(fm_type_equal(uint32_t1, uint32_t2));
  EXPECT_FALSE(fm_type_equal(uint32_t1, time64_t2));

  auto array_t1 = fm_array_type_get(ts, time64_t2, 13);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto array_t2 = fm_array_type_get(ts, time64_t2, 32);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto array_t3 = fm_array_type_get(ts, uint32_t1, 13);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto array_t4 = fm_array_type_get(ts, uint32_t2, 13);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);

  ASSERT_NE(array_t1, nullptr);
  ASSERT_NE(array_t2, nullptr);
  ASSERT_NE(array_t3, nullptr);
  ASSERT_NE(array_t4, nullptr);

  ASSERT_FALSE(fm_type_equal(array_t1, array_t2));
  ASSERT_FALSE(fm_type_equal(array_t1, array_t3));
  ASSERT_TRUE(fm_type_equal(array_t3, array_t4));

  auto *frame_t0 = fm_frame_type_get(ts, 3, 2, "ts", time64_t2, "price",
                                     uint32_t1, "price", uint32_t2, 2, 3);
  ASSERT_EQ(frame_t0, nullptr);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_DUPLICATE);

  auto *frame_t1 = fm_frame_type_get(ts, 3, 2, "ts", time64_t2, "price",
                                     uint32_t1, "qty", uint32_t2, 2, 3);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto *frame_t2 =
      fm_frame_type_get(ts, 2, 2, "ts", time64_t2, "price", uint32_t2, 2, 3);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto *frame_t3 =
      fm_frame_type_get(ts, 2, 2, "ts", time64_t2, "price", uint32_t1, 2, 3);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto *frame_t4 =
      fm_frame_type_get(ts, 2, 2, "ts", time64_t2, "price", uint32_t1, 2, 4);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto *frame_t5 =
      fm_frame_type_get(ts, 2, 2, "t", time64_t2, "price", uint32_t1, 2, 3);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto *frame_t6 =
      fm_frame_type_get(ts, 2, 1, "t", time64_t2, "price", uint32_t1, 2);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);
  auto *frame_t7 = fm_frame_type_get(ts, 3, 2, "price", uint32_t1, "ts",
                                     time64_t2, "qty", uint32_t2, 2, 3);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_OK);

  auto *frame_t8 =
      fm_frame_type_get(ts, 2, 2, "ts", time64_t2, "price", frame_t1, 2, 3);
  ASSERT_EQ(frame_t8, nullptr);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_CHILD);

  auto array_t5 = fm_array_type_get(ts, frame_t1, 13);
  ASSERT_EQ(array_t5, nullptr);
  EXPECT_EQ(fm_type_sys_errno(ts), FM_TYPE_ERROR_CHILD);

  ASSERT_NE(frame_t1, nullptr);
  ASSERT_NE(frame_t2, nullptr);
  ASSERT_NE(frame_t3, nullptr);
  ASSERT_NE(frame_t4, nullptr);
  ASSERT_NE(frame_t5, nullptr);
  ASSERT_NE(frame_t6, nullptr);
  ASSERT_NE(frame_t7, nullptr);

  EXPECT_FALSE(fm_type_equal(frame_t1, frame_t2));
  EXPECT_FALSE(fm_type_equal(frame_t3, frame_t4));
  EXPECT_FALSE(fm_type_equal(frame_t3, frame_t5));
  EXPECT_FALSE(fm_type_equal(frame_t5, frame_t6));
  EXPECT_TRUE(fm_type_equal(frame_t2, frame_t3));
  EXPECT_TRUE(fm_type_equal(frame_t1, frame_t7));

  auto record_t1 = fm_record_type_get(ts, "record1", 10);
  auto record_t2 = fm_record_type_get(ts, "record1", 20);
  auto record_t3 = fm_record_type_get(ts, "record2", 10);
  auto record_t4 = fm_record_type_get(ts, "record1", 10);

  ASSERT_NE(record_t1, nullptr);
  ASSERT_NE(record_t2, nullptr);
  ASSERT_NE(record_t3, nullptr);
  ASSERT_NE(record_t4, nullptr);

  EXPECT_FALSE(fm_type_equal(record_t1, record_t2));
  EXPECT_FALSE(fm_type_equal(record_t1, record_t3));
  EXPECT_TRUE(fm_type_equal(record_t1, record_t4));

  EXPECT_FALSE(fm_type_equal(uint32_t1, array_t1));
  EXPECT_FALSE(fm_type_equal(array_t1, frame_t1));
  EXPECT_FALSE(fm_type_equal(frame_t1, record_t1));
  EXPECT_FALSE(fm_type_equal(record_t1, array_t1));

  auto *type1 = fm_frame_type_get(
      ts, 6, 1, "receive", fm_base_type_get(ts, FM_TYPE_TIME64), "sequence",
      fm_base_type_get(ts, FM_TYPE_UINT64), "bidqty",
      fm_base_type_get(ts, FM_TYPE_INT32), "askqty",
      fm_base_type_get(ts, FM_TYPE_INT32), "bidprice",
      fm_base_type_get(ts, FM_TYPE_DECIMAL64), "askprice",
      fm_base_type_get(ts, FM_TYPE_DECIMAL64), 1);

  auto *type2 = fm_frame_type_get(
      ts, 6, 1, "receive", fm_base_type_get(ts, FM_TYPE_TIME64), "sequence",
      fm_base_type_get(ts, FM_TYPE_UINT64), "bidprice",
      fm_base_type_get(ts, FM_TYPE_DECIMAL64), "askprice",
      fm_base_type_get(ts, FM_TYPE_DECIMAL64), "bidqty",
      fm_base_type_get(ts, FM_TYPE_INT32), "askqty",
      fm_base_type_get(ts, FM_TYPE_INT32), 1);

  auto *str = fm_type_to_str(type1);
  auto *teststr = "frame(askprice:DECIMAL64,askqty:INT32,bidprice:DECIMAL64,"
                  "bidqty:INT32,receive:TIME64,sequence:UINT64)";
  EXPECT_STREQ(str, teststr);
  free(str);

  EXPECT_TRUE(fm_type_equal(type1, type2));

  auto *chararray =
      fm_array_type_get(ts, fm_base_type_get(ts, FM_TYPE_CHAR), 16);

  auto *type3 = fm_frame_type_get(
      ts, 8, 1, "receive", fm_base_type_get(ts, FM_TYPE_TIME64), "market",
      chararray, "ticker", chararray, "type",
      fm_base_type_get(ts, FM_TYPE_CHAR), "bidprice",
      fm_base_type_get(ts, FM_TYPE_DECIMAL64), "askprice",
      fm_base_type_get(ts, FM_TYPE_DECIMAL64), "bidqty",
      fm_base_type_get(ts, FM_TYPE_INT32), "askqty",
      fm_base_type_get(ts, FM_TYPE_INT32), 0);

  EXPECT_NE(type3, nullptr);
  cout << fm_type_sys_errmsg(ts) << endl;

  fm_type_sys_del(ts);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
