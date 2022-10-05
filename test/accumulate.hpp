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
 * @file accumulate.hpp
 * @author Andres Rangel
 * @date 18 Sep 2018
 * @brief File contains tests for split operator
 *
 * Replays the content of messagepack encoded file
 * Validates the context
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
#include "fmc/platform.h"
#include <fstream>
#include <iostream>
#include <vector>

using namespace fmc;

TEST(accumulate, accumulate_data) {
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

  auto *csv_play_param_t = fm_tuple_type_get(
      tsys, 2, fm_cstring_type_get(tsys),
      fm_tuple_type_get(tsys, 8, row_desc_t, row_desc_t, row_desc_t, row_desc_t,
                        row_desc_t, row_desc_t, row_desc_t, row_desc_t));

  auto *comp_A = fm_comp_decl(
      sys, g, "csv_play", 0, csv_play_param_t,
      (src_dir + "/data/pandas_play_file.csv").c_str(), "receive",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "market",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16), "",
      "ticker",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16), "",
      "type", fm_base_type_get(tsys, FM_TYPE_CHAR), "", "bidprice",
      fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "", "askprice",
      fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "", "bidqty",
      fm_base_type_get(tsys, FM_TYPE_INT32), "", "askqty",
      fm_base_type_get(tsys, FM_TYPE_INT32), "");

  ASSERT_NE(comp_A, nullptr);
  auto *comp_B = fm_comp_decl(sys, g, "accumulate", 1, nullptr, comp_A);

  ASSERT_NE(comp_B, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);

  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);
    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  auto *result = fm_data_get(fm_result_ref_get(comp_B));

  ASSERT_NE(result, nullptr);

  auto f_decl = fm_frame_type(result);

  ASSERT_NE(f_decl, nullptr);

  auto r_field = fm_type_frame_field_idx(f_decl, "receive");
  auto m_field = fm_type_frame_field_idx(f_decl, "market");
  auto tick_field = fm_type_frame_field_idx(f_decl, "ticker");
  auto t_field = fm_type_frame_field_idx(f_decl, "type");
  auto bp_field = fm_type_frame_field_idx(f_decl, "bidprice");
  auto ap_field = fm_type_frame_field_idx(f_decl, "askprice");
  auto bq_field = fm_type_frame_field_idx(f_decl, "bidqty");
  auto aq_field = fm_type_frame_field_idx(f_decl, "askqty");

  FILE *f = fopen((src_dir + "/data/accumulate.test.csv").c_str(), "w");
  fprintf(f, "%s",
          "receive,ticker,market,type,bidprice,askprice,bidqty,"
          "askqty\n");
  for (int i = 0; i < fm_frame_dim(result, 0); ++i) {
    fprintf(
        f, "%ld,%s,%s,%c,%g,%g,%d,%d\n",
        fm_time64_to_nanos(
            *(fm_time64_t *)fm_frame_get_cptr1(result, r_field, i)),
        string((char *)fm_frame_get_cptr1(result, tick_field, i), 16).c_str(),
        string((char *)fm_frame_get_cptr1(result, m_field, i), 16).c_str(),
        *(char *)fm_frame_get_cptr1(result, t_field, i),
        fm_decimal64_to_double(
            *(fm_decimal64_t *)fm_frame_get_cptr1(result, bp_field, i)),
        fm_decimal64_to_double(
            *(fm_decimal64_t *)fm_frame_get_cptr1(result, ap_field, i)),
        *(int32_t *)fm_frame_get_cptr1(result, bq_field, i),
        *(int32_t *)fm_frame_get_cptr1(result, aq_field, i));
  }
  fclose(f);

  fm_comp_sys_del(sys);

  EXPECT_BASE((src_dir + "/data/accumulate.base.csv").c_str(),
              (src_dir + "/data/"
                         "accumulate.test.csv")
                  .c_str());
}
