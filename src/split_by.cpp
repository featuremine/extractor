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
 * @file split_by.cpp
 * @author Andres Rangel
 * @date Jan 28 2019
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "split_by.h"
#include "comp.h"
#include "comp_graph.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/module.h"
#include "fmc/time.h"
#include "stream_ctx.h"
}
#include "comp_sys.hpp"

#include "fmc++/time.hpp"
#include "frame_pool.hpp"
#include "unique_pq.hpp"

#include "fmc++/metatable.hpp"

#include <condition_variable>
#include <deque>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

bool fm_split_by_input_call_stream_init(fm_frame_t *result, size_t args,
                                        const fm_frame_t *const argv[],
                                        fm_call_ctx_t *ctx,
                                        fm_call_exec_cl *cl) {
  return true;
}

bool fm_split_by_input_stream_exec(fm_frame_t *result, size_t,
                                   const fm_frame_t *const argv[],
                                   fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  return true;
}

fm_call_def *fm_split_by_input_stream_call(fm_comp_def_cl comp_cl,
                                           const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_split_by_input_call_stream_init);
  fm_call_def_exec_set(def, fm_split_by_input_stream_exec);
  return def;
}

fm_ctx_def_t *fm_split_by_input_gen(fm_comp_sys_t *sys, fm_comp_def_cl closure,
                                    unsigned argc, fm_type_decl_cp argv[],
                                    fm_type_decl_cp ptype,
                                    fm_arg_stack_t plist) {
  auto tsys = fm_type_sys_get(sys);

  if (argc != 0) {
    auto *errstr = "expect no operator arguments";
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto param_error = [&]() {
    auto *errstr = "expect output type as single parameter";
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!fm_type_is_type(ptype)) {
    return param_error();
  }

  fm_type_decl_cp type = STACK_POP(plist, fm_type_decl_cp);

  if (!type)
    return param_error();

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, closure);
  fm_ctx_def_stream_call_set(def, &fm_split_by_input_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

struct module_cl {
  std::string key;
  size_t index;
  fm_frame_t *inp;
  fm_call_handle_t in_handle;
  fm_stream_ctx_t *stream_ctx;

  struct split_by_cl *split_by_cl;
};

struct split_by_cl {
  using item = std::pair<fmc_time64_t, module_cl *>;
  struct compare {
    bool operator()(const item &a, const item &b) {
      return fmc_time64_less(a.first, b.first) ||
             (!fmc_time64_less(b.first, a.first) &&
              a.second->index < b.second->index);
    }
  };

  split_by_cl(fm_field_t in_split_field, fm_field_t out_split_field,
              std::vector<std::pair<fm_field_t, fm_field_t>> fields_map,
              size_t split_field_size, fm_comp_sys_t *sys, fm_module_t *module,
              fm_type_decl_cp base_type)
      : module_map([&](const std::string &key) { return module_cl_gen(key); }),
        in_split_field(in_split_field), out_split_field(out_split_field),
        fields_map(std::move(fields_map)),
        split_field_buffer(split_field_size, '\0'), sys(sys), module(module),
        base_type(base_type) {}

  module_cl *module_cl_gen(const std::string &key) {
    auto tsys = fm_type_sys_get(sys);

    auto *g = fm_comp_graph_get(sys);
    if (!g) {
      fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_UNKNOWN,
                             "unable to obtain new graph for operator");
      return nullptr;
    }

    fm_comp_t *inp_comps[1] = {fm_comp_decl(sys, g, "split_by_input", 0,
                                            fm_type_type_get(tsys), base_type)};

    if (!inp_comps[0]) {
      fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_UNKNOWN,
                             "unable to generate split_by_input computation");
      return nullptr;
    }

    fm_comp_t *out_comps[1];
    if (!fm_module_inst(sys, g, module, inp_comps, out_comps)) {
      fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_UNKNOWN,
                             "unable to instantiate module");
      return nullptr;
    }

    auto m_cl = std::make_unique<module_cl>();

    fm_comp_clbck_set(
        out_comps[0],
        [](const fm_frame *f, void *cl, fm_call_ctx_t *) {
          auto *module = (module_cl *)cl;
          auto *split_by_cl = module->split_by_cl;

          for (auto &m : split_by_cl->fields_map) {
            fm_frame_field_copy(split_by_cl->result, m.second, f, m.first);
          }

          auto *to = fm_frame_get_ptr1(split_by_cl->result,
                                       split_by_cl->out_split_field, 0);
          memcpy(to, module->key.data(), module->key.size());

          split_by_cl->output_updated = true;
        },
        m_cl.get());

    fm_stream_ctx_t *g_ctx = fm_stream_ctx_get(sys, g);

    if (!g_ctx) {
      fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_UNKNOWN,
                             "unable to obtain new graph for operator");
      return nullptr;
    }

    m_cl->key = key;
    m_cl->index = module_map.size();
    m_cl->inp = fm_data_get(fm_result_ref_get(inp_comps[0]));
    m_cl->in_handle = fm_call_obj_handle(fm_comp_call(inp_comps[0]));
    m_cl->stream_ctx = g_ctx;
    m_cl->split_by_cl = this;
    return m_cl.release();
  }

  fmc::metatable<std::string, module_cl> module_map;
  fm::unique_pq<item, std::vector<item>, compare> queue;
  fmc_time64_t next_scheduled_time = fmc_time64_end();
  fm_field_t in_split_field = -1;
  fm_field_t out_split_field = -1;
  std::vector<std::pair<fm_field_t, fm_field_t>> fields_map;
  std::string split_field_buffer;
  fm_comp_sys_t *sys;
  fm_module_t *module;
  fm_type_decl_cp base_type;
  fm_frame_t *result;
  size_t index = 0;
  bool input_updated = false;
  bool output_updated = false;
};

