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
 * @file csv_record.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "csv_record.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"

#include "fmc++/strings.hpp"
#include "fmc/files.h"

#include <cassert>
#include <functional>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

using namespace std;

struct csv_record_comp_cl {
  csv_record_comp_cl(const string &s, fm_type_sys_t *ts) : file(s), tsys(ts) {}
  string file;
  fm_type_sys_t *tsys;
  vector<string> fields;
};

struct csv_record_exec_cl {
  using writer = std::function<bool(const fm_frame_t *, int)>;
  vector<writer> writers;
  csv_record_exec_cl(FILE *f, vector<writer> &&ws, bool is_pipe)
      : writers(ws), file_(f), is_pipe_(is_pipe) {}
  ~csv_record_exec_cl() {
    if (file_) {
      if (is_pipe_) {
        fmc_error_t *err = nullptr;
        fmc_pclose(file_, &err);
      } else
        fclose(file_);
    }
  }
  FILE *file_ = nullptr;
  bool is_pipe_;
};

bool fm_comp_csv_record_call_stream_init(fm_frame_t *result, size_t args,
                                         const fm_frame_t *const argv[],
                                         fm_call_ctx_t *ctx,
                                         fm_call_exec_cl *cl) {
  using namespace std;
  auto *info = (csv_record_comp_cl *)ctx->comp;
  auto *tsys = info->tsys;

  FILE *file = nullptr;
  auto pipe = fmc::begins_with_pipe(info->file);
  auto *name = pipe.second.c_str();
  if (pipe.first) {
    fmc_error_t *err = nullptr;
    file = fmc_popen(name, "w", &err);
    if (err) {
      fm_exec_ctx_error_set(ctx->exec, "cannot execute %s: %s", name,
                            fmc_error_msg(err));
      return false;
    }
    if (!file) {
      fm_exec_ctx_error_set(ctx->exec, "cannot execute %s: %s", name,
                            strerror(errno));
      return false;
    }
  } else {
    file = fopen(name, "w");
    if (!file) {
      fm_exec_ctx_error_set(ctx->exec, "cannot open file %s for writing: %s",
                            name, strerror(errno));
      return false;
    }
  }

  auto error = [&]() {
    fm_exec_ctx_error_set(ctx->exec, "cannot write to file %s", name);
    return false;
  };

  auto *type = fm_frame_type(result);
  vector<csv_record_exec_cl::writer> writers;

  bool first = true;
  for (auto &field : info->fields) {
    const char *name = field.c_str();
    auto idx = fm_type_frame_field_idx(type, name);
    auto *ftype = fm_type_frame_field_type(type, idx);
    if (!first) {
      if (EOF == fputc(',', file))
        return error();
      writers.push_back([=](const fm_frame_t *frame, int row) {
        return fputc(',', file) != EOF;
      });
    }
    first = false;

    if (fputs(name, file) == EOF)
      return error();

    auto offset = fm_frame_field(result, name);
    auto *type_writer = fm_type_io_get(tsys, ftype);

    writers.push_back([=](const fm_frame_t *frame, int row) {
      const void *slot = fm_frame_get_cptr1(frame, offset, row);
      return fm_type_io_fwrite(type_writer, file, slot);
    });
  }
  if (EOF == fputc('\n', file))
    return error();
  writers.push_back([=](const fm_frame_t *frame, int row) {
    return fputc('\n', file) != EOF;
  });

  auto *exec_cl = new csv_record_exec_cl(file, move(writers), pipe.first);
  *cl = exec_cl;

  return true;
}

void fm_comp_csv_record_call_destroy(fm_call_exec_cl cl) {
  auto *exec_cl = (csv_record_exec_cl *)cl;
  if (exec_cl)
    delete exec_cl;
}

void fm_comp_csv_record_call_stream_destroy(fm_call_exec_cl cl) {
  fm_comp_csv_record_call_destroy(cl);
}

