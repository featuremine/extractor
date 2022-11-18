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
 * @date 10 Aug 2017
 * @brief File contains C++ implementation of the comp graph object
 * @see http://www.featuremine.com
 */

#include "comp_graph.h"

#include "fmc++/mpl.hpp"
#include <algorithm>
#include <set>
#include <string.h>
#include <unordered_map>
#include <vector>

typedef struct {
  fm_comp_node_t *out;
  fm_comp_edge_i next;
} fm_comp_edge_t;

struct fm_comp_graph {
  std::vector<fm_comp_node *> nodes;
  std::vector<fm_comp_edge_t> edges;
  std::unordered_map<std::string, fm_comp_node *> map_;
  std::unordered_map<std::string, unsigned> prefices_;
};

struct fm_comp_node {
  fm_comp_t *obj;
  fm_comp_edge_i edges;
  unsigned index;
  unsigned inputc;
  fm_comp_node_t *inputs[];
};

void fm_comp_graph_edge_add(fm_comp_graph_t *g, fm_comp_node_t *from,
                            fm_comp_node_t *to) {
  int edge_i = g->edges.size();
  g->edges.push_back({to, from->edges});
  from->edges = edge_i;
}

fm_comp_node_t *fm_comp_node_new(fm_comp_t *obj, unsigned inc,
                                 fm_comp_node_t **ins) {
  auto size = sizeof(fm_comp_node_t) + inc * sizeof(fm_comp_node_t *);
  auto *node = (fm_comp_node_t *)malloc(size);
  node->obj = obj;
  node->inputc = inc;
  node->edges = -1;
  if (inc) {
    memcpy(node->inputs, ins, inc * sizeof(fm_comp_node_t *));
  }
  return node;
}

bool fm_comp_node_name_add(fm_comp_graph_t *g, const char *name,
                           fm_comp_node *node) {
  return g->map_.emplace(name, node).second;
}

fm_comp_node *fm_comp_node_name_find(fm_comp_graph_t *g, const char *name) {
  auto where = g->map_.find(name);
  if (where == g->map_.end())
    return nullptr;
  return where->second;
}

char *fm_comp_node_uniq_name_gen(fm_comp_graph_t *g, const char *comp) {
  using namespace std;
  string name = comp;
  name.append(1, '_');
  size_t s = name.size();
  auto &count = g->prefices_[comp];

  while (fm_comp_node_name_find(g, fmc::append_int(name, count).c_str()) !=
         NULL) {
    name.resize(s);
    ++count;
  }
  auto sz = name.size();
  char *name_ptr = (char *)malloc(sz + 1);
  memcpy(name_ptr, name.data(), sz);
  name_ptr[sz] = 0;
  return name_ptr;
}

void fm_comp_node_del(fm_comp_node_t *node) {
  if (node) {
    if (node->obj) {
      fm_comp_del(node->obj);
    }
    free(node);
  }
}

fm_comp_graph_t *fm_comp_graph_new() { return new fm_comp_graph(); }

void fm_comp_graph_del(fm_comp_graph_t *g) {
  for (auto *ptr : g->nodes)
    fm_comp_node_del(ptr);
  delete g;
}

fm_comp_node_t *fm_comp_graph_add(fm_comp_graph_t *g, fm_comp_t *obj,
                                  unsigned inc, fm_comp_node_t **ins) {
  auto *node = fm_comp_node_new(obj, inc, ins);
  for (unsigned i = 0; i < inc; ++i) {
    fm_comp_graph_edge_add(g, ins[i], node);
  }
  node->index = g->nodes.size();
  g->nodes.push_back(node);
  return node;
}

unsigned fm_comp_node_idx(const fm_comp_node_t *node) { return node->index; }

fm_comp_t *fm_comp_node_obj(fm_comp_node_t *node) { return node->obj; }

const fm_comp_t *fm_comp_node_const_obj(const fm_comp_node_t *node) {
  return node->obj;
}

