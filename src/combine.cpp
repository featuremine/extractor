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
 * @file combine.cpp
 * @author Maxim Trokhimtchouk
 * @date Mar 28 2017
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "combine.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

struct combine_closure {
  struct field {
    unsigned input;
    fm_field_t from;
    fm_field_t to;
  };
  vector<field> fields;
};

bool fm_comp_combine_call_stream_init(fm_frame_t *result, size_t args,
                                      const fm_frame_t *const argv[],
                                      fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto &fields = ((combine_closure *)ctx->comp)->fields;
  for (auto &&[argi, from, to] : fields) {
    fm_frame_field_copy(result, to, argv[argi], from);
  }
  return true;
}

bool fm_comp_combine_stream_exec(fm_frame_t *result, size_t,
                                 const fm_frame_t *const argv[],
                                 fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &fields = ((combine_closure *)ctx->comp)->fields;
  for (auto &&[argi, from, to] : fields) {
    fm_frame_field_copy(result, to, argv[argi], from);
  }
  return true;
}

fm_call_def *fm_comp_combine_stream_call(fm_comp_def_cl comp_cl,
                                         const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_combine_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_combine_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_combine_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                  unsigned argc, fm_type_decl_cp argv[],
                                  fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc < 1) {
    auto *errstr = "expect at lease one operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != argc) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                           "expecting to have the same number of "
                           "parameters as inputs");
    return nullptr;
  }

  vector<tuple<unsigned, int, string>> fields;

  for (unsigned argi = 0; argi < argc; ++argi) {
    auto *inp = argv[argi];
    auto *param = fm_type_tuple_arg(ptype, argi);
    if (!fm_type_is_tuple(param)) {
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                             "expecting each parameter to be a "
                             "tuple of field mappings");
      return nullptr;
    }
    auto nf = fm_type_tuple_size(param);
    if (nf == 0) {
      int nf = fm_type_frame_nfields(inp);
      for (int idx = 0; idx < nf; ++idx) {
        string name = fm_type_frame_field_name(inp, idx);
        fields.emplace_back(argi, idx, name);
      }
    } else if (nf == 1 && fm_type_is_cstring(fm_type_tuple_arg(param, 0))) {
      auto nf = fm_type_frame_nfields(inp);
      if (nf != 1) {
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                               "default field map can only be used "
                               "with an input with a single field");
        return nullptr;
      }
      string name = STACK_POP(plist, const char *);
      fields.emplace_back(argi, 0, name);
    } else {
      for (unsigned idx = 0; idx < nf; ++idx) {
        auto fmap = fm_type_tuple_arg(param, idx);
        if (!fm_type_is_tuple(fmap) || fm_type_tuple_size(fmap) != 2) {
          fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS,
                                 "each field mapping must be a tuple "
                                 "of two field names");
          return nullptr;
        }
        auto *from_name = STACK_POP(plist, const char *);
        string name = STACK_POP(plist, const char *);
        auto fld = fm_type_frame_field_idx(inp, from_name);
        if (fld < 0) {
          ostringstream os;
          os << "field " << from_name << " does not exist in the input "
             << argi;
          fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, os.str().c_str());
          return nullptr;
        }
        fields.emplace_back(argi, fld, name);
      }
    }
  }

  int nd = fm_type_frame_ndims(argv[0]);
  vector<int> dims(nd);
  for (int i = 0; i < nd; ++i) {
    dims[i] = fm_type_frame_dim(argv[0], i);
    if (dims[i] <= 0) {
      ostringstream os;
      os << "the dimension " << i
         << " of input frame 0 has a "
            "non-positive value";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, os.str().c_str());
      return nullptr;
    }
  }
  for (unsigned argi = 1; argi < argc; ++argi) {
    auto *inp = argv[argi];
    int inp_nd = fm_type_frame_ndims(inp);
    if (nd != inp_nd) {
      ostringstream os;
      os << "input " << argi << " has " << inp_nd
         << " dimensions while input 0 has " << nd;
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, os.str().c_str());
      return nullptr;
    }
    for (int i = 0; i < nd; ++i) {
      auto inp_dim = fm_type_frame_dim(inp, i);
      if (dims[i] != inp_dim) {
        ostringstream os;
        os << "the dimension " << i << " of the input 0 is " << dims[i]
           << " while input " << argi << " has " << inp_dim;
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, os.str().c_str());
        return nullptr;
      }
    }
  }

  auto nf = fields.size();
  vector<const char *> names(nf);
  vector<fm_type_decl_cp> types(nf);
  int i = 0;
  for (auto &&[argi, fld, name] : fields) {
    auto *inp = argv[argi];
    names[i] = name.c_str();
    types[i] = fm_type_frame_field_type(inp, fld);
    ++i;
  }

  auto type =
      fm_frame_type_get1(sys, nf, names.data(), types.data(), nd, dims.data());
  if (!type) {
    return nullptr;
  }
  auto *cl = new combine_closure();

  for (auto &&[argi, from_fld, name] : fields) {
    auto to_fld = fm_type_frame_field_idx(type, name.c_str());
    cl->fields.emplace_back(combine_closure::field{argi, from_fld, to_fld});
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_combine_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_combine_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (combine_closure *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
