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
 * @file less_or_equal.hpp
 * @authors Andres Rangel
 * @date 22 Aug 2018
 * @brief File contains tests for the "less_or_equal" logical operator feature
 *
 * @see http://www.featuremine.com
 */

#include "extractor/comp_sys.h"
#include "extractor/frame.h"
#include "extractor/std_comp.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_sys.h"

#include "fmc++/gtestwrap.hpp"

using namespace fmc;

TEST(less_or_equal_comp, multiple_field) {
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

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 3, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *less_or_equal_param_t = fm_tuple_type_get(tsys, 0);

  auto *cstr_t = fm_cstring_type_get(tsys);

  auto *comp_A =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_one.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_RPRICE), "", "val2",
                   fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_A, nullptr);

  auto *comp_B =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_two.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_RPRICE), "", "val2",
                   fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_B, nullptr);

  auto *comp_C = fm_comp_decl(sys, g, "less_equal", 2, less_or_equal_param_t,
                              comp_B, comp_A);
  ASSERT_NE(comp_C, nullptr);

  auto *comp_D =
      fm_comp_decl(sys, g, "csv_record", 1, cstr_t, comp_C,
                   (src_dir + "/data/less_or_equal_multiple.test.csv").c_str());
  ASSERT_NE(comp_D, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fmc_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fmc_time64_is_end(now));

  fm_comp_sys_del(sys);

  EXPECT_BASE((src_dir + "/data/less_or_equal_multiple.base.csv").c_str(),
              (src_dir + "/data/less_or_equal_multiple.test.csv").c_str());
}

TEST(less_or_equal_comp, string_field) {
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

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *less_or_equal_param_t = fm_tuple_type_get(tsys, 0);

  auto *cstr_t = fm_cstring_type_get(tsys);

  auto *comp_A = fm_comp_decl(
      sys, g, "csv_play", 0, csv_play_param_t,
      (src_dir + "/data/logical_op_file_one.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "val1",
      fm_base_type_get(tsys, FM_TYPE_RPRICE), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_INT32), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16), "");
  ASSERT_NE(comp_A, nullptr);

  auto *comp_B = fm_comp_decl(
      sys, g, "csv_play", 0, csv_play_param_t,
      (src_dir + "/data/logical_op_file_two.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "val1",
      fm_base_type_get(tsys, FM_TYPE_RPRICE), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_INT32), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16), "");
  ASSERT_NE(comp_B, nullptr);

  auto *comp_C = fm_comp_decl(sys, g, "less_equal", 2, less_or_equal_param_t,
                              comp_B, comp_A);
  ASSERT_NE(comp_C, nullptr);

  auto *comp_D =
      fm_comp_decl(sys, g, "csv_record", 1, cstr_t, comp_C,
                   (src_dir + "/data/less_or_equal_string.test.csv").c_str());
  ASSERT_NE(comp_D, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fmc_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fmc_time64_is_end(now));

  fm_comp_sys_del(sys);

  EXPECT_BASE((src_dir + "/data/less_or_equal_string.base.csv").c_str(),
              (src_dir + "/data/"
                         "less_or_equal_string.test.csv")
                  .c_str());
}

TEST(less_or_equal_comp, single_field) {
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

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 3, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *less_or_equal_param_t = fm_tuple_type_get(tsys, 0);

  auto *cstr_t = fm_cstring_type_get(tsys);

  auto *field_param_t = fm_tuple_type_get(tsys, 1, cstr_t);

  auto *comp_A =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_one.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_RPRICE), "", "val2",
                   fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_A, nullptr);

  auto *comp_B =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_two.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_RPRICE), "", "val2",
                   fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_B, nullptr);

  auto *comp_AF =
      fm_comp_decl(sys, g, "field", 1, field_param_t, comp_A, "val1");
  ASSERT_NE(comp_AF, nullptr);

  auto *comp_BF =
      fm_comp_decl(sys, g, "field", 1, field_param_t, comp_B, "val1");
  ASSERT_NE(comp_BF, nullptr);

  auto *comp_C = fm_comp_decl(sys, g, "less_equal", 2, less_or_equal_param_t,
                              comp_BF, comp_AF);
  ASSERT_NE(comp_C, nullptr);

  auto *comp_D =
      fm_comp_decl(sys, g, "csv_record", 1, cstr_t, comp_C,
                   (src_dir + "/data/less_or_equal_single.test.csv").c_str());
  ASSERT_NE(comp_D, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fmc_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fmc_time64_is_end(now));

  fm_comp_sys_del(sys);

  EXPECT_BASE((src_dir + "/data/less_or_equal_single.base.csv").c_str(),
              (src_dir + "/data/less_or_equal_single.test.csv").c_str());
}
