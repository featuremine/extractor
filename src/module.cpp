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
 * @file module.h
 * @author Andres Rangel
 * @date 12 Sep 2018
 * @brief File contains C definitions of the module object
 *
 * This file contains declarations of the module object
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/module.h"
#include "extractor/arg_stack.h"
#include "arg_serial.h"
#include "comp_graph.h"
#include "comp_sys_module.h"
}

#include "serial_util.hpp"
#include "fmc++/mpl.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

struct fm_module {
  string name_;
  vector<fm_module_comp_t *> nodes_;
  vector<fm_module_comp_t *> inputs_;
  vector<fm_module_comp_t *> outputs_;
  unordered_map<string, fm_module_comp_t *> map_;
  unordered_map<string, unsigned> prefices_;
};

struct fm_module_comp {
  string comp_;
  string name_;
  fm_arg_buffer_t *params_;
  fm_module_t *owner_;
  vector<fm_module_comp_t *> inputs_;
};

char *fm_module_comp_uniq_name_gen(fm_module_t *m, const char *comp) {
  using namespace std;
  string name = comp;
  name.append(1, '_');
  size_t s = name.size();
  auto &count = m->prefices_[comp];

  while (m->map_.find(fmc::append_int(name, count)) != m->map_.end()) {
    name.resize(s);
    ++count;
  }

  auto sz = name.size();
  char *name_ptr = (char *)malloc(sz + 1);
  memcpy(name_ptr, name.data(), sz);
  name_ptr[sz] = 0;
  return name_ptr;
}

// Creates new module
fm_module_t *fm_module_new(const char *name, unsigned nargs,
                           fm_module_comp_t **inputs) {
  auto *m = new fm_module();

  if (name)
    m->name_ = name;

  for (unsigned i = 0; i < nargs; ++i) {
    string inputstr = "input_";
    auto *input_comp = new fm_module_comp();
    input_comp->owner_ = m;
    input_comp->comp_ = "input";
    input_comp->name_ = fmc::append_int(inputstr, i);
    m->inputs_.push_back(input_comp);
    inputs[i] = input_comp;
  }

  return m;
}

// Add comp to module
fm_module_comp_t *fm_module_comp_add(fm_module_t *m, const char *comp,
                                     const char *name, unsigned nargs,
                                     fm_module_comp_t **inputs,
                                     fm_type_decl_cp type, ...) {
  fm_module_comp_t *inst = nullptr;
  va_list args;
  va_start(args, type);
  STACK(4096, stack);
  auto res = fm_arg_stack_build(type, STACK_FWD(stack), &args);
  if (res == 0) {
    inst = fm_module_comp_add1(m, comp, name, nargs, inputs, type,
                               STACK_ARGS(stack));
  }
  va_end(args);
  return inst;
}

// Add comp to module
fm_module_comp_t *fm_module_comp_add1(fm_module_t *m, const char *comp,
                                      const char *name, unsigned nargs,
                                      fm_module_comp_t **inputs,
                                      fm_type_decl_cp type,
                                      fm_arg_stack_t params) {
  if (name && m->map_.find(name) != m->map_.end()) {
    return nullptr;
  }

  for (unsigned i = 0; i < nargs; ++i) {
    if (inputs[i]->owner_ != m)
      return nullptr;
  }

  string comp_name;
  if (name)
    comp_name = name;

  auto *m_comp = new fm_module_comp();
  m_comp->comp_ = comp;
  m_comp->name_ = comp_name;
  m_comp->owner_ = m;
  m_comp->params_ = fm_arg_buffer_new(type, params);

  for (unsigned i = 0; i < nargs; ++i)
    m_comp->inputs_.push_back(inputs[i]);

  m->nodes_.push_back(m_comp);

  if (name)
    m->map_.emplace(name, m_comp);

  return m_comp;
}

