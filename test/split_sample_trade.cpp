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
 * @file split_sample_trade.cpp
 * @author Andres Rangel
 * @date 18 Aug 2017
 * @brief File contains tests for messagepack play operator
 *
 * This file uses an input file not included with the repository.
 * The test was built to measure performance.
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/comp_sys.h"
#include "extractor/frame.h"
#include "extractor/std_comp.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_sys.h"
}

#include "fmc++/gtestwrap.hpp"
#include <iostream>

void csv_play_clbck(const fm_frame_t *frame, fm_frame_clbck_cl cl,
                    fm_call_ctx_t *) {}

TEST(csv_plays, identity) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new(&errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  fm_comp_sys_std_comp(sys);

  auto *g = fm_comp_graph_get(sys);
  auto *tsys = fm_type_sys_get(sys);

  auto cstring_t = fm_cstring_type_get(tsys);
  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, cstring_t, fm_type_type_get(tsys), cstring_t);

  auto *mp_play_param_t = fm_tuple_type_get(
      tsys, 2, cstring_t,
      fm_tuple_type_get(tsys, 6, row_desc_t, row_desc_t, row_desc_t, row_desc_t,
                        row_desc_t, row_desc_t));

  auto *chararray16 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16);
  auto *chararray32 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 32);

  auto *comp_A = fm_comp_decl(
      sys, g, "mp_play", 0, mp_play_param_t, "../test/data/trade_20161003.mp",
      "receive", fm_base_type_get(tsys, FM_TYPE_TIME64), "", "ticker",
      chararray16, "", "market", chararray32, "", "price",
      fm_base_type_get(tsys, FM_TYPE_RPRICE), "", "qty",
      fm_base_type_get(tsys, FM_TYPE_INT32), "", "side",
      fm_base_type_get(tsys, FM_TYPE_INT32), "");

  ASSERT_NE(comp_A, nullptr);

  auto *split_param_t = fm_tuple_type_get(
      tsys, 2, cstring_t,
      fm_tuple_type_get(tsys, 3, cstring_t, cstring_t, cstring_t));

  auto split_op = fm_comp_decl(sys, g, "split", 1, split_param_t, comp_A,
                               "market", "NYSEArca", "NASDAQOMX", "NYSEMKT");

  auto *id1_op = fm_comp_decl(sys, g, "identity", 1, nullptr, split_op);
  auto *id2_op = fm_comp_decl(sys, g, "identity", 1, nullptr, split_op);
  auto *id3_op = fm_comp_decl(sys, g, "identity", 1, nullptr, split_op);
  ASSERT_NE(id1_op, nullptr);
  ASSERT_NE(id2_op, nullptr);
  ASSERT_NE(id3_op, nullptr);

  fm_comp_clbck_set(id1_op, &csv_play_clbck, (fm_frame_clbck_cl) "NYSEArca");
  fm_comp_clbck_set(id2_op, &csv_play_clbck, (fm_frame_clbck_cl) "NASDAQOMX");
  fm_comp_clbck_set(id3_op, &csv_play_clbck, (fm_frame_clbck_cl) "NYSEMKT");

  auto *csv_rec1 = fm_comp_decl(sys, g, "csv_record", 1, cstring_t, id1_op,
                                "../test/data/id1_op.test.csv");
  auto *csv_rec2 = fm_comp_decl(sys, g, "csv_record", 1, cstring_t, id2_op,
                                "../test/data/id2_op.test.csv");
  auto *csv_rec3 = fm_comp_decl(sys, g, "csv_record", 1, cstring_t, id3_op,
                                "../test/data/id3_op.test.csv");
  ASSERT_NE(csv_rec1, nullptr);
  ASSERT_NE(csv_rec2, nullptr);
  ASSERT_NE(csv_rec3, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fmc_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fmc_time64_is_end(now));

  fm_comp_sys_del(sys);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
