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
 * @file csv_play.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

#include "csv_play.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/frame.hpp"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"

#include "csv_utils.hpp"
#include "errno.h"
#include "fmc/files.h"
#include <functional>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include "fmc++/strings.hpp"

using namespace std;

struct csv_play_info {
  template <class B, class E>
  csv_play_info(fm_type_sys_t *ts, const string &f, const string &i, B b, E e)
      : tsys(ts), file(f), index(i), columns(b, e) {}
  fm_type_sys_t *tsys;
  string file;
  string index;
  vector<csv_column_info> columns;
};

struct csv_play_exec_cl {
  csv_play_exec_cl(FILE *file, bool is_pipe = false) : buf(file, is_pipe) {}
  parse_buf buf;
  fm_field_t index;
  fm_frame_t *next;
  vector<csv_parser> parsers;
  vector<string> header;
  uint64_t row = 0;
};

void csv_play_error_set(fm_exec_ctx *ctx, csv_play_info *info,
                        const char *error_cause = nullptr) {
  string errorstr;
  if (error_cause) {
    errorstr = error_cause;
    errorstr.append("\n");
  }
  if (errno != 0) {
    errorstr.append("failed to read ");
    errorstr.append(info->file.c_str());
    errorstr.append(" with system error ");
    errorstr.append(strerror(errno));
    fm_exec_ctx_error_set(ctx, errorstr.c_str());
  } else {
    errorstr.append("failed to read ");
    errorstr.append(info->file.c_str());
    fm_exec_ctx_error_set(ctx, errorstr.c_str());
  }
}

int csv_parse_one(fm_call_ctx *ctx, csv_play_exec_cl *cl, fm_frame_t *frame) {
  ++cl->row;
  auto *exec_ctx = (fm_exec_ctx *)ctx->exec;
  auto *info = (csv_play_info *)ctx->comp;

  auto error = [exec_ctx, info](const char *err = nullptr) {
    string errstr;
    errstr.append(err);
    csv_play_error_set(exec_ctx, info, errstr.c_str());
    return -1;
  };

  if (!cl->buf.file_)
    return 0;
  if (auto res = cl->buf.read_line(); res < 1) {
    if (res < 0)
      return error("failed to read line from buffer");
    return 0;
  }
  if (cl->buf.is_pipe && feof(cl->buf.file_)) {
    fmc_error_t *err = nullptr;
    int pc = fmc_pclose(cl->buf.file_, &err);
    if (err)
      return error(fmc_error_msg(err));
    cl->buf.file_ = nullptr;
    if (pc != 0)
      return error("failed to close pipe successfully");
  }
  auto view = cl->buf.view();
  bool first = true;
  int column = 0;
  for (auto &parser : cl->parsers) {
    ++column;
    if (!first) {
      auto pos = parse_comma(view);
      if (!pos)
        return error((string("unable to parse comma in row ") +
                      to_string(cl->row) + " before column " +
                      to_string(column) + " with the name " +
                      cl->header[column - 1])
                         .c_str());
      view = view.substr(pos);
    }
    first = false;
    auto pos = parser(view, frame, 0);
    if (pos == std::string_view::npos) {
      std::string typeinfo = " with type ";
      fm_type_decl_cp tp =
          fm_frame_field_type(frame, cl->header[column - 1].c_str());
      if (tp)
        typeinfo += fm::fm_type_to_string(tp);
      else
        typeinfo += "unknown";

      return error((string("unable to parse value in row ") +
                    to_string(cl->row) + " in column " + to_string(column) +
                    " with the name " + cl->header[column - 1] + typeinfo)
                       .c_str());
    }

    view = view.substr(pos);
  }
  return cl->buf.view().size();
}