bool fm_comp_split_by_call_stream_init(fm_frame_t *result, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx,
                                       fm_call_exec_cl *cl) {
  return true;
}

bool fm_comp_split_by_stream_exec(fm_frame_t *result, size_t,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *comp_cl = (split_by_cl *)ctx->comp;
  auto stream_ctx = (fm_stream_ctx_t *)ctx->exec;
  auto now = fm_stream_ctx_now(stream_ctx);

  module_cl *module = nullptr;
  if (comp_cl->input_updated) {
    comp_cl->input_updated = false;
    auto data = argv[0];

    if (!fm_frame_singleton(data)) {
      fm_type_sys_err_custom(fm_type_sys_get(comp_cl->sys),
                             FM_TYPE_ERROR_UNKNOWN,
                             "source data must be a singleton");
      return false;
    }

    memcpy(comp_cl->split_field_buffer.data(),
           fm_frame_get_cptr1(data, comp_cl->in_split_field, 0),
           comp_cl->split_field_buffer.size());

    module = &comp_cl->module_map(comp_cl->split_field_buffer);

    if (module == nullptr) {
      return false;
    }

    fm_frame_assign(module->inp, argv[0]);
    fm_call_queue_push(fm_stream_ctx_get_queue(module->stream_ctx),
                       module->in_handle);
  } else {
    if (comp_cl->queue.top().first > now) {
      return false;
    }
    module = comp_cl->queue.pop().second;
  }

  if (comp_cl->next_scheduled_time <= now) {
    comp_cl->next_scheduled_time = fmc_time64_end();
  }

  comp_cl->result = result;

  fm_stream_ctx_proc_one(module->stream_ctx, now);

  bool output_updated = comp_cl->output_updated;
  comp_cl->output_updated = false;

  auto module_next_time = fm_stream_ctx_next_time(module->stream_ctx);
  if (module_next_time != fmc_time64_end()) {
    comp_cl->queue.push(std::make_pair(module_next_time, module));
  }

  if (!comp_cl->queue.empty()) {
    auto desired_next_scheduled_time = comp_cl->queue.top().first;
    if (fmc_time64_less(desired_next_scheduled_time,
                       comp_cl->next_scheduled_time)) {
      fm_stream_ctx_schedule(stream_ctx, ctx->handle,
                             desired_next_scheduled_time);
      comp_cl->next_scheduled_time = desired_next_scheduled_time;
    }
  }

  return output_updated;
}

fm_call_def *fm_comp_split_by_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_split_by_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_split_by_stream_exec);
  return def;
}

void fm_comp_split_by_queuer(size_t idx, fm_call_ctx_t *ctx) {
  auto *comp_cl = (split_by_cl *)ctx->comp;
  if (idx == 0) {
    comp_cl->input_updated = true;
  }
}

