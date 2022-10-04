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
 * @file module_serial.hpp
 * @author Andres Rangel
 * @date 11 Oct 2018
 * @brief File contains tests for module serialization
 *
 * @see http://www.featuremine.com
 */
extern "C" {
#include "extractor/comp_sys.h"
#include "extractor/module.h"
#include "extractor/std_comp.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_sys.h"
}

#include "test_util.hpp"
#include <fmc++/gtestwrap.hpp>
#include <iostream>

TEST(module_serial, serialize) {
  char *errstring;
  auto *sys = fm_comp_sys_new((src_dir + "/test.lic").c_str(), &errstring);
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

  string buf;
  ASSERT_TRUE(fm_module_write(m, string_writer, &buf));
  cout << buf << endl;

  string_view view = buf;
  auto *m_2 = fm_module_read(sys, string_view_reader, &view);
  cout << fm_comp_sys_error_msg(sys) << endl;
  ASSERT_NE(m_2, nullptr);

  string buf_2;
  ASSERT_TRUE(fm_module_write(m_2, string_writer, &buf_2));
  cout << buf_2 << endl;
  ASSERT_EQ(buf, buf_2);

  fm_comp_sys_del(sys);
}