// Instanciates the module
bool fm_module_inst(fm_comp_sys_t *sys, fm_comp_graph_t *g, fm_module_t *m,
                    fm_comp_t **inputs, fm_comp_t **outputs) {
  if (m->name_.empty()) {
    char *module_name = fm_module_uniq_name_gen(sys);
    m->name_.append(module_name);
    free(module_name);
  }

  auto *found = fm_module_name_find(sys, m->name_.c_str());
  if (found != nullptr && found != m) {
    fm_comp_sys_error_set(sys,
                          "[ERROR]\t(comp_sys) module name \"%s\" is"
                          " already registered in sys",
                          m->name_.c_str());
    return false;
  }

  if (found == nullptr && !fm_module_name_add(sys, m->name_.c_str(), m)) {
    fm_comp_sys_error_set(sys,
                          "[ERROR]\t(comp_sys) could not add module "
                          "name \"%s\" to sys",
                          m->name_.c_str());
    return false;
  }

  unordered_map<fm_module_comp_t *, fm_comp_t *> node_map;
  auto *tsys = fm_type_sys_get(sys);
  for (unsigned i = 0; i < fm_module_inps_size(m); ++i) {
    node_map[m->inputs_[i]] = inputs[i];
  }

  for (auto node : m->nodes_) {
    auto nargs = node->inputs_.size();
    vector<fm_comp_t *> c_inputs(nargs);

    for (unsigned i = 0; i < nargs; ++i) {
      c_inputs[i] = node_map[node->inputs_[i]];
    }

    string comp_name(m->name_);
    comp_name.append("/");

    if (!node->name_.empty())
      comp_name.append(node->name_);
    else {
      auto *dat = fm_module_comp_uniq_name_gen(m, node->comp_.c_str());
      comp_name.append(dat);
      m->map_.emplace(dat, node);
      free(dat);
    }

    fm_comp_t *comp = nullptr;

    if (!node->params_) {
      fm_arg_stack_t t = {{0, nullptr}};
      comp = fm_comp_decl4(sys, g, node->comp_.c_str(), comp_name.c_str(),
                           nargs, c_inputs.data(), nullptr, t);
    } else {
      fm_type_decl_cp node_type = nullptr;
      fm_arg_stack_t *params = nullptr;

      fm_arg_buffer_t *buff =
          fm_arg_stack_from_buffer(tsys, node->params_, &node_type, &params);

      if (!buff) {
        fm_comp_sys_error_set(sys,
                              "[ERROR]\t(comp_sys) could not recreate "
                              "the arguments for comp %s from module "
                              "%s",
                              node->comp_.c_str(), m->name_.c_str());
        return false;
      }
      comp =
          fm_comp_decl4(sys, g, node->comp_.c_str(), comp_name.c_str(), nargs,
                        c_inputs.data(), node_type, fm_arg_stack_args(params));
      fm_arg_stack_free(params);
      fm_arg_buffer_del(buff);
    }

    if (!comp) {
      // we dont set the error here so we dont override the error that
      // happened trying to generate the comp
      return false;
    }

    node_map[node] = comp;
    auto pos = find(m->outputs_.begin(), m->outputs_.end(), node);
    if (pos != m->outputs_.end()) {
      outputs[pos - m->outputs_.begin()] = comp;
    }
  }

  return true;
}

// Returns number of inputs
unsigned fm_module_inps_size(fm_module_t *m) { return m->inputs_.size(); }

// Returns number of outputs
unsigned fm_module_outs_size(fm_module_t *m) { return m->outputs_.size(); }

// Set outputs of module
bool fm_module_outs_set(fm_module_t *m, unsigned nargs,
                        fm_module_comp_t **comps) {

  for (unsigned i = 0; i < nargs; ++i) {
    if (comps[i]->owner_ != m)
      return false;
  }

  auto &v = m->outputs_;

  v.clear();

  v.insert(v.begin(), comps, comps + nargs);

  return true;
}

// Delete module
void fm_module_del(fm_module_t *m) {
  for (auto node : m->inputs_) {
    delete node;
  }
  for (auto node : m->nodes_) {
    if (node->params_)
      fm_arg_buffer_del(node->params_);
    delete node;
  }
  delete m;
}

bool fm_module_comp_write(fm_module_comp_t *m, fm_writer writer,
                          void *closure) {
  if (!write_string(m->comp_, writer, closure)) {
    return false;
  }

  if (!write_string(m->name_, writer, closure)) {
    return false;
  }

  if (!write_number(m->inputs_.size(), writer, closure)) {
    return false;
  }

  auto comps = m->owner_->nodes_.size();
  auto inps = m->inputs_.size();

  for (unsigned i = 0; i < inps; ++i) {
    bool found = false;
    for (unsigned j = 0; j < comps; ++j) {
      auto *comp = m->owner_->nodes_[i];
      if (comp == m) {
        break;
      }
      if (comp == m->inputs_[i]) {
        if (!write_number(0, writer, closure)) {
          return false;
        }
        if (!write_number(j, writer, closure)) {
          return false;
        }
        found = true;
        break;
      }
    }
    if (found)
      continue;
    for (unsigned j = 0; j < m->owner_->inputs_.size(); ++j) {
      auto *comp = m->owner_->inputs_[j];
      if (comp == m->inputs_[i]) {
        if (!write_number(1, writer, closure)) {
          return false;
        }
        if (!write_number(j, writer, closure)) {
          return false;
        }
        break;
      }
    }
  }
  if (!fm_arg_write(m->params_, writer, closure)) {
    return false;
  }

  return true;
}