bool fm_comp_csv_play_call_init(fm_frame_t *result, size_t args,
                                const fm_frame_t *const argv[],
                                fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  using namespace std;
  auto *info = (csv_play_info *)ctx->comp;
  auto *ts = info->tsys;

  FILE *file = nullptr;
  auto pipe = fmc::ends_with_pipe(info->file);
  auto name_str = string(pipe.second);
  const char *name = name_str.c_str();
  if (pipe.first) {
    fmc_error_t *err = nullptr;
    file = fmc_popen(name, "r", &err);
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
    fmc_error_t *error;
    if (!fmc_basedir_exists(name, &error)) {
      fm_exec_ctx_error_set(ctx->exec, "folder doesn't exist for file %s",
                            name);
      return false;
    }
    file = fopen(name, "r");
    if (!file) {
      fm_exec_ctx_error_set(ctx->exec, "cannot open file %s: %s", name,
                            strerror(errno));
      return false;
    }
  }

  auto *exec_cl = new csv_play_exec_cl(file, pipe.first);
  auto error = [=]() {
    delete exec_cl;
    return false;
  };

  auto &columns = info->columns;
  using column_use = pair<csv_column_info *, bool>;
  auto size = columns.size();
  vector<column_use> cols(size);
  unsigned i = 0;
  for (auto it = begin(columns); i < size; ++i, ++it) {
    cols[i].first = &(*it);
    cols[i].second = false;
  }

  unsigned count = 0;
  auto &buf = exec_cl->buf;

  int res = buf.read_line();
  if (res < 0) {
    fm_exec_ctx_error_set(ctx->exec, "error reading csv header in %s", name);
    return error();
  }
  if (res == 0) {
    fm_exec_ctx_error_set(ctx->exec, "expecting csv header in %s", name);
    return error();
  }

  bool first = true;
  auto view = buf.view();
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
      return error();
    }
    if (pos == std::string_view::npos) {
      fm_exec_ctx_error_set(ctx->exec, "invalid header in %s", name);
      return error();
    }
    auto header_name = view.substr(0, pos);
    exec_cl->header.push_back(string(header_name));
    unsigned i = 0;
    for (; i < size; ++i) {
      if (header_name == cols[i].first->name)
        break;
    }
    if (i == size) {
      exec_cl->parsers.push_back(skip_parser);
    } else {
      auto &found = cols[i];
      if (found.second == true) {
        fm_exec_ctx_error_set(ctx->exec, "duplicate field %s in column %d",
                              string(header_name).c_str(), i);
        return error();
      }
      exec_cl->parsers.push_back(get_column_parser(ts, result, found.first));
      found.second = true;
      ++count;
    }

    view = view.substr(pos);
  };

  if (!view.empty()) {
    fm_exec_ctx_error_set(ctx->exec, "header format error in %s", name);
    return error();
  }

  if (count < size) {
    int i = 0;
    while (cols[i].second) {
      ++i;
    }
    fm_exec_ctx_error_set(ctx->exec, "header %s does not contain column %s",
                          name, cols[i].first->name.c_str());
    return error();
  }

  *cl = exec_cl;
  return true;
}

bool fm_comp_csv_play_call_stream_init(fm_frame_t *result, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx,
                                       fm_call_exec_cl *cl) {
  if (!fm_comp_csv_play_call_init(result, args, argv, ctx, cl))
    return false;

  fm_frame_reserve(result, 1);

  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exec_cl = (csv_play_exec_cl *)*cl;

  auto *frames = fm_exec_ctx_frames((fm_exec_ctx *)ctx->exec);
  auto type = fm_frame_type(result);
  exec_cl->next = fm_frame_from_type(frames, type);
  fm_frame_reserve(exec_cl->next, 1);

  auto good = csv_parse_one(ctx, exec_cl, exec_cl->next);
  if (good < 1) {
    if (good < 0) {
      return false;
    }
    return true;
  }
  auto *info = (csv_play_info *)ctx->comp;
  auto idx_field = fm_frame_field(exec_cl->next, info->index.c_str());
  exec_cl->index = idx_field;
  auto next = *(fmc_time64_t *)fm_frame_get_ptr1(exec_cl->next, idx_field, 0);
  fm_stream_ctx_schedule(exec_ctx, ctx->handle, next);
  return true;
}

void fm_comp_csv_play_call_destroy(fm_call_exec_cl cl) {
  auto *exec_cl = (csv_play_exec_cl *)cl;
  if (exec_cl)
    delete exec_cl;
}

