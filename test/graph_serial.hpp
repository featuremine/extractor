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
 * @file graph_serial.hpp
 * @author Maxim Trokhimtchouk
 * @date 10 Jul 2018
 * @brief File contains tests for split operator
 *
 * Replays the content of messagepack encoded file
 * Validates the context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/comp_sys.h"
#include "extractor/frame.h"
#include "comp_graph.h"
#include "extractor/std_comp.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_sys.h"
}

#include "test_util.hpp"
#include "fmc++/gtestwrap.hpp"
#include "fmc/platform.h"
#include <iostream>

using namespace fmc;

TEST(graph_serial, dep_sort) {
  using namespace std;

  string testout = "mp_play_0\n"
                   "split_0\n"
                   "identity_0\n"
                   "identity_1\n"
                   "identity_2\n";

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

  auto cstring_t = fm_cstring_type_get(tsys);
  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, cstring_t, fm_type_type_get(tsys), cstring_t);

  auto *mp_play_param_t = fm_tuple_type_get(
      tsys, 2, cstring_t,
      fm_tuple_type_get(tsys, 7, row_desc_t, row_desc_t, row_desc_t, row_desc_t,
                        row_desc_t, row_desc_t, row_desc_t));

  auto *chararray16 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16);
  auto *chararray32 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 32);

  auto *comp_A =
      fm_comp_decl(sys, g, "mp_play", 0, mp_play_param_t,
                   (src_dir + "/data/sip_quotes_20171018.mp").c_str(),
                   "receive", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "ticker", chararray16, "", "market", chararray32, "",
                   "bidprice", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "askprice", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "bidqty", fm_base_type_get(tsys, FM_TYPE_INT32), "",
                   "askqty", fm_base_type_get(tsys, FM_TYPE_INT32), "");

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

  auto g_size = fm_comp_graph_nodes_size(g);
  vector<const fm_comp_node_t *> sorted(g_size);
  auto indep_c = fm_comp_graph_indep(g, sorted.data());
  auto sorted_c = fm_comp_graph_dep_sort(g, indep_c, sorted.data());
  ASSERT_EQ(sorted_c, g_size);

  ostringstream ss;
  for (unsigned i = 0; i < g_size; ++i) {
    auto *comp = fm_comp_node_const_obj(sorted[i]);
    ss << fm_comp_name(comp) << endl;
  }
  ASSERT_EQ(ss.str(), testout);

  fm_comp_sys_del(sys);
}

typedef bool (*fm_reader)(void *data, size_t limit, void *closure);

// @todo need to test for failure cases

TEST(graph_serial, serialize) {
  using namespace std;

  string testout = "mp_play_0\n"
                   "split_0\n"
                   "identity_0\n"
                   "identity_1\n"
                   "identity_2\n";

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

  auto cstring_t = fm_cstring_type_get(tsys);
  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, cstring_t, fm_type_type_get(tsys), cstring_t);

  auto *mp_play_param_t = fm_tuple_type_get(
      tsys, 2, cstring_t,
      fm_tuple_type_get(tsys, 7, row_desc_t, row_desc_t, row_desc_t, row_desc_t,
                        row_desc_t, row_desc_t, row_desc_t));

  auto *chararray16 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16);
  auto *chararray32 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 32);

  auto *comp_A =
      fm_comp_decl(sys, g, "mp_play", 0, mp_play_param_t,
                   (src_dir + "/data/sip_quotes_20171018.mp").c_str(),
                   "receive", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "ticker", chararray16, "", "market", chararray32, "",
                   "bidprice", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "askprice", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "bidqty", fm_base_type_get(tsys, FM_TYPE_INT32), "",
                   "askqty", fm_base_type_get(tsys, FM_TYPE_INT32), "");

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

  string buf;
  ASSERT_TRUE(fm_comp_graph_write(g, string_writer, &buf));

  cout << buf << endl;

  string_view view = buf;
  auto *g_2 = fm_comp_graph_read(sys, string_view_reader, &view);
  ASSERT_NE(g_2, nullptr);

  string buf_2;
  ASSERT_TRUE(fm_comp_graph_write(g_2, string_writer, &buf_2));
  cout << buf_2 << endl;
  ASSERT_EQ(buf, buf_2);

  fm_comp_sys_del(sys);
}
