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
 * @file csv_tail.cpp
 * @author Andrus Suvalau
 * @date 17 Mar 2020
 * @brief File contains C++ definitions of the csv_tail operator
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "csv_tail.h"
#include "csv_play.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}
#include "csv_utils.hpp"
#include "errno.h"
#include "fmc/files.h"
#include <functional>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include "fmc++/time.hpp"
#include "fmc++/strings.hpp"

struct csv_tail_exec_cl {
  explicit csv_tail_exec_cl(FILE *file, bool is_pipe) : reader(file, is_pipe) {}
  csv_reader reader;
  std::vector<csv_parser> parsers;
  std::vector<std::string> header;
  uint64_t row = 0;
  bool columns_initialized = false;
};

struct csv_tail_info {
  fm_type_sys_t *tsys;
  std::string file;
  fmc_time64_t polling_period;
  std::vector<csv_column_info> columns;
};

int process_row(fm_frame_t *frame, fm_call_ctx_t *ctx,
                csv_tail_exec_cl *exec_cl) {
  auto *exec_ctx = (fm_exec_ctx *)ctx->exec;

  auto error = [exec_ctx](auto &&...args) {
    fm_exec_ctx_error_set(exec_ctx, args...);
    return -1;
  };

  auto executed_line = exec_cl->reader.try_read_line();
  if (executed_line < 0)
    return error("unable to read a line");
  else if (executed_line == 0)
    return 0;

  auto view = exec_cl->reader.view();
  bool first = true;
  int column = 0;
  for (auto &parser : exec_cl->parsers) {
    ++column;
    if (!first) {
      auto pos = parse_comma(view);
      if (!pos)
        return error("unable to parse comma in row %d before column %d  with "
                     "the name %s",
                     exec_cl->row + 1, column,
                     exec_cl->header[column - 1].c_str());
      view = view.substr(pos);
    }
    first = false;
    auto pos = parser(view, frame, 0);
    if (pos == -1)
      return error("unable to parse value in row %d in column %d with "
                   "the name %s",
                   exec_cl->row + 1, column,
                   exec_cl->header[column - 1].c_str());
    view = view.substr(pos);
  }
  ++exec_cl->row;
  return 1;
}

int try_init_columns(fm_frame_t *result, fm_call_ctx_t *ctx,
                     csv_tail_exec_cl *exec_cl) {
  auto *info = (csv_tail_info *)ctx->comp;
  auto *ts = info->tsys;
  const auto *name = info->file.c_str();

  auto &reader = exec_cl->reader;
  auto executed_line = reader.try_read_line();

  if (executed_line < 0) {
    fm_exec_ctx_error_set(ctx->exec, "unable to read a line");
    return -1;
  } else if (executed_line == 0)
    return 0;
  else if (reader.view().empty()) {
    fm_exec_ctx_error_set(ctx->exec, "expecting csv header in %s", name);
    return -1;
  }

  auto &columns = info->columns;

  std::vector<std::pair<csv_column_info *, bool>> cols;
  cols.reserve(columns.size());
  std::transform(columns.begin(), columns.end(), std::back_inserter(cols),
                 [](auto &column) -> std::pair<csv_column_info *, bool> {
                   return {&column, false};
                 });

  std::vector<std::string> headers;
  std::vector<csv_parser> parsers;

  bool first = true;
  auto view = reader.view();
  while (!view.empty()) {
    if (!first) {
      auto pos = parse_comma(view);
      if (!pos)
        break;
      view = view.substr(pos);
    }
    first = false;

    auto pos = parse_header(view);
    if (!pos) {
      fm_exec_ctx_error_set(ctx->exec, "expecting non-empty header in %s",
                            name);
      return -1;
    }
    auto header_name = view.substr(0, pos);

    headers.emplace_back(header_name);

    auto it =
        std::find_if(cols.begin(), cols.end(), [header_name](const auto &col) {
          return header_name == col.first->name;
        });

    if (it == cols.cend())
      parsers.push_back(skip_parser);
    else {
      if (it->second) {
        fm_exec_ctx_error_set(ctx->exec, "duplicate field %s",
                              std::string(header_name).c_str());
        return -1;
      }
      parsers.push_back(get_column_parser(ts, result, it->first));
      it->second = true;
    }

    view = view.substr(pos);
  };

  if (!view.empty()) {
    fm_exec_ctx_error_set(ctx->exec, "header format error in %s", name);
    return -1;
  }

  auto count = std::count_if(cols.cbegin(), cols.cend(),
                             [](const auto &col) { return col.second; });

  if (static_cast<size_t>(count) < cols.size()) {
    auto it = std::find_if(cols.cbegin(), cols.cend(),
                           [](const auto &col) { return !col.second; });
    fm_exec_ctx_error_set(ctx->exec, "header %s does not contain column %s",
                          name, it->first->name.c_str());
    return -1;
  }

  std::swap(headers, exec_cl->header);
  std::swap(parsers, exec_cl->parsers);

  return 1;
}

