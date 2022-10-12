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
 * @file mp_play.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ definitions of the comp object
 *
 * This file contains definitions of the comp context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "mp_play.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
#include <cmp/cmp.h>
}

#include "errno.h"
#include "fmc++/counters.hpp"
#include "fmc++/mpl.hpp"
#include "mp_util.hpp"
#include <cassert>
#include <functional>
#include <stdlib.h>
#include <string>
#include <utility>
#include <vector>

#include <iostream>

#include "fmc++/strings.hpp"

using namespace std;

struct mp_column_info {
  string name;
  fm_type_decl_cp type;
  string format;
};

struct mp_play_info {
  template <class B, class E>
  mp_play_info(fm_type_sys_t *ts, const string &f, const string &i, B b, E e)
      : tsys(ts), file(f), index(i), columns(b, e) {}
  fm_type_sys_t *tsys;
  string file;
  string index;
  vector<mp_column_info> columns;
};

struct mp_play_exec_cl {
  using parser = vector<uint32_t>;
  mp_play_exec_cl(bool is_pipe = false) : is_pipe(is_pipe) {}
  ~mp_play_exec_cl() {
    auto file = (FILE *)cmp.buf;
    if (file) {
      if (is_pipe) {
        fmc_error_t *err = nullptr;
        fmc_pclose(file, &err);
      } else
        fclose(file);
    }
  }
  fm_field_t index;
  fm_frame_t *next;
  parser parsers;
  cmp_ctx_t cmp;
  bool is_pipe;
};

static bool read_bytes(void *data, size_t sz, FILE *fh) {
  return fread(data, sizeof(uint8_t), sz, fh) == (sz * sizeof(uint8_t));
}

static bool file_skipper(cmp_ctx_t *ctx, size_t count) {
  return fseek((FILE *)ctx->buf, count, SEEK_CUR);
}

static bool file_reader(cmp_ctx_t *ctx, void *data, size_t limit) {
  FMC_NANO_AVG(mp_play_reader_fread);

  return read_bytes(data, limit, (FILE *)ctx->buf);
}

static bool skip_parser(cmp_ctx_t &cmp, fm_frame_t *frame, int row) {
  cmp_object_t obj;
  return cmp_skip_object(&cmp, &obj);
}

static void add_column_parser(fm_type_sys_t *ts, fm_frame_t *frame,
                              mp_column_info *info,
                              mp_play_exec_cl::parser &parsers, bool old) {

  auto offset = fm_frame_field(frame, info->name.c_str());
  assert(fm_field_valid(offset));
  auto decl = info->type;

  if (fm_type_is_base(decl)) {
    uint32_t type = 0;
    switch (fm_type_base_enum(decl)) {
    case FM_TYPE_INT8:
      type = 1;
      break;
    case FM_TYPE_INT16:
      type = 2;
      break;
    case FM_TYPE_INT32:
      type = 3;
      break;
    case FM_TYPE_INT64:
      type = 4;
      break;
    case FM_TYPE_UINT8:
      type = 5;
      break;
    case FM_TYPE_UINT16:
      type = 6;
      break;
    case FM_TYPE_UINT32:
      type = 7;
      break;
    case FM_TYPE_UINT64:
      type = 8;
      break;
    case FM_TYPE_FLOAT32:
      type = 9;
      break;
    case FM_TYPE_FLOAT64:
      type = 10;
      break;
    case FM_TYPE_DECIMAL64:
      type = old ? 111 : 11;
      break;
    case FM_TYPE_TIME64:
      type = 12;
      break;
    case FM_TYPE_CHAR:
      type = 13;
      break;
    case FM_TYPE_WCHAR:
      type = 14;
      break;
    case FM_TYPE_BOOL:
      type = 15;
      break;
    case FM_TYPE_RATIONAL64:
      type = 16;
      break;
    case FM_TYPE_LAST:
    default:
      break;
    }
    parsers.push_back(type);
    parsers.push_back(offset);
    return;
  } else if (fm_type_is_array(decl) &&
             fm_type_is_base(fm_type_array_of(decl)) &&
             fm_type_base_enum(fm_type_array_of(decl)) == FM_TYPE_CHAR) {
    parsers.push_back(17);
    parsers.push_back(offset);
    parsers.push_back(fm_type_array_size(decl));
    return;
  }

  parsers.push_back(0);
}

