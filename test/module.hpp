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
 * @file module.hpp
 * @author Maxim Trokhimtchouk
 * @date 12 Sep 2018
 * @brief Module test
 *
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

using namespace fmc;

TEST(module, create) {
  auto *m = fm_module_new("test_module", 0, nullptr);

  ASSERT_NE(m, nullptr);

  fm_module_del(m);
}

TEST(module, add_comps) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  auto *m = fm_module_new("test_module", 0, nullptr);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *m_comp_A = fm_module_comp_add(
      m, "csv_play", nullptr, 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_A, nullptr);

  fm_module_del(m);
}

TEST(module, add_multi_comps) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  auto *m = fm_module_new("test_module", 0, nullptr);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *m_comp_A = fm_module_comp_add(
      m, "csv_play", nullptr, 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_A, nullptr);

  auto *m_comp_B = fm_module_comp_add(
      m, "csv_play", nullptr, 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_B, nullptr);

  fm_module_del(m);
}

TEST(module, add_repeated_comps) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  auto *m = fm_module_new("test_module", 0, nullptr);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *m_comp_A = fm_module_comp_add(
      m, "csv_play", "csv_play_0", 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_A, nullptr);

  auto *m_comp_B = fm_module_comp_add(
      m, "csv_play", "csv_play_0", 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_EQ(m_comp_B, nullptr);

  fm_module_del(m);
}

TEST(module, run_comps) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  ASSERT_TRUE(fm_comp_sys_std_comp(sys));

  auto *m = fm_module_new("test_module", 0, nullptr);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *csv_record_param_t = fm_cstring_type_get(tsys);

  auto *m_comp_A = fm_module_comp_add(
      m, "csv_play", nullptr, 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_A, nullptr);

  fm_module_comp_t *m_inputs_B[1];
  m_inputs_B[0] = m_comp_A;

  auto *m_comp_B = fm_module_comp_add(
      m, "csv_record", nullptr, 1, m_inputs_B, csv_record_param_t,
      (src_dir + "/data/module.test.csv").c_str());

  ASSERT_NE(m_comp_B, nullptr);

  auto *g = fm_comp_graph_get(sys);

  ASSERT_TRUE(fm_module_inst(sys, g, m, nullptr, nullptr));

  auto *ctx = fm_stream_ctx_get(sys, g);

  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_module_del(m);

  fm_comp_sys_del(sys);

  ASSERT_BASE((src_dir + "/data/module.base.csv").c_str(),
              (src_dir + "/data/module.test.csv").c_str());
}

TEST(module, comp_no_params) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  ASSERT_TRUE(fm_comp_sys_std_comp(sys));

  auto *m = fm_module_new("test_module", 0, nullptr);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *csv_record_param_t = fm_cstring_type_get(tsys);

  auto *m_comp_A = fm_module_comp_add(
      m, "csv_play", nullptr, 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_A, nullptr);

  fm_module_comp_t *m_inputs_AB[1];
  m_inputs_AB[0] = m_comp_A;

  auto *m_comp_AB =
      fm_module_comp_add(m, "identity", nullptr, 1, m_inputs_AB, nullptr);

  fm_module_comp_t *m_inputs_B[1];
  m_inputs_B[0] = m_comp_AB;

  auto *m_comp_B = fm_module_comp_add(
      m, "csv_record", nullptr, 1, m_inputs_B, csv_record_param_t,
      (src_dir + "/data/module.test.csv").c_str());

  ASSERT_NE(m_comp_B, nullptr);

  auto *g = fm_comp_graph_get(sys);

  ASSERT_TRUE(fm_module_inst(sys, g, m, nullptr, nullptr));

  auto *ctx = fm_stream_ctx_get(sys, g);

  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_module_del(m);

  fm_comp_sys_del(sys);

  ASSERT_BASE((src_dir + "/data/module.base.csv").c_str(),
              (src_dir + "/data/module.test.csv").c_str());
}

