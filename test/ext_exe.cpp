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
 * @file ext_test.cpp
 * @author Maxim Trokhimtchouk
 * @date 29 Nov 2017
 * @brief Timer extension sample
 *
 * @see http://www.featuremine.com
 */

#include "extractor/comp_sys.h"
#include "extractor/std_comp.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wexpansion-to-defined"
#endif
#include <gtest/gtest.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include "fmc++/time.hpp"
#include <string>

TEST(ext_lib, check) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new(&errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  const char *name = "__none__";
  ASSERT_FALSE(fm_comp_sys_ext_load(sys, name));

  fm_comp_sys_del(sys);
}

TEST(ext_lib, stream) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new(&errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);
  ASSERT_TRUE(fm_comp_sys_std_comp(sys));

  const char *name = "ext_lib";
  fmc_error_t *error;
  fm_comp_sys_paths_set_default(sys, &error);
  ASSERT_EQ(error, nullptr);
  auto res = fm_comp_sys_ext_load(sys, name);

  ASSERT_TRUE(res);

  auto *g = fm_comp_graph_get(sys);
  auto *tsys = fm_type_sys_get(sys);

  auto *argtype =
      fm_tuple_type_get(tsys, 1, fm_base_type_get(tsys, FM_TYPE_TIME64));

  auto period = fmc_time64_from_seconds(5);
  auto *comp_A = fm_comp_decl(sys, g, "timer", 0, argtype, period);
  ASSERT_NE(comp_A, nullptr);
  auto *comp_B = fm_comp_decl(sys, g, "timer_count", 1, NULL, comp_A);
  ASSERT_NE(comp_B, nullptr);
  auto *comp_C = fm_comp_decl(sys, g, "timer_count", 1, NULL, comp_A);
  ASSERT_NE(comp_C, nullptr);
  auto *comp_D =
      fm_comp_decl(sys, g, "timer_count_avg", 2, NULL, comp_B, comp_C);
  ASSERT_NE(comp_D, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  int counter = 0;
  fmc_time64_t now = fmc_time64_from_seconds(1400000001);
  do {
    fm_stream_ctx_proc_one(ctx, now);
    now = fm_stream_ctx_next_time(ctx);
    ++counter;
  } while (now < fmc_time64_from_seconds(1400000102));

  ASSERT_EQ(fmc_time64_to_nanos(now), 1400000105000000000L);
  ASSERT_EQ(counter, 21);
  fm_comp_sys_del(sys);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
