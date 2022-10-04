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
 * @file comp_sys.cpp
 * @author Maxim Trokhimtchouk
 * @date 1 Aug 2017
 * @brief File contains C++ implementation of the computational system
 *
 * This file contains implementation of the computational system
 * @see http://www.featuremine.com
 */

extern "C" {
#include "arg_stack.h"
#include "comp_sys.h"
#include "frame.h"
#include "src/comp.h"
#include "src/comp_graph.h"
#include "src/frame_serial.h"
#include "src/stream_ctx.h"
#include "time64.h"
#include "type_sys.h"
#include <fmc/cmp/cmp.h>
}

#include "src/comp_sys.hpp"
#include "src/mp_util.hpp"
#include "src/serial_util.hpp"
#include <functional>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

unsigned fm_comp_graph_stable_sort(const fm_comp_graph_t *g,
                                   const fm_comp_node_t **nodes) {
  auto indep_c = fm_comp_graph_indep(g, nodes);
  sort(nodes, nodes + indep_c, [](auto *a, auto *b) -> unsigned {
    auto *a_str = fm_comp_name(fm_comp_node_const_obj(a));
    auto *b_str = fm_comp_name(fm_comp_node_const_obj(b));
    return strcmp(a_str, b_str);
  });
  return fm_comp_graph_dep_sort(g, indep_c, nodes);
}

bool fm_comp_graph_node_write(const fm_comp_t *comp, fm_writer writer,
                              void *closure) {
  auto *node = fm_comp_node_cptr(comp);
  if (!(write_string(fm_comp_name(comp), writer, closure) &&
        write_string(fm_comp_get_def(comp)->name, writer, closure)))
    return false;

  if (!write_number(fm_comp_node_inps_size(node), writer, closure))
    return false;

  auto it = fm_comp_node_inps_cbegin(node);
  for (; it != fm_comp_node_inps_cend(node); ++it) {
    if (!write_string(fm_comp_name(fm_comp_node_const_obj(*it)), writer,
                      closure))
      return false;
  }

  auto *arg_buf = fm_comp_arg_buffer(comp);

  if (arg_buf) {
    if (!fm_arg_write(arg_buf, writer, closure))
      return false;
  } else if (!write_char('\n', writer, closure))
    return false;

  if (fm_comp_clbck_has(comp)) {
    if (!write_string("Y", writer, closure))
      return false;
  } else if (!write_string("N", writer, closure))
    return false;

  if (fm_comp_data_required(comp))
    return write_string("Y", writer, closure);
  else
    return write_string("N", writer, closure);
}

bool fm_comp_graph_write(const fm_comp_graph *g, fm_writer writer,
                         void *closure) {
  auto g_size = fm_comp_graph_nodes_size(g);
  vector<const fm_comp_node_t *> sorted(g_size);
  auto sorted_c = fm_comp_graph_stable_sort(g, sorted.data());
  if (sorted_c != g_size) {
    return false;
  }
  if (!write_number(g_size, writer, closure)) {
    return false;
  }
  for (unsigned i = 0; i < g_size; ++i) {
    auto *comp = fm_comp_node_const_obj(sorted[i]);

    if (!fm_comp_graph_node_write(comp, writer, closure)) {
      return false;
    }
  }
  return true;
}

/**
 * @todo need to add proper error reporting
 */
fm_comp_graph *fm_comp_graph_read(fm_comp_sys_t *sys, fm_reader reader,
                                  void *closure) {
  string buf;
  unsigned g_size;
  if (!fm_item_read(buf, g_size, reader, closure) || g_size < 0) {
    fm_comp_sys_error_set(sys, "[ERROR]\t(comp_sys) malformed graph "
                               "serialization; failed to read graph size");
    return nullptr;
  }
  auto *g = fm_comp_graph_new();
  auto error = [g, sys](const char *str) {
    fm_comp_sys_error_set(
        sys, "[ERROR]\t(comp_sys) malformed graph serialization; %s", str);
    fm_comp_graph_del(g);
    return nullptr;
  };
  auto same_error = [g]() {
    fm_comp_graph_del(g);
    return nullptr;
  };
  auto *tsys = fm_type_sys_get(sys);
  for (unsigned i = 0; i < g_size; ++i) {
    auto name = read_str(reader, closure);
    auto comp_name = read_str(reader, closure);
    if (name.empty()) {
      return error("could not read computation name");
    }
    if (comp_name.empty()) {
      return error("could not read computation type");
    }
    unsigned inpc;
    if (!read_unsigned(inpc, reader, closure)) {
      return error("could not read number of inputs");
    }
    vector<fm_comp_t *> inps(inpc);
    for (unsigned i = 0; i < inpc; ++i) {
      auto inp_name = read_str(reader, closure);
      auto *node = fm_comp_node_name_find(g, inp_name.c_str());
      if (!node) {
        return error("could not find input operator");
      }
      inps[i] = fm_comp_node_obj(node);
    }
    fm_type_decl_cp td = nullptr;
    fm_arg_stack_t *args = nullptr;
    fm_arg_buffer_t *arg_buf = fm_arg_read(tsys, &td, &args, reader, closure);
    if (!args) {
      return error("could not parse operator parameters");
    }

    // has callbacks
    read_str(reader, closure);
    // data required
    read_str(reader, closure);

    auto *comp = fm_comp_decl4(sys, g, comp_name.c_str(), name.c_str(), inpc,
                               inps.data(), td, fm_arg_stack_args(args));
    fm_arg_stack_free(args);
    if (arg_buf)
      fm_arg_buffer_del(arg_buf);
    if (!comp) {
      return same_error();
    }
  }
  sys->graphs.push_back(g);
  return g;
}