TEST(module, multi_inst) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  ASSERT_TRUE(fm_comp_sys_std_comp(sys));

  auto *m = fm_module_new("test_module", 0, nullptr);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *m_comp_A = fm_module_comp_add(
      m, "csv_play", nullptr, 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_A, nullptr);

  auto *g = fm_comp_graph_get(sys);

  ASSERT_TRUE(fm_module_inst(sys, g, m, nullptr, nullptr));

  ASSERT_TRUE(fm_module_inst(sys, g, m, nullptr, nullptr));

  auto *ctx = fm_stream_ctx_get(sys, g);

  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_module_del(m);

  fm_comp_sys_del(sys);

  ASSERT_BASE((src_dir + "/data/module.base.csv").c_str(),
              (src_dir + "/data/module.test.csv").c_str());
}

TEST(module, failed_multi_equal_comp_name) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  ASSERT_TRUE(fm_comp_sys_std_comp(sys));

  auto *m = fm_module_new("test_module", 0, nullptr);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *m_comp_A = fm_module_comp_add(
      m, "csv_play", "csv_play_0", 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_A, nullptr);

  auto *g = fm_comp_graph_get(sys);

  ASSERT_TRUE(fm_module_inst(sys, g, m, nullptr, nullptr));

  ASSERT_FALSE(fm_module_inst(sys, g, m, nullptr, nullptr));
}

TEST(module, multi_module) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  ASSERT_TRUE(fm_comp_sys_std_comp(sys));

  auto *m = fm_module_new("test_module", 0, nullptr);

  auto *m_2 = fm_module_new("test_module_two", 0, nullptr);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *csv_record_param_t = fm_cstring_type_get(tsys);

  auto *m_comp_A = fm_module_comp_add(
      m, "csv_play", nullptr, 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_A, nullptr);

  fm_module_comp_t *m_inputs_B[1];
  m_inputs_B[0] = m_comp_A;

  auto *m_comp_B = fm_module_comp_add(
      m, "csv_record", nullptr, 1, m_inputs_B, csv_record_param_t,
      (src_dir + "/data/multi_module.test.csv").c_str());

  ASSERT_NE(m_comp_B, nullptr);

  auto *m_comp_C = fm_module_comp_add(
      m_2, "csv_play", nullptr, 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_C, nullptr);

  fm_module_comp_t *m_inputs_D[1];
  m_inputs_D[0] = m_comp_C;

  auto *m_comp_D = fm_module_comp_add(
      m_2, "csv_record", nullptr, 1, m_inputs_D, csv_record_param_t,
      (src_dir + "/data/multi_module_two.test.csv").c_str());

  ASSERT_NE(m_comp_D, nullptr);

  auto *g = fm_comp_graph_get(sys);

  ASSERT_TRUE(fm_module_inst(sys, g, m, nullptr, nullptr));

  ASSERT_TRUE(fm_module_inst(sys, g, m_2, nullptr, nullptr));

  auto *ctx = fm_stream_ctx_get(sys, g);

  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_module_del(m);

  fm_comp_sys_del(sys);

  ASSERT_BASE((src_dir + "/data/module.base.csv").c_str(),
              (src_dir + "/data/"
                         "multi_module.test.csv")
                  .c_str());

  ASSERT_BASE((src_dir + "/data/module.base.csv").c_str(),
              (src_dir + "/data/"
                         "multi_module_two.test.csv")
                  .c_str());
}

TEST(module, failed_multi_equal_module_name) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  ASSERT_TRUE(fm_comp_sys_std_comp(sys));

  auto *m = fm_module_new("test_module", 0, nullptr);

  auto *m_2 = fm_module_new("test_module", 0, nullptr);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *csv_record_param_t = fm_cstring_type_get(tsys);

  auto *m_comp_A = fm_module_comp_add(
      m, "csv_play", nullptr, 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_A, nullptr);

  fm_module_comp_t *m_inputs_B[1];
  m_inputs_B[0] = m_comp_A;

  auto *m_comp_B = fm_module_comp_add(
      m, "csv_record", nullptr, 1, m_inputs_B, csv_record_param_t,
      (src_dir + "/data/module.test.csv").c_str());

  ASSERT_NE(m_comp_B, nullptr);

  auto *g = fm_comp_graph_get(sys);

  ASSERT_TRUE(fm_module_inst(sys, g, m, nullptr, nullptr));

  ASSERT_FALSE(fm_module_inst(sys, g, m_2, nullptr, nullptr));
}