fm_ctx_def_t *fm_comp_split_by_gen(fm_comp_sys_t *sys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
                                   fm_type_decl_cp ptype,
                                   fm_arg_stack_t plist) {
  auto tsys = fm_type_sys_get(sys);

  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto param_error = [&]() {
    auto *errstr = "expect a module and a field name as parameters";
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 2) {
    return param_error();
  }

  if (!fm_type_is_module(fm_type_tuple_arg(ptype, 0))) {
    return param_error();
  }

  if (!fm_type_is_cstring(fm_type_tuple_arg(ptype, 1))) {
    return param_error();
  }

  auto *m = STACK_POP(plist, fm_module_t *);

  if (!m) {
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_PARAMS,
                           "unable to obtain "
                           "module from parameters");
    return nullptr;
  }

  if (fm_module_inps_size(m) != 1) {
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_PARAMS,
                           "module parameter must have "
                           "one input");
    return nullptr;
  }

  if (fm_module_outs_size(m) != 1) {
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_PARAMS,
                           "module parameter must have "
                           "one output");
    return nullptr;
  }

  auto field_name = STACK_POP(plist, const char *);
  auto in_field_idx = fm_type_frame_field_idx(argv[0], field_name);
  if (in_field_idx < 0) {
    auto *errstr = "field name parameter must be field name of operator "
                   "argument input";
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto type_error = [&]() {
    auto *errstr = "split value type must be an array of char";
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  auto field_type = fm_type_frame_field_type(argv[0], in_field_idx);
  if (!fm_type_is_array(field_type)) {
    return type_error();
  }
  if (!fm_type_is_base(fm_type_array_of(field_type))) {
    return type_error();
  }
  if (fm_type_base_enum(fm_type_array_of(field_type)) != FM_TYPE_CHAR) {
    return type_error();
  }

  auto field_size = fm_type_array_size(field_type);
  auto *g = fm_comp_graph_new();

  if (!g) {
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_UNKNOWN,
                           "unable to obtain new graph for operator");
    return nullptr;
  }

  fm_comp_t *inp_comps[1] = {fm_comp_decl(sys, g, "split_by_input", 0,
                                          fm_type_type_get(tsys), argv[0])};
  if (!inp_comps[0]) {
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_UNKNOWN,
                           "unable to generate base computation");
    return nullptr;
  }

  fm_comp_t *out_comps[1];
  if (!fm_module_inst(sys, g, m, inp_comps, out_comps)) {
    fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_UNKNOWN,
                           "unable to instantiate module");
    return nullptr;
  }

  auto out_t = fm_comp_result_type(out_comps[0]);

  auto nf = fm_type_frame_nfields(out_t);
  vector<const char *> names(nf + 1);
  vector<fm_type_decl_cp> types(nf + 1);
  for (unsigned int idx = 0; idx < nf; ++idx) {
    names[idx] = fm_type_frame_field_name(out_t, idx);
    types[idx] = fm_type_frame_field_type(out_t, idx);
    if (strcmp(names[idx], field_name) == 0) {
      auto *error = "join label cannot be the same one of the fields";
      fm_type_sys_err_custom(tsys, FM_TYPE_ERROR_PARAMS, error);
      return nullptr;
    }
  }
  names.back() = field_name;
  types.back() = field_type;

  auto nd = fm_type_frame_ndims(out_t);
  vector<int> dims(nd);
  for (unsigned int i = 0; i < nd; ++i) {
    dims[i] = fm_type_frame_dim(out_t, i);
  }

  auto type = fm_frame_type_get1(tsys, names.size(), names.data(), types.data(),
                                 dims.size(), dims.data());
  if (!type) {
    return nullptr;
  }

  std::vector<std::pair<fm_field_t, fm_field_t>> fields_map(nf);
  for (unsigned int idx = 0; idx < nf; ++idx) {
    fields_map[idx] = {
        fm_type_frame_field_idx(out_t, names[idx]),
        fm_type_frame_field_idx(type, names[idx]),
    };
  }

  auto *cl =
      new split_by_cl(in_field_idx, fm_type_frame_field_idx(type, field_name),
                      std::move(fields_map), field_size, sys, m, argv[0]);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, cl);
  fm_ctx_def_queuer_set(def, &fm_comp_split_by_queuer);
  fm_ctx_def_stream_call_set(def, &fm_comp_split_by_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);

  fm_comp_graph_del(g);

  return def;
}

void fm_comp_split_by_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (split_by_cl *)fm_ctx_def_closure(def);
  delete ctx_cl;
}

bool fm_comp_split_by_add(fm_comp_sys_t *sys) {
  fm_comp_def_t base_def = {"split_by_input", fm_split_by_input_gen, NULL,
                            NULL};

  fm_comp_def_t sb_def = {"split_by", fm_comp_split_by_gen,
                          fm_comp_split_by_destroy, NULL};

  return fm_comp_type_add(sys, &base_def) && fm_comp_type_add(sys, &sb_def);
}