unsigned fm_comp_graph_nodes_size(const fm_comp_graph_t *graph) {
  return graph->nodes.size();
}

unsigned fm_comp_node_inps_size(const fm_comp_node_t *node) {
  return node->inputc;
}

unsigned fm_comp_node_outs_size(const fm_comp_graph_t *g,
                                const fm_comp_node_t *node) {
  unsigned size = 0;
  auto *outp = fm_comp_node_out_cbegin(node);
  for (; !fm_comp_node_out_cend(outp); ++size) {
    outp = fm_comp_node_out_cnext(g, outp);
  }
  return size;
}

fm_comp_node_it fm_comp_graph_nodes_begin(fm_comp_graph_t *graph) {
  return graph->nodes.data();
}

fm_comp_node_it fm_comp_graph_nodes_end(fm_comp_graph_t *graph) {
  return graph->nodes.data() + graph->nodes.size();
}

fm_comp_node_it fm_comp_node_inps_begin(fm_comp_node_t *node) {
  return node->inputs;
}

fm_comp_node_it fm_comp_node_inps_end(fm_comp_node_t *node) {
  return node->inputs + node->inputc;
}

fm_comp_node_const_it fm_comp_graph_nodes_cbegin(const fm_comp_graph_t *graph) {
  return graph->nodes.data();
}

fm_comp_node_const_it fm_comp_graph_nodes_cend(const fm_comp_graph_t *graph) {
  return graph->nodes.data() + graph->nodes.size();
}

fm_comp_node_const_it fm_comp_node_inps_cbegin(const fm_comp_node_t *node) {
  return node->inputs;
}

fm_comp_node_const_it fm_comp_node_inps_cend(const fm_comp_node_t *node) {
  return node->inputs + node->inputc;
}

typedef const fm_comp_edge_i *fm_comp_edge_const_it;
typedef fm_comp_edge_i *fm_comp_edge_it;

fm_comp_edge_const_it fm_comp_node_out_cbegin(const fm_comp_node_t *n) {
  return &n->edges;
}

bool fm_comp_node_out_cend(fm_comp_edge_const_it it) { return *it == -1; }

fm_comp_edge_const_it fm_comp_node_out_cnext(const fm_comp_graph_t *g,
                                             fm_comp_edge_const_it it) {
  return &(g->edges[*it].next);
}

const fm_comp_node_t *fm_comp_node_out_cnode(const fm_comp_graph_t *g,
                                             fm_comp_edge_const_it it) {
  return g->edges[*it].out;
}

fm_comp_edge_it fm_comp_node_out_begin(fm_comp_node_t *n) { return &n->edges; }

bool fm_comp_node_out_end(fm_comp_edge_it it) { return *it == -1; }

fm_comp_edge_it fm_comp_node_out_next(fm_comp_graph_t *g, fm_comp_edge_it it) {
  return &(g->edges[*it].next);
}

fm_comp_node_t *fm_comp_node_out_node(fm_comp_graph_t *g, fm_comp_edge_it it) {
  return g->edges[*it].out;
}

/*
 * @brief returns a linear predecessor of a node
 *
 * A linear predecessor is a single input to a given
 * node that points only points to that node.
 */
fm_comp_node_t *get_linear_predecessor(fm_comp_graph_t *g, fm_comp_node_t *n) {
  if (n->inputc != 1 || g->edges[n->inputs[0]->edges].next != -1)
    return n;
  return n->inputs[0];
}

void node_swap(fm_comp_graph_t *g, unsigned i, unsigned j) {
  auto &nodes = g->nodes;
  auto *ni = nodes[i];
  nodes[i] = nodes[j];
  nodes[j] = ni;
  nodes[i]->index = i;
  nodes[j]->index = j;
}