int mp_parse_one(mp_play_exec_cl *cl, fm_frame_t *frame, int row) {
  FMC_NANO_AVG(mp_parse_one);

  int count = 0;
  bool success = false;

  for (uint32_t p_off = 0; p_off < cl->parsers.size();) {
    ++count;

    switch (cl->parsers[p_off]) {
    case 0:
      success = skip_parser(cl->cmp, frame, row);
      p_off += 1;
      break;
    case 1:
      success = msgpack_parser(
          cl->cmp,
          *(INT8 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 2:
      success = msgpack_parser(
          cl->cmp,
          *(INT16 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 3:
      success = msgpack_parser(
          cl->cmp,
          *(INT32 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 4:
      success = msgpack_parser(
          cl->cmp,
          *(INT64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 5:
      success = msgpack_parser(
          cl->cmp,
          *(UINT8 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 6:
      success = msgpack_parser(
          cl->cmp,
          *(UINT16 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 7:
      success = msgpack_parser(
          cl->cmp,
          *(UINT32 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 8:
      success = msgpack_parser(
          cl->cmp,
          *(UINT64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 9:
      success = msgpack_parser(
          cl->cmp,
          *(FLOAT32 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 10:
      success = msgpack_parser(
          cl->cmp,
          *(FLOAT64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 11:
      success = msgpack_parser(
          cl->cmp,
          *(DECIMAL64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 111: {
      fm_decimal64_t value;
      success = msgpack_parser(cl->cmp, value);
      *(DECIMAL64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row) =
          fm_decimal64_from_old(value);
      p_off += 2;
    } break;
    case 12:
      success = msgpack_parser(
          cl->cmp,
          *(TIME64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 13:
      success = msgpack_parser(
          cl->cmp,
          *(CHAR *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 14:
      success = msgpack_parser(
          cl->cmp,
          *(WCHAR *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 15:
      success = msgpack_parser(
          cl->cmp,
          *(bool *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 16:
      success = msgpack_parser(
          cl->cmp,
          *(RATIONAL64 *)fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row));
      p_off += 2;
      break;
    case 17: {
      uint32_t read_size = cl->parsers[p_off + 2];
      if (cmp_read_bin(&cl->cmp,
                       fm_frame_get_ptr1(frame, cl->parsers[p_off + 1], row),
                       &read_size))
        success = cl->parsers[p_off + 2] == read_size;
      p_off += 3;
    } break;
    default:
      break;
    }
  }

  if (!success) {
    if (feof((FILE *)cl->cmp.buf)) {
      if (cl->is_pipe) {
        fmc_error_t *err = nullptr;
        int pc = fmc_pclose((FILE *)cl->cmp.buf, &err);
        if (err)
          return 1;
        cl->cmp.buf = nullptr;
        if (pc != 0)
          return 1;
      }
      return 0;
    } else {
      return count;
    }
  }

  return -1;
}

bool fm_comp_mp_play_call_init(fm_frame_t *result, size_t args,
                               const fm_frame_t *const argv[],
                               fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  using namespace std;
  auto *info = (mp_play_info *)ctx->comp;
  auto *ts = info->tsys;

  FILE *file = nullptr;
  auto pipe = fmc::ends_with_pipe(info->file);
  const char *name = pipe.second.c_str();
  if (pipe.first) {
    fmc_error_t *err = nullptr;
    file = fmc_popen(name, "r", &err);
    if (err) {
      fm_exec_ctx_error_set(ctx->exec, "cannot execute %s: %s", name,
                            fmc_error_msg(err));
      return false;
    }
  } else {
    file = fopen(name, "r");
    if (!file) {
      fm_exec_ctx_error_set(ctx->exec, "cannot open file %s: %s", name,
                            strerror(errno));
      return false;
    }
  }

  auto *exec_cl = new mp_play_exec_cl(pipe.first);
  auto &cmp = exec_cl->cmp;
  cmp_init(&cmp, file, file_reader, file_skipper, nullptr);
  auto error = [&]() {
    delete exec_cl;
    return false;
  };

  auto &columns = info->columns;
  using column_use = pair<mp_column_info *, bool>;
  auto size = columns.size();
  vector<column_use> cols(size);
  unsigned i = 0;
  for (auto it = begin(columns); i < size; ++i, ++it) {
    cols[i].first = &(*it);
    cols[i].second = false;
  }

  auto header_error = [&](std::string err) {
    fm_exec_ctx_error_set(ctx->exec, "error (%s) cannot read header in %s",
                          err.c_str(), name);
    return error();
  };

  unsigned count = 0;

  std::string errmsg;
  vector<std::string> hdr_names;
  uint16_t ver[3] = {0};
  if (!fm::mp_util::read_header(&cmp, ver, hdr_names, errmsg)) {
    return header_error(errmsg);
  }

  bool old_version = ver[0] == 1 && ver[1] == 0 && ver[1] == 0;

  for (auto &hdr_name : hdr_names) {
    unsigned i = 0;
    for (; i < size; ++i) {
      if (hdr_name == cols[i].first->name)
        break;
    }
    if (i == size) {
      exec_cl->parsers.push_back(0);
    } else {
      auto &found = cols[i];
      if (found.second == true) {
        fm_exec_ctx_error_set(ctx->exec, "duplicate field %s in header",
                              string(hdr_name).c_str());
        return error();
      }
      add_column_parser(ts, result, found.first, exec_cl->parsers, old_version);
      found.second = true;
      ++count;
    }
  };

  if (count < size) {
    int i = 0;
    while (cols[i].second) {
      ++i;
    }
    fm_exec_ctx_error_set(ctx->exec, "mp header %s does not contain column %s",
                          name, cols[i].first->name.c_str());
    return error();
  }

  *cl = exec_cl;
  return true;
}

void mp_play_error_set(fm_exec_ctx *ctx, mp_play_exec_cl *cl, const char *name,
                       int col_idx) {
  if (cmp_strerror(&cl->cmp)) {
    if (errno != 0) {
      // No column is printed because return code is
      // forced to be 1 to generate the error.
      fm_exec_ctx_error_set(ctx,
                            "failed to read column %i running command %s "
                            "with parsing error %s and system error %s",
                            col_idx, name, cmp_strerror(&cl->cmp),
                            strerror(errno));
    } else {
      fm_exec_ctx_error_set(ctx,
                            "failed to read column %i of file %s "
                            "with parsing error %s",
                            col_idx, name, cmp_strerror(&cl->cmp));
    }
  } else {
    fm_exec_ctx_error_set(ctx, "failed to run command %s with system error %s",
                          name, strerror(errno));
  }
}

bool fm_comp_mp_play_call_stream_init(fm_frame_t *result, size_t args,
                                      const fm_frame_t *const argv[],
                                      fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  if (!fm_comp_mp_play_call_init(result, args, argv, ctx, cl))
    return false;

  fm_frame_reserve(result, 1);

  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exec_cl = (mp_play_exec_cl *)*cl;

  auto *frames = fm_exec_ctx_frames((fm_exec_ctx *)ctx->exec);
  auto type = fm_frame_type(result);
  exec_cl->next = fm_frame_from_type(frames, type);
  fm_frame_reserve(exec_cl->next, 1);

  auto res = mp_parse_one(exec_cl, exec_cl->next, 0);
  if (res >= 0) {
    if (res > 0) {
      auto *info = (mp_play_info *)ctx->comp;
      auto name = info->file.c_str();
      mp_play_error_set(ctx->exec, exec_cl, name, res);
      return false;
    }
    return true;
  }
  auto *info = (mp_play_info *)ctx->comp;
  auto idx_field = fm_frame_field(exec_cl->next, info->index.c_str());
  exec_cl->index = idx_field;
  auto next = *(fmc_time64_t *)fm_frame_get_ptr1(exec_cl->next, idx_field, 0);
  fm_stream_ctx_schedule(exec_ctx, ctx->handle, next);
  return true;
}

void fm_comp_mp_play_call_destroy(fm_call_exec_cl cl) {
  auto *exec_cl = (mp_play_exec_cl *)cl;
  if (exec_cl)
    delete exec_cl;
}

void fm_comp_mp_play_call_stream_destroy(fm_call_exec_cl cl) {
  fm_comp_mp_play_call_destroy(cl);
}

bool fm_comp_mp_play_stream_exec(fm_frame_t *result, size_t,
                                 const fm_frame_t *const argv[],
                                 fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  FMC_NANO_AVG(mp_play_stream_exec);

  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exec_cl = (mp_play_exec_cl *)cl;

  fm_frame_swap(result, exec_cl->next);

  auto res = mp_parse_one(exec_cl, exec_cl->next, 0);
  if (res < 0) {
    auto next =
        *(fmc_time64_t *)fm_frame_get_ptr1(exec_cl->next, exec_cl->index, 0);
    fm_stream_ctx_schedule(exec_ctx, ctx->handle, next);
  } else if (res > 0) {
    auto *info = (mp_play_info *)ctx->comp;
    auto name = info->file.c_str();
    mp_play_error_set(ctx->exec, exec_cl, name, res);
    return false;
  }

  return true;
}

fm_call_def *fm_comp_mp_play_stream_call(fm_comp_def_cl comp_cl,
                                         const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_mp_play_call_stream_init);
  fm_call_def_destroy_set(def, fm_comp_mp_play_call_stream_destroy);
  fm_call_def_exec_set(def, fm_comp_mp_play_stream_exec);
  return def;
}

/**
 * @brief generates MessagePack play operator
 *
 * The operator expects 3 parameters, file name of the
 * MessagePack encoded file, the time index configuration tuple
 * and a tuple of configuration tuples for each field in
 * the frame. The configuration tuple consist of name
 * of the header, type of the field, and the format string.
 */
fm_ctx_def_t *fm_comp_mp_play_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                                  unsigned argc, fm_type_decl_cp argv[],
                                  fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 0) {
    fm_type_sys_err_set(sys, FM_TYPE_ERROR_ARGS);
    return nullptr;
  }

  auto error = [&]() {
    const char *errstr = "expect a file name and time index field";
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

  vector<mp_column_info> cols(size);
  vector<const char *> names(size);
  vector<fm_type_decl_cp> types(size);
  int dims[1] = {1};

  for (unsigned i = 0; i < size; ++i) {
    auto *row_desc = fm_type_tuple_arg(row_descs, i);
    if (!fm_type_equal(row_desc, row_desc_t))
      return error();
    cols[i].name = names[i] = STACK_POP(plist, const char *);
    cols[i].type = types[i] = STACK_POP(plist, fm_type_decl_cp);
    cols[i].format = STACK_POP(plist, const char *);

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
      new mp_play_info(sys, file, index, cols.data(), cols.data() + size);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, ctx_cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_mp_play_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_mp_play_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (mp_play_info *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
