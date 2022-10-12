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
 * @file split_by.hpp
 * @author Andres Rangel
 * @date 23 Jan 2019
 * @brief File contains C++ implementation of the computational system
 *
 * This file contains implementation of the computational system
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/comp_sys.h"
#include "extractor/frame.h"
#include "extractor/module.h"
#include "extractor/std_comp.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_sys.h"
}

#include "fmc++/gtestwrap.hpp"
#include <iostream>

using namespace fmc;

fm_module_t *mod_with_delay(fm_type_sys_t *tsys) {
  fm_module_comp_t *module_inputs[1];

  auto *m = fm_module_new(nullptr, 1, module_inputs);

  auto *time_lag_params_t =
      fm_tuple_type_get(tsys, 2, fm_base_type_get(tsys, FM_TYPE_TIME64),
                        fm_base_type_get(tsys, FM_TYPE_TIME64));

  auto *m_comp_A =
      fm_module_comp_add(m, "identity", nullptr, 1, module_inputs, nullptr);

  fm_module_comp_t *inps_TL[1] = {m_comp_A};

  auto *m_comp_TL = fm_module_comp_add(
      m, "time_lag", nullptr, 1, inps_TL, time_lag_params_t,
      fmc_time64_from_nanos(500000), fmc_time64_from_nanos(5000));

  fm_module_comp_t *outs[1] = {m_comp_TL};

  fm_module_outs_set(m, 1, outs);

  return m;
}

void timer_test() {
  char *errstring;
  auto *sys = fm_comp_sys_new(&errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  ASSERT_TRUE(fm_comp_sys_std_comp(sys));

  auto *tsys = fm_type_sys_get(sys);

  auto *m = mod_with_delay(tsys);

  ASSERT_NE(m, nullptr);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *split_by_params_t = fm_tuple_type_get(
      tsys, 2, fm_module_type_get(tsys, 1, 1), fm_cstring_type_get(tsys));

  auto *fields_params_t = fm_tuple_type_get(
      tsys, 1,
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_cstring_type_get(tsys), fm_cstring_type_get(tsys)));

  auto *combine_param_t = fm_tuple_type_get(
      tsys, 2,
      fm_tuple_type_get(tsys, 1,
                        fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                                          fm_cstring_type_get(tsys))),
      fm_tuple_type_get(tsys, 0));

  auto *g = fm_comp_graph_get(sys);

  auto *comp_A = fm_comp_decl(
      sys, g, "csv_play", 0, csv_play_param_t, "../test/data/split_by.base.csv",
      "timestamp", fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");
  ASSERT_NE(comp_A, nullptr);

  auto *comp_SB =
      fm_comp_decl(sys, g, "split_by", 1, split_by_params_t, comp_A, m, "text");
  ASSERT_NE(comp_SB, nullptr);

  auto *comp_T = fm_comp_decl(sys, g, "trigger", 1, nullptr, comp_SB);
  ASSERT_NE(comp_T, nullptr);

  auto *comp_SB_no_ts = fm_comp_decl(sys, g, "fields", 1, fields_params_t,
                                     comp_SB, "text", "val1", "val2");
  ASSERT_NE(comp_SB_no_ts, nullptr);

  auto *comp_out = fm_comp_decl(sys, g, "combine", 2, combine_param_t, comp_T,
                                comp_SB_no_ts, "time", "timestamp");
  ASSERT_NE(comp_out, nullptr);

  auto *comp_acc = fm_comp_decl(sys, g, "accumulate", 1, nullptr, comp_A);

  ASSERT_NE(comp_acc, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);

  ASSERT_NE(ctx, nullptr);

  fmc_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fmc_time64_is_end(now));

  auto *result = fm_data_get(fm_result_ref_get(comp_acc));

  ASSERT_NE(result, nullptr);

  auto f_decl = fm_frame_type(result);

  ASSERT_NE(f_decl, nullptr);

  auto val1_field = fm_type_frame_field_idx(f_decl, "val1");
  auto val2_field = fm_type_frame_field_idx(f_decl, "val2");

  int32_t val1_vec[23] = {22, -13, 11, 21, -12, 1, 21, -12, 1,  21,  -12, 1,
                          21, -12, 1,  21, -12, 1, 21, -3,  -4, -12, 1};
  uint16_t val2_vec[23] = {34, 32, 9, 34, 32, 9, 34, 32, 9,  34, 32, 9,
                           34, 32, 9, 34, 32, 9, 34, 24, 11, 32, 9};

  for (int i = 0; i < fm_frame_dim(result, 0); ++i) {
    ASSERT_EQ(*(int32_t *)fm_frame_get_cptr1(result, val1_field, i),
              val1_vec[i]);
    ASSERT_EQ(*(uint16_t *)fm_frame_get_cptr1(result, val2_field, i),
              val2_vec[i]);
  }

  fm_module_del(m);

  fm_comp_sys_del(sys);
}

TEST(split_by, multithread) {
  std::vector<std::thread> vec;

  for (int i = 0; i < 20; ++i) {
    vec.emplace_back(timer_test);
  }
  for (auto &&v : vec)
    v.join();
}
