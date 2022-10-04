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
 * @file logical_or.hpp
 * @authors Andres Rangel
 * @date 22 Aug 2018
 * @brief File contains tests for the "or" logical operator feature
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "comp_sys.h"
#include "frame.h"
#include "std_comp.h"
#include "stream_ctx.h"
#include "type_sys.h"
}

#include <fmc++/gtestwrap.hpp>

using namespace fmc;

TEST(or_comp, multiple_field) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new((src_dir + "/test.lic").c_str(), &errstring);
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

  auto *or_param_t = fm_tuple_type_get(tsys, 0);

  auto *greater_param_t = fm_tuple_type_get(tsys, 0);

  auto *cstr_t = fm_cstring_type_get(tsys);

  auto *comp_A =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_one.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "val2", fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_A, nullptr);

  auto *comp_B =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_two.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "val2", fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_B, nullptr);

  auto *comp_BA =
      fm_comp_decl(sys, g, "greater", 2, greater_param_t, comp_B, comp_A);
  ASSERT_NE(comp_BA, nullptr);

  auto *comp_AB =
      fm_comp_decl(sys, g, "greater", 2, greater_param_t, comp_B, comp_A);
  ASSERT_NE(comp_AB, nullptr);

  auto *comp_C =
      fm_comp_decl(sys, g, "logical_or", 2, or_param_t, comp_AB, comp_BA);
  ASSERT_NE(comp_C, nullptr);

  auto *comp_D = fm_comp_decl(sys, g, "csv_record", 1, cstr_t, comp_C,
                              (src_dir + "/data/or_multiple.test.csv").c_str());
  ASSERT_NE(comp_D, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_comp_sys_del(sys);

  EXPECT_BASE((src_dir + "/data/or_multiple.base.csv").c_str(),
              (src_dir + "/data/"
                         "or_multiple.test.csv")
                  .c_str());
}

TEST(or_comp, single_field) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new((src_dir + "/test.lic").c_str(), &errstring);
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

  auto *or_param_t = fm_tuple_type_get(tsys, 0);

  auto *greater_param_t = fm_tuple_type_get(tsys, 0);

  auto *cstr_t = fm_cstring_type_get(tsys);

  auto *field_param_t = fm_tuple_type_get(tsys, 1, cstr_t);

  auto *comp_A =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_one.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "val2", fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_A, nullptr);

  auto *comp_B =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_two.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "val2", fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_B, nullptr);

  auto *comp_BA =
      fm_comp_decl(sys, g, "greater", 2, greater_param_t, comp_B, comp_A);
  ASSERT_NE(comp_BA, nullptr);

  auto *comp_AB =
      fm_comp_decl(sys, g, "greater", 2, greater_param_t, comp_B, comp_A);
  ASSERT_NE(comp_AB, nullptr);

  auto *comp_AF =
      fm_comp_decl(sys, g, "field", 1, field_param_t, comp_AB, "val1");
  ASSERT_NE(comp_AF, nullptr);

  auto *comp_BF =
      fm_comp_decl(sys, g, "field", 1, field_param_t, comp_BA, "val1");
  ASSERT_NE(comp_BF, nullptr);

  auto *comp_C =
      fm_comp_decl(sys, g, "logical_or", 2, or_param_t, comp_AF, comp_BF);
  ASSERT_NE(comp_C, nullptr);

  auto *comp_D = fm_comp_decl(sys, g, "csv_record", 1, cstr_t, comp_C,
                              (src_dir + "/data/or_single.test.csv").c_str());
  ASSERT_NE(comp_D, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_comp_sys_del(sys);

  EXPECT_BASE((src_dir + "/data/or_single.base.csv").c_str(),
              (src_dir + "/data/"
                         "or_single.test.csv")
                  .c_str());
}

TEST(or_comp, multiple_comps) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new((src_dir + "/test.lic").c_str(), &errstring);
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

  auto *or_param_t = fm_tuple_type_get(tsys, 0);

  auto *greater_param_t = fm_tuple_type_get(tsys, 0);

  auto *cstr_t = fm_cstring_type_get(tsys);

  auto *comp_A =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_one.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "val2", fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_A, nullptr);

  auto *comp_B =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_two.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "val2", fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_B, nullptr);

  auto *comp_C =
      fm_comp_decl(sys, g, "csv_play", 0, csv_play_param_t,
                   (src_dir + "/data/logical_op_file_three.csv").c_str(),
                   "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "val1", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "val2", fm_base_type_get(tsys, FM_TYPE_INT32), "");
  ASSERT_NE(comp_C, nullptr);

  auto *comp_BA =
      fm_comp_decl(sys, g, "greater", 2, greater_param_t, comp_B, comp_A);
  ASSERT_NE(comp_BA, nullptr);

  auto *comp_CA =
      fm_comp_decl(sys, g, "greater", 2, greater_param_t, comp_C, comp_A);
  ASSERT_NE(comp_CA, nullptr);

  auto *comp_BC =
      fm_comp_decl(sys, g, "greater", 2, greater_param_t, comp_B, comp_C);
  ASSERT_NE(comp_BC, nullptr);

  auto *comp_D = fm_comp_decl(sys, g, "logical_or", 3, or_param_t, comp_BC,
                              comp_BA, comp_CA);

  ASSERT_NE(comp_D, nullptr);

  auto *comp_E =
      fm_comp_decl(sys, g, "csv_record", 1, cstr_t, comp_D,
                   (src_dir + "/data/or_multiple_comps.test.csv").c_str());
  ASSERT_NE(comp_E, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_comp_sys_del(sys);

  EXPECT_BASE((src_dir + "/data/or_multiple_comps.base.csv").c_str(),
              (src_dir + "/data/or_multiple_comps.test.csv").c_str());
}