bool fm_comp_csv_record_stream_exec(fm_frame_t *result, size_t,
                                    const fm_frame_t *const argv[],
                                    fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *exec_cl = (csv_record_exec_cl *)cl;
  auto *input = argv[0];
  auto size = fm_frame_dim(input, 0);
  bool res = true;
  for (int i = 0; i < size; ++i) {
    for (auto &writer : exec_cl->writers) {
      res = writer(input, i);
      if (!res)
        goto error;
    }
  }

  return true;

error:
  auto *comp_cl = (csv_record_comp_cl *)ctx->comp;
  auto *name = comp_cl->file.c_str();
  fm_exec_ctx_error_set((fm_exec_ctx *)ctx->exec, "failed to write to %s",
                        name);

  return false;
}

fm_call_def *fm_comp_csv_record_stream_call(fm_comp_def_cl comp_cl,
                                            const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_csv_record_call_stream_init);
  fm_call_def_destroy_set(def, fm_comp_csv_record_call_stream_destroy);
  fm_call_def_exec_set(def, fm_comp_csv_record_stream_exec);
  return def;
}

/**
 * @brief generates CSV play operator
 *
 * The operator expects 1 parameters, file name
 * of the CSV file. It expects a single frame as
 * an argument.
 */
static const char *get_cstring_arg(fm_type_decl_cp ptype,
                                   fm_arg_stack_t &plist) {
  if (!ptype)
    return nullptr;
  if (fm_type_is_tuple(ptype) && fm_type_tuple_size(ptype) > 0) {
    ptype = fm_type_tuple_arg(ptype, 0);
  }
  return fm_arg_try_cstring(ptype, &plist);
}

fm_ctx_def_t *fm_comp_csv_record_gen(fm_comp_sys_t *csys,
                                     fm_comp_def_cl closure, unsigned argc,
                                     fm_type_decl_cp argv[],
                                     fm_type_decl_cp ptype,
                                     fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 1) {
    auto *errstr = "expect a single operator argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }
  auto type = argv[0];
  if (!fm_type_is_frame(type) || fm_type_frame_ndims(type) != 1) {
    auto *errstr = "the result of the input operator is expected to be "
                   "one-dimensional frame";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  const char *file = get_cstring_arg(ptype, plist);
  if (!file) {
    auto *errstr = "expect a file name as a parameter";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  fm_type_decl_cp second_arg = nullptr;
  if (fm_type_is_tuple(ptype)) {
    if (fm_type_tuple_size(ptype) > 2) {
      auto *errstr = "expecting at most two arguments";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    } else if (fm_type_tuple_size(ptype) == 2) {
      second_arg = fm_type_tuple_arg(ptype, 1);
      if (!fm_type_is_tuple(second_arg)) {
        auto *errstr = "expect second argument to be a tuple";
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
        return nullptr;
      }
    }
  }

  auto nf = fm_type_frame_nfields(type);
  for (size_t i = 0; i < nf; ++i) {
    if (!fm_type_is_simple(fm_type_frame_field_type(type, i))) {
      auto *errstr = "expect an input operator to have simple field "
                     "types";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    }
  }

  auto *ctx_cl = new csv_record_comp_cl(file, sys);

  if (second_arg) {
    auto size = fm_type_tuple_size(second_arg);
    for (unsigned i = 0; i < size; ++i) {
      auto field_t = fm_type_tuple_arg(second_arg, i);
      if (!fm_type_is_cstring(field_t)) {
        auto *errstr = "second argument must be a tuple of frame field "
                       "names";
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
        delete ctx_cl;
        return nullptr;
      }
      auto *field = STACK_POP(plist, const char *);
      if (fm_type_frame_field_idx(type, field) < 0) {
        auto *errstr = "second argument must be a tuple of frame field "
                       "names";
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
        delete ctx_cl;
        return nullptr;
      }
      ctx_cl->fields.emplace_back(field);
    }
  } else {
    for (unsigned i = 0; i < nf; ++i) {
      auto *name = fm_type_frame_field_name(type, i);
      ctx_cl->fields.emplace_back(name);
    }
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, true);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_csv_record_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_csv_record_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (csv_record_comp_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
