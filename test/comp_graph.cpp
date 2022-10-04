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
 * @file comp_graph.cpp
 * @author Maxim Trokhimtchouk
 * @date 8 Aug 2017
 * @brief File contains tests for computational graph object
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "comp_graph.h"
}

#include <fmc++/gtestwrap.hpp>

#include <string>

struct fm_comp {
  fm_comp(const std::string &n) : name(n) {}
  std::string name;
};

void fm_comp_del(fm_comp_t *obj) { delete obj; }

TEST(comp_graph, sort) {
  using namespace std;
  auto *g = fm_comp_graph_new();

  fm_comp_node_t *inps_B[1];
  fm_comp_node_t *inps_C[1];
  fm_comp_node_t *inps_D[1];
  fm_comp_node_t *inps_E[1];
  fm_comp_node_t *inps_F[1];
  fm_comp_node_t *inps_G[3];

  auto *node_A = fm_comp_graph_add(g, new fm_comp("A"), 0, NULL);

  inps_B[0] = node_A;
  auto *node_B = fm_comp_graph_add(g, new fm_comp("B"), 1, inps_B);

  inps_C[0] = node_A;
  auto *node_C = fm_comp_graph_add(g, new fm_comp("C"), 1, inps_C);

  inps_D[0] = node_C;
  auto *node_D = fm_comp_graph_add(g, new fm_comp("D"), 1, inps_D);

  inps_E[0] = node_B;
  auto *node_E = fm_comp_graph_add(g, new fm_comp("E"), 1, inps_E);

  inps_F[0] = node_B;
  auto *node_F = fm_comp_graph_add(g, new fm_comp("F"), 1, inps_F);

  inps_G[0] = node_D;
  inps_G[1] = node_E;
  inps_G[2] = node_F;
  fm_comp_graph_add(g, new fm_comp("G"), 3, inps_G);

  fm_comp_graph_stable_top_sort(g);

  string str_rep;
  auto it = fm_comp_graph_nodes_begin(g);
  auto end = fm_comp_graph_nodes_end(g);
  for (; it != end; ++it) {
    str_rep += fm_comp_node_obj(*it)->name;
  }

  ASSERT_EQ(str_rep, "GFEBDCA");

  fm_comp_graph_del(g);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