void fm_comp_csv_play_call_stream_destroy(fm_call_exec_cl cl) {
  fm_comp_csv_play_call_destroy(cl);
}

bool fm_comp_csv_play_stream_exec(fm_frame_t *result, size_t,
                                  const fm_frame_t *const argv[],
                                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exec_cl = (csv_play_exec_cl *)cl;

  auto prev =
      *(fmc_time64_t *)fm_frame_get_ptr1(exec_cl->next, exec_cl->index, 0);

  fm_frame_swap(result, exec_cl->next);

  auto good = csv_parse_one(ctx, exec_cl, exec_cl->next);
  if (good > 0) {
    auto next =
        *(fmc_time64_t *)fm_frame_get_ptr1(exec_cl->next, exec_cl->index, 0);
    if (fmc_time64_less(next, prev)) {
      csv_play_error_set(
          (fm_exec_ctx *)exec_ctx, (csv_play_info *)ctx->comp,
          (std::string(
               "next timestamp provided is lower than last timestamp in row ") +
           to_string(exec_cl->row) + ".")
              .c_str());
      return false;
    }
    fm_stream_ctx_schedule(exec_ctx, ctx->handle, next);
  } else if (good < 0) {
    return false;
  }

  return true;
}

fm_call_def *fm_comp_csv_play_stream_call(fm_comp_def_cl comp_cl,
                                          const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_csv_play_call_stream_init);
  fm_call_def_destroy_set(def, fm_comp_csv_play_call_stream_destroy);
  fm_call_def_exec_set(def, fm_comp_csv_play_stream_exec);
  return def;
}

/**
 * @brief generates CSV play operator
 *
 * The operator expects 3 parameters, file name
 * of the CSV file, the time index configuration tuple
 * and a tuple of configuration tuples for each field in
 * the frame. The configuration tuple consist of name
 * of the header, type of the field, and the format string.
 */
fm_ctx_def_t *fm_comp_csv_play_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                   unsigned argc, fm_type_decl_cp argv[],
                                   fm_type_decl_cp ptype,
                                   fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 0) {
    fm_type_sys_err_set(sys, FM_TYPE_ERROR_ARGS);
    return nullptr;
  }

  auto error = [&]() {
    const char *errstr = "expect a file name and a tuple of field descriptions;"
                         "first description must be an index field";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!ptype)
    return error();

  if (!fm_type_is_tuple(ptype))
    return error();
  if (fm_type_tuple_size(ptype) != 2)
    return error();

  auto *param1 = fm_type_tuple_arg(ptype, 0);
  if (!fm_type_is_cstring(param1))
    return error();
  const char *file = STACK_POP(plist, const char *);

  auto *row_descs = fm_type_tuple_arg(ptype, 1);
  if (!fm_type_is_tuple(row_descs))
    return error();
  auto size = fm_type_tuple_size(row_descs);

  auto *str_t = fm_cstring_type_get(sys);
  auto *type_t = fm_type_type_get(sys);
  auto *row_desc_t = fm_tuple_type_get(sys, 3, str_t, type_t, str_t);
  auto *row_desc2_t = fm_tuple_type_get(sys, 2, str_t, type_t);

  vector<csv_column_info> cols(size);
  vector<const char *> names(size);
  vector<fm_type_decl_cp> types(size);
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
    } else {
      const char *errstr = "each field description must contain field name, "
                           "field type and an optional format description";
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
      return nullptr;
    }

    if (!fm_type_is_simple(types[i])) {
      auto *typestr = fm_type_to_str(types[i]);
      auto errstr = string("expect simple type, got: ") + typestr;
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr.c_str());
      free(typestr);
      return nullptr;
    }
  }
  auto *type =
      fm_frame_type_get1(sys, size, names.data(), types.data(), 1, dims);
  if (!type)
    return error();

  string index = names[0];
  if (fm_type_base_enum(types[0]) != FM_TYPE_TIME64) {
    const char *errstr = "the first (index) field must be of type time64";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  auto *ctx_cl =
      new csv_play_info(sys, file, index, cols.data(), cols.data() + size);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_csv_play_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_csv_play_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (csv_play_info *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