bool fm_comp_csv_tail_stream_init(fm_frame_t *result, size_t args,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto *info = (csv_tail_info *)ctx->comp;

  FILE *file = nullptr;
  auto [is_pipe, name] = fmc::ends_with_pipe(info->file);

  if (is_pipe) {
    fmc_error_t *err = nullptr;
    file = fmc_popen(name.c_str(), "r", &err);
    if (err) {
      fm_exec_ctx_error_set(ctx->exec, "cannot execute %s: %s", name.c_str(),
                            fmc_error_msg(err));
      return false;
    }
    if (file == nullptr) {
      fm_exec_ctx_error_set(ctx->exec, "cannot execute %s: %s", name.c_str(),
                            strerror(errno));
      return false;
    }
  } else {
    fmc_error_t *error;
    if (!fmc_basedir_exists(name.c_str(), &error)) {
      fm_exec_ctx_error_set(ctx->exec, "folder doesn't exist for file %s",
                            name.c_str());
      return false;
    }
    file = fopen(name.c_str(), "r");
    if (file == nullptr) {
      fm_exec_ctx_error_set(ctx->exec, "cannot open file %s: %s", name.c_str(),
                            strerror(errno));
      return false;
    }
  }

  auto *exec_cl = new csv_tail_exec_cl(file, is_pipe);
  auto error = [=]() {
    delete exec_cl;
    return false;
  };

  auto init_columns_res = try_init_columns(result, ctx, exec_cl);
  if (init_columns_res < 0)
    return error();

  exec_cl->columns_initialized = init_columns_res > 0;
  *cl = exec_cl;

  auto *s_ctx = (fm_stream_ctx *)ctx->exec;
  fm_stream_ctx_queue(s_ctx, ctx->handle);
  return true;
}

void fm_comp_csv_tail_call_destroy(fm_call_exec_cl cl) {
  auto *exec_cl = (csv_tail_exec_cl *)cl;
  if (exec_cl != nullptr)
    delete exec_cl;
}

void fm_comp_csv_tail_call_stream_destroy(fm_call_exec_cl cl) {
  fm_comp_csv_tail_call_destroy(cl);
}

bool fm_comp_csv_tail_stream_exec(fm_frame_t *result, size_t,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *exec_cl = (csv_tail_exec_cl *)cl;
  auto *s_ctx = (fm_stream_ctx *)ctx->exec;
  auto *info = (csv_tail_info *)ctx->comp;

  if (!exec_cl->columns_initialized) {
    auto init_column = try_init_columns(result, ctx, exec_cl);
    if (init_column < 0)
      return false;
    exec_cl->columns_initialized = init_column > 0;
  }

  bool res = false;
  if (exec_cl->columns_initialized) {
    auto proc_row = process_row(result, ctx, exec_cl);
    if (proc_row < 0)
      return false;
    res = proc_row > 0;
  }

  if (res)
    fm_stream_ctx_queue(s_ctx, ctx->handle);
  else {
    auto now = fm_stream_ctx_now(s_ctx);
    auto next = now + info->polling_period;
    fm_stream_ctx_schedule(s_ctx, ctx->handle, next);
  }

  return res;
}

fm_call_def *fm_comp_csv_tail_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_csv_tail_stream_init);
  fm_call_def_destroy_set(def, fm_comp_csv_tail_call_stream_destroy);
  fm_call_def_exec_set(def, fm_comp_csv_tail_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_csv_tail_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
                                   fm_type_decl_cp ptype,
                                   fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 0) {
    fm_type_sys_err_set(sys, FM_TYPE_ERROR_ARGS);
    return nullptr;
  }

  auto error = [&](const char *errstr) {
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 3)
    return error("expect a file name, polling period and"
                 "a tuple of field descriptions");

  auto *file_name_param = fm_type_tuple_arg(ptype, 0);
  if (!fm_type_is_cstring(file_name_param))
    return error("expect first parameter to be a file name");
  const char *file = STACK_POP(plist, const char *);

  fmc_time64_t polling_period{0};
  auto *polling_period_param = fm_type_tuple_arg(ptype, 1);
  if (!fm_arg_try_time64(polling_period_param, &plist, &polling_period))
    return error("expect second parameter to be a polling period");

  auto *row_descs = fm_type_tuple_arg(ptype, 2);
  if (!fm_type_is_tuple(row_descs))
    return error("expect third parameter to be a tuple of field descriptions");
  auto size = fm_type_tuple_size(row_descs);

  auto *str_t = fm_cstring_type_get(sys);
  auto *type_t = fm_type_type_get(sys);
  auto *row_desc_t = fm_tuple_type_get(sys, 3, str_t, type_t, str_t);
  auto *row_desc2_t = fm_tuple_type_get(sys, 2, str_t, type_t);

  std::vector<csv_column_info> cols(size);
  std::vector<const char *> names(size);
  std::vector<fm_type_decl_cp> types(size);
  int dims[1] = {1};

  for (unsigned i = 0; i < size; ++i) {
    auto *row_desc = fm_type_tuple_arg(row_descs, i);
    if (fm_type_equal(row_desc, row_desc_t)) {
      cols[i].name = names[i] = STACK_POP(plist, const char *);
      cols[i].type = types[i] = STACK_POP(plist, fm_type_decl_cp);
      cols[i].format = STACK_POP(plist, const char *);
    } else if (fm_type_equal(row_desc, row_desc2_t)) {
      cols[i].name = names[i] = STACK_POP(plist, const char *);
      cols[i].type = types[i] = STACK_POP(plist, fm_type_decl_cp);
      cols[i].format = "";
    } else
      return error("each field description must contain field name, "
                   "field type and an optional format description");

    if (!fm_type_is_simple(types[i])) {
      auto *typestr = fm_type_to_str(types[i]);
      auto errstr = std::string("expect simple type, got: ") + typestr;
      free(typestr);
      return error(errstr.c_str());
    }
  }

  auto *type =
      fm_frame_type_get1(sys, size, names.data(), types.data(), 1, dims);
  if (!type)
    return error("unable to create result frame type");

  auto *ctx_cl = new csv_tail_info{sys, file, polling_period, cols};

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_csv_tail_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_csv_tail_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (csv_tail_info *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
