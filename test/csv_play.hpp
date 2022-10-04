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
 * @file csv_play.hpp
 * @author Maxim Trokhimtchouk
 * @date 18 Aug 2017
 * @brief File contains tests for CSV play operator
 *
 * Generates random CSV file in a book delta format
 * Replays the content of the CSV
 * Validates the context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/comp_sys.h"
#include "extractor/frame.h"
#include "csv_play.h"
#include "csv_record.h"
#include "identity.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_sys.h"
}

#include <fmc++/gtestwrap.hpp>

using namespace fmc;

void csv_play_clbck(const fm_frame_t *frame, fm_frame_clbck_cl cl,
                    fm_call_ctx_t *) {}

TEST(csv_plays, check) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new((src_dir + "/test.lic").c_str(), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  fm_comp_type_add(sys, &fm_comp_csv_play);

  auto *g = fm_comp_graph_get(sys);
  auto *tsys = fm_type_sys_get(sys);
  auto *comp_A = fm_comp_decl(sys, g, "csv_play", 0, nullptr);
  ASSERT_EQ(comp_A, nullptr);

  string errmsg = "expect a file name and a tuple of field descriptions;"
                  "first description must be an index field";
  ASSERT_EQ(errmsg, fm_type_sys_errmsg(tsys));

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  comp_A = fm_comp_decl(
      sys, g, "csv_play", 0, csv_play_param_t, "__does_not_exist__",
      "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");
  ASSERT_NE(comp_A, nullptr);

  fm_comp_clbck_set(comp_A, &csv_play_clbck, &testout);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_EQ(ctx, nullptr);

  ASSERT_TRUE(fm_comp_sys_is_error(sys));

  fm_comp_sys_del(sys);
}

TEST(csv_plays, stream) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new((src_dir + "/test.lic").c_str(), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  fm_comp_type_add(sys, &fm_comp_csv_play);
  fm_comp_type_add(sys, &fm_comp_csv_record);
  fm_comp_type_add(sys, &fm_comp_identity);

  auto *g = fm_comp_graph_get(sys);
  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *comp_A = fm_comp_decl(
      sys, g, "csv_play", 0, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");
  ASSERT_NE(comp_A, nullptr);

  auto *cstr_t = fm_cstring_type_get(tsys);
  auto *comp_B =
      fm_comp_decl(sys, g, "csv_record", 1, cstr_t, comp_A,
                   (src_dir + "/data/csv_play_file_2.test.csv").c_str());
  ASSERT_NE(comp_B, nullptr);

  ASSERT_STREQ("csv_play_0", fm_comp_name(comp_A));
  ASSERT_STREQ("csv_record_0", fm_comp_name(comp_B));

  auto *comp_C =
      fm_comp_decl2(sys, g, "csv_record", "csv_record_0", 1, cstr_t, comp_A,
                    (src_dir + "/data/csv_play_file.test.csv").c_str());
  ASSERT_EQ(comp_C, nullptr);
  ASSERT_STREQ("[ERROR]\t(comp_sys) computation with name "
               "csv_record_0 already exists",
               fm_comp_sys_error_msg(sys));

  fm_comp_clbck_set(comp_A, &csv_play_clbck, &testout);
  fm_result_ref_get(comp_A);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_comp_sys_del(sys);

  EXPECT_BASE((src_dir + "/data/csv_play_file_2.base.csv").c_str(),
              (src_dir + "/data/csv_play_file_2.test.csv").c_str());
}

TEST(csv_plays, identity) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new((src_dir + "/test.lic").c_str(), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  fm_comp_type_add(sys, &fm_comp_csv_play);
  fm_comp_type_add(sys, &fm_comp_csv_record);
  fm_comp_type_add(sys, &fm_comp_identity);

  auto *g = fm_comp_graph_get(sys);
  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *comp_A = fm_comp_decl(
      sys, g, "csv_play", 0, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");
  ASSERT_NE(comp_A, nullptr);

  auto *id_op = fm_comp_decl(sys, g, "identity", 1, nullptr, comp_A);
  ASSERT_NE(id_op, nullptr);

  auto *cstr_t = fm_cstring_type_get(tsys);
  auto *comp_B =
      fm_comp_decl(sys, g, "csv_record", 1, cstr_t, id_op,
                   (src_dir + "/data/csv_play_file_3.test.csv").c_str());
  ASSERT_NE(comp_B, nullptr);

  ASSERT_STREQ("csv_play_0", fm_comp_name(comp_A));
  ASSERT_STREQ("csv_record_0", fm_comp_name(comp_B));

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_comp_sys_del(sys);

  EXPECT_BASE((src_dir + "/data/csv_play_file_3.base.csv").c_str(),
              (src_dir + "/data/csv_play_file_3.test.csv").c_str());
}