bool fm_module_comp_read_to(fm_comp_sys_t *sys, fm_module_t *m,
                            fm_reader reader, void *closure) {

  auto *tsys = fm_type_sys_get(sys);
  auto error = [sys](const char *str) {
    fm_comp_sys_error_set(
        sys, "[ERROR]\t(comp_sys) malformed module serialization; \n%s", str);
    return false;
  };

  auto comp_name = read_str(reader, closure);
  if (comp_name.empty()) {
    return error("could not read computation name");
  }

  auto name = read_str(reader, closure);
  const char *c_name = nullptr;
  if (!name.empty()) {
    c_name = name.c_str();
  }
  unsigned inpc;
  if (!read_unsigned(inpc, reader, closure)) {
    return error("could not read number of inputs");
  }

  vector<fm_module_comp_t *> inps(inpc);
  for (unsigned i = 0; i < inpc; ++i) {
    unsigned select_input;
    unsigned inp_idx;
    if (!read_unsigned(select_input, reader, closure)) {
      return error("could not read input idx");
    }
    if (!read_unsigned(inp_idx, reader, closure)) {
      return error("could not read input idx");
    }
    if (select_input)
      inps[i] = m->inputs_[inp_idx];
    else
      inps[i] = m->nodes_[inp_idx];
  }

  fm_type_decl_cp td = nullptr;
  fm_arg_stack_t *args = nullptr;
  fm_arg_buffer_t *arg_buf = fm_arg_read(tsys, &td, &args, reader, closure);
  if (!args) {
    return error("could not parse operator parameters");
  }

  auto *comp = fm_module_comp_add1(m, comp_name.c_str(), c_name, inpc,
                                   inps.data(), td, fm_arg_stack_args(args));

  fm_arg_stack_free(args);

  if (arg_buf)
    fm_arg_buffer_del(arg_buf);

  if (!comp) {
    return false;
  }

  return true;
}

bool fm_module_comp_is_out(fm_module_t *m, fm_module_comp_t *comp) {
  for (auto &c : m->outputs_) {
    if (c == comp)
      return true;
  }
  return false;
}
bool fm_module_write(fm_module_t *m, fm_writer writer, void *closure) {
  if (!write_string(m->name_, writer, closure)) {
    return false;
  }
  auto m_size = m->nodes_.size();
  if (!write_number(m_size, writer, closure)) {
    return false;
  }
  auto m_inps = fm_module_inps_size(m);
  if (!write_number(m_inps, writer, closure)) {
    return false;
  }
  auto m_outs = fm_module_outs_size(m);
  if (!write_number(m_outs, writer, closure)) {
    return false;
  }
  std::vector<size_t> out_idxs;
  for (unsigned i = 0; i < m_size; ++i) {
    auto *comp = m->nodes_[i];
    if (fm_module_comp_is_out(m, comp))
      out_idxs.push_back(i);
    if (!fm_module_comp_write(comp, writer, closure)) {
      return false;
    }
  }
  for (unsigned i = 0; i < m_outs; ++i) {
    if (!write_number(out_idxs[i], writer, closure)) {
      return false;
    }
  }
  return true;
}

fm_module_t *fm_module_read(fm_comp_sys_t *sys, fm_reader reader,
                            void *closure) {
  string name = read_str(reader, closure);

  auto error = [sys](const char *errmsg) {
    string errstr = "[ERROR]\t(comp_sys) malformed module serialization; ";
    errstr.append(errmsg);
    fm_comp_sys_error_set(sys, errstr.c_str());
    return nullptr;
  };
  const char *c_name = nullptr;
  if (!name.empty())
    c_name = name.c_str();
  unsigned m_size;
  if (!read_unsigned(m_size, reader, closure)) {
    return error("failed to read module nodes");
  }
  unsigned inps_size;
  if (!read_unsigned(inps_size, reader, closure)) {
    return error("failed to read module inputs");
  }
  unsigned outs_size;
  if (!read_unsigned(outs_size, reader, closure)) {
    return error("failed to read module outputs");
  }
  vector<fm_module_comp_t *> inputs(inps_size);
  auto *m = fm_module_new(c_name, inps_size, inputs.data());
  if (!m) {
    return error("unable to generate module");
  }

  for (unsigned i = 0; i < m_size; ++i) {
    if (!fm_module_comp_read_to(sys, m, reader, closure)) {
      fm_module_del(m);
      // we dont return an error so we dont override the error set in
      // fm_module_comp_read_to
      return nullptr;
    }
  }

  vector<fm_module_comp_t *> outputs(outs_size);
  for (unsigned i = 0; i < outs_size; ++i) {
    unsigned out_idx;
    if (!read_unsigned(out_idx, reader, closure)) {
      fm_module_del(m);
      return error("unable to read output node index");
    }
    outputs[i] = m->nodes_[out_idx];
  }

  if (!fm_module_outs_set(m, outs_size, outputs.data())) {
    fm_module_del(m);
    return error("unable to set outputs of module");
  }
  return m;
}