TEST(module, run_comps_with_input) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  ASSERT_TRUE(fm_comp_sys_std_comp(sys));

  fm_module_comp_t *module_inputs[1];

  auto *m = fm_module_new("test_module", 1, module_inputs);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *csv_record_param_t = fm_cstring_type_get(tsys);

  auto *g = fm_comp_graph_get(sys);

  auto *comp_A = fm_comp_decl(
      sys, g, "csv_play", 0, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");
  ASSERT_NE(comp_A, nullptr);

  fm_module_comp_t *m_inputs_B[1];
  m_inputs_B[0] = module_inputs[0];

  auto *m_comp_B = fm_module_comp_add(
      m, "csv_record", nullptr, 1, m_inputs_B, csv_record_param_t,
      (src_dir + "/data/module_inputs.test.csv").c_str());

  ASSERT_NE(m_comp_B, nullptr);

  fm_comp_t *inst_inputs[1];
  inst_inputs[0] = comp_A;

  ASSERT_TRUE(fm_module_inst(sys, g, m, inst_inputs, nullptr));

  auto *ctx = fm_stream_ctx_get(sys, g);

  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_module_del(m);

  fm_comp_sys_del(sys);

  ASSERT_BASE((src_dir + "/data/module.base.csv").c_str(),
              (src_dir + "/data/"
                         "module_inputs.test.csv")
                  .c_str());
}

TEST(module, run_comps_with_output) {
  char *errstring;
  auto *sys = fm_comp_sys_new(std::getenv("LICENSE_PATH"), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  ASSERT_TRUE(fm_comp_sys_std_comp(sys));

  auto *m = fm_module_new("test_module", 0, nullptr);

  ASSERT_NE(m, nullptr);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t =
      fm_tuple_type_get(tsys, 2, fm_cstring_type_get(tsys),
                        fm_tuple_type_get(tsys, 4, row_desc_t, row_desc_t,
                                          row_desc_t, row_desc_t));

  auto *csv_record_param_t = fm_cstring_type_get(tsys);

  auto *m_comp_A = fm_module_comp_add(
      m, "csv_play", nullptr, 0, nullptr, csv_play_param_t,
      (src_dir + "/data/csv_play_file.csv").c_str(), "timestamp",
      fm_base_type_get(tsys, FM_TYPE_TIME64), "", "text",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 4), "",
      "val1", fm_base_type_get(tsys, FM_TYPE_INT32), "", "val2",
      fm_base_type_get(tsys, FM_TYPE_UINT16), "");

  ASSERT_NE(m_comp_A, nullptr);

  fm_module_comp_t *module_outputs[1];
  module_outputs[0] = m_comp_A;

  auto *g = fm_comp_graph_get(sys);

  ASSERT_TRUE(fm_module_outs_set(m, 1, module_outputs));

  fm_comp_t *module_comp_outputs[1];

  ASSERT_TRUE(fm_module_inst(sys, g, m, nullptr, module_comp_outputs));

  auto *comp_B = fm_comp_decl(
      sys, g, "csv_record", 1, csv_record_param_t, module_comp_outputs[0],
      (src_dir + "/data/module_outputs.test.csv").c_str());
  ASSERT_NE(comp_B, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fm_module_del(m);

  fm_comp_sys_del(sys);

  ASSERT_BASE((src_dir + "/data/module.base.csv").c_str(),
              (src_dir + "/data/"
                         "module_outputs.test.csv")
                  .c_str());
}
