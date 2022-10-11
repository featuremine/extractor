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
 * @file book_build.hpp
 * @authors Maxim Trokhimtchouk
 * @date 31 Oct 2018
 * @brief File contains tests for book building operator feature
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/comp_sys.h"
#include "extractor/frame.h"
#include "extractor/python/py_extractor.h"
#include "extractor/std_comp.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_sys.h"
}

#include "fmc++/gtestwrap.hpp"

using namespace fmc;

void book_building_test(const char *input_file) {
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
  fm_comp_sys_py_comp(sys);

  auto *g = fm_comp_graph_get(sys);
  auto *tsys = fm_type_sys_get(sys);

  auto *cstring_t = fm_cstring_type_get(tsys);
  auto *type_t = fm_type_type_get(tsys);
  auto *empty_t = fm_tuple_type_get(tsys, 0);
  auto *bsp_param_t = fm_tuple_type_get(
      tsys, 2, cstring_t,
      fm_tuple_type_get(tsys, 3, cstring_t, cstring_t, cstring_t));
  auto *chararray16 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16);
  auto *join_param_t = fm_tuple_type_get(
      tsys, 3, cstring_t, type_t,
      fm_tuple_type_get(tsys, 3, cstring_t, cstring_t, cstring_t));

  auto *comp_A = fm_comp_decl(sys, g, "book_play_split", 0, bsp_param_t,
                              (src_dir + "/data/book.base.ore").c_str(),
                              "2_YEAR", "3_YEAR", "10_YEAR");

  ASSERT_NE(comp_A, nullptr);

  auto *book_build1_op = fm_comp_decl(sys, g, "book_build", 1, empty_t, comp_A);
  auto *book_build2_op = fm_comp_decl(sys, g, "book_build", 1, empty_t, comp_A);
  auto *book_build3_op = fm_comp_decl(sys, g, "book_build", 1, empty_t, comp_A);

  ASSERT_NE(book_build1_op, nullptr);
  ASSERT_NE(book_build2_op, nullptr);
  ASSERT_NE(book_build3_op, nullptr);

  auto join_op = fm_comp_decl(sys, g, "join", 3, join_param_t, book_build1_op,
                              book_build2_op, book_build3_op, "imnt",
                              chararray16, "2_YEAR", "3_YEAR", "10_YEAR");

  fm_comp_decl2(sys, g, "csv_record", nullptr, 1, cstring_t, join_op,
                (src_dir + "/data/book_levels.extr.test.csv").c_str());

  auto *ctx = fm_stream_ctx_get(sys, g);
  cout << fm_comp_sys_error_msg(sys) << endl;
  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_comp_sys_del(sys);
}

TEST(book_build, single_field) {
  book_building_test((src_dir + "/data/book.base.ore").c_str());
}

TEST(book_build, non_decorated_trades) {
  book_building_test((src_dir + "/data/book.base.old.ore").c_str());
}