unsigned make_nodes_sorted(fm_comp_graph_t *g, fm_comp_node_t *n, unsigned s) {
  do {
    node_swap(g, n->index, s++);
    n = get_linear_predecessor(g, n);
  } while (n->index + 1 != s);
  return s;
}

bool term_node_check(fm_comp_graph_t *g, fm_comp_node_t *n, unsigned sorted) {
  fm_comp_edge_i *edge_p = &n->edges;
  for (; *edge_p != -1; edge_p = &(g->edges[*edge_p].next)) {
    if (g->edges[*edge_p].out->index >= sorted) {
      return false;
    }
  }
  return true;
}

void add_term_nodes(fm_comp_graph_t *g, unsigned sorted,
                    std::vector<fm_comp_node_t *> &term) {
  auto *last = g->nodes[sorted - 1];
  for (unsigned i = 0; i < last->inputc; ++i) {
    auto *candidate = last->inputs[i];
    if (term_node_check(g, candidate, sorted)) {
      term.push_back(candidate);
    }
  }
}

unsigned fm_comp_graph_term(fm_comp_graph_t *g, fm_comp_node_t **nodes) {
  unsigned count = 0;
  for (auto *node : g->nodes) {
    if (node->edges == -1) {
      nodes[count++] = node;
    }
  }
  return count;
}

bool fm_comp_graph_stable_top_sort(fm_comp_graph_t *g) {
  unsigned term_s = 0;
  std::vector<fm_comp_node_t *> term_n(g->nodes.size());
  term_s = fm_comp_graph_term(g, term_n.data());
  return g->nodes.size() ==
         fm_comp_subgraph_stable_top_sort(g, term_s, term_n.data());
}

unsigned fm_comp_subgraph_stable_top_sort(fm_comp_graph_t *g, unsigned count,
                                          fm_comp_node_t **nodes) {
  unsigned sorted = 0;

  std::vector<fm_comp_node_t *> term_n;
  term_n.reserve(g->nodes.size());

  for (unsigned i = 0; i < count; ++i) {
    term_n.push_back(nodes[i]);
  }

  while (!term_n.empty()) {
    if (term_n.back()->index < sorted) {
      term_n.pop_back();
    } else {
      sorted = make_nodes_sorted(g, term_n.back(), sorted);
      term_n.pop_back();
      add_term_nodes(g, sorted, term_n);
    }
  }

  return sorted;
}

unsigned fm_comp_graph_indep(const fm_comp_graph_t *g,
                             const fm_comp_node_t **nodes) {
  unsigned count = 0;
  for (auto *node : g->nodes) {
    if (node->inputc == 0) {
      nodes[count++] = node;
    }
  }
  return count;
}

unsigned fm_comp_graph_dep_sort(const fm_comp_graph_t *g, unsigned count,
                                const fm_comp_node_t **nodes) {
  std::set<unsigned> sorted;
  for (unsigned i = 0; i < count; ++i) {
    sorted.insert(nodes[i]->index);
  }

  unsigned first = 0;
  unsigned last = count;
  while (first < last) {
    // go through all child nodes and see if they are terminal
    std::vector<const fm_comp_node_t *> outps(fm_comp_graph_nodes_size(g));
    unsigned outpc = 0;
    auto it = fm_comp_node_out_cbegin(nodes[first++]);
    for (; !fm_comp_node_out_cend(it); it = fm_comp_node_out_cnext(g, it)) {
      outps[outpc++] = fm_comp_node_out_cnode(g, it);
    }
    while (outpc > 0) {
      auto *node = outps[--outpc];
      bool terminal = true;
      for (unsigned i = 0; i < node->inputc; ++i) {
        auto inp_idx = node->inputs[i]->index;
        if (sorted.find(inp_idx) == sorted.end()) {
          terminal = false;
          break;
        }
      }
      // if node is terminal and not sorted yet add to the array
      if (terminal && sorted.insert(node->index).second) {
        nodes[last++] = node;
      }
    }
  }
  return first;
}
