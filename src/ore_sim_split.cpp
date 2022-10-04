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
 * @file ore_sim_split.cpp
 * @authors Federico Ravchina
 * @date 01 Apr 2021
 * @brief File contains C++ definitions for the "ore_sim_split" operator
 *
 * @see http://www.featuremine.com
 * This operator is intendend to stream book updates for certain symbols
 */

extern "C" {
#include "ore_sim_split.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
#include <ytp/peer.h>
#include <ytp/yamal.h>
}

#include "extractor/book/ore.hpp"
#include "extractor/book/updates.hpp"
#include "extractor/decimal64.hpp"
#include "extractor/time64.hpp"
#include "fmc++/serialization.hpp"

#include <fcntl.h>
#include "fmc/platform.h"
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <unordered_map>
#include <variant>
#include <vector>

using namespace std;
using namespace fm;
using namespace book;

struct oss_op_cl {
  string fname;
  vector<string> symbols;
  bool time_msg;
};

struct oss_exe_cl {
  oss_exe_cl() : parser(imnts) { cmp_mem_init(&cmp); }
  ~oss_exe_cl() {
    if (mml) {
      fmc_error_t *err;
      ytp_yamal_del(mml, &err);
      // TODO: handle error
    }
    if (fmc_fvalid(fd)) {
      fmc_error_t *err = nullptr;
      fmc_fclose(fd, &err);
    }
  }
  ytp_yamal_t *mml = nullptr;
  ytp_iterator_t it;
  cmp_mem_t cmp;
  ore::imnt_infos_t imnts;
  ore::parser parser;
  unordered_map<string, int> symbols;
  optional<book::message> queued_msg;
  int32_t queued_imnt_index;
  fmc_fd fd;
  bool time_msg;

  void read_msg(fm_call_ctx_t *ctx) {
    if (parser.expand) {
      queued_msg = parser.expanded;
      queued_imnt_index = parser.imnt->index + time_msg;
      fm_stream_ctx_schedule((fm_stream_ctx *)ctx->exec, ctx->handle,
                             parser.time);
      parser.expand = false;
    } else {
      fmc_error_t *err = nullptr;
      while (!queued_msg.has_value() && !ytp_yamal_term(it)) {
        using ore::result;
        ytp_peer_t peer;
        size_t size;
        const char *buf;
        ytp_peer_read(mml, it, &peer, &size, &buf, &err);
        if (err)
          break;
        cmp_mem_set(&cmp, size, (void *)buf);
        auto res = parser.parse(&cmp.ctx);
        it = ytp_yamal_next(mml, it, &err);

        if (res.is_success()) {
          queued_msg = parser.msg;
          queued_imnt_index = parser.imnt->index + time_msg;
          fm_stream_ctx_schedule((fm_stream_ctx *)ctx->exec, ctx->handle,
                                 parser.time);
        } else if (res.is_time()) {
          if (time_msg) {
            queued_msg = parser.msg;
            queued_imnt_index = 0;
            fm_stream_ctx_schedule((fm_stream_ctx *)ctx->exec, ctx->handle,
                                   parser.time);
          }
        } else if (res.is_announce()) {
          auto *msg = std::get_if<updates::announce>(&parser.msg);
          auto where = symbols.find(msg->symbol);
          if (where != symbols.end()) {
            auto &info = imnts[msg->imnt_idx];
            info.index = where->second;
            info.px_denum = msg->tick;
            info.qty_denum = msg->qty_tick;
          }
        } else if (!res.is_skip()) {
          auto *op_cl = (oss_op_cl *)ctx->comp;
          fm_exec_ctx_error_set(
              ctx->exec, "error reading FM Ore file %s, format incorrect",
              op_cl->fname.c_str());
        }
      }
      if (err) {
        auto *op_cl = (oss_op_cl *)ctx->comp;
        fm_exec_ctx_error_set(ctx->exec,
                              "error reading FM Ore file %s, error message: %s",
                              op_cl->fname.c_str(), fmc_error_msg(err));
      }
    }
  }
};

bool fm_comp_ore_sim_split_call_stream_init(fm_frame_t *result, size_t args,
                                            const fm_frame_t *const argv[],
                                            fm_call_ctx_t *ctx,
                                            fm_call_exec_cl *cl) {
  using namespace fm;

  auto *op_cl = (oss_op_cl *)ctx->comp;
  auto exe_cl = make_unique<oss_exe_cl>();

  auto *fname = op_cl->fname.c_str();
  fmc_error_t *err = nullptr;
  exe_cl->fd = fmc_fopen(fname, fmc_fmode::READWRITE, &err);
  if (err) {
    fm_exec_ctx_error_set(ctx->exec, "cannot open file %s, with error %s",
                          fname, fmc_error_msg(err));
    return false;
  }
  if (!fmc_fvalid(exe_cl->fd)) {
    fm_exec_ctx_error_set(ctx->exec, "cannot open file %s", fname);
    return false;
  }
  exe_cl->mml = ytp_yamal_new(exe_cl->fd, &err);
  if (err) {
    fm_exec_ctx_error_set(
        ctx->exec,
        "cannot initialize yamal with provided file %s, with error %s", fname,
        fmc_error_msg(err));
    return false;
  }

  if (exe_cl->mml == nullptr) {
    fm_exec_ctx_error_set(ctx->exec, "unable to initialize mmlist");
    return false;
  }

  exe_cl->it = ytp_yamal_begin(exe_cl->mml, &err);
  if (err) {
    fm_exec_ctx_error_set(ctx->exec,
                          "cannot obtain yamal iterator, failed with error: %s",
                          fname, fmc_error_msg(err));
    return false;
  }

  uint64_t index = 0;
  for (auto symbol : op_cl->symbols) {
    exe_cl->symbols.emplace(symbol, index++);
  }

  exe_cl->time_msg = op_cl->time_msg;

  exe_cl->read_msg(ctx);

  *cl = exe_cl.release();
  return true;
}

bool fm_comp_ore_sim_split_stream_exec(fm_frame_t *fres, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exe_cl = (oss_exe_cl *)cl;

  using ore::result;

  if (exe_cl->queued_msg.has_value()) {
    auto &box = *(book::message *)fm_frame_get_ptr1(fres, 0, 0);
    box = *exe_cl->queued_msg;
    fm_stream_ctx_queue(exec_ctx, ctx->deps[exe_cl->queued_imnt_index]);
    exe_cl->queued_msg.reset();
  }

  exe_cl->read_msg(ctx);

  return false;
}

void fm_comp_ore_sim_split_stream_destroy(fm_call_exec_cl cl) {
  if (auto *exe_cl = (oss_exe_cl *)cl; exe_cl) {
    delete exe_cl;
  }
}

fm_call_def *fm_comp_ore_sim_split_stream_call(fm_comp_def_cl comp_cl,
                                               const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_ore_sim_split_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_ore_sim_split_stream_exec);
  fm_call_def_destroy_set(def, fm_comp_ore_sim_split_stream_destroy);
  return def;
}

fm_ctx_def_t *fm_comp_ore_sim_split_gen(fm_comp_sys_t *csys,
                                        fm_comp_def_cl closure, unsigned argc,
                                        fm_type_decl_cp argv[],
                                        fm_type_decl_cp ptype,
                                        fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 0) {
    auto *errstr = "expect no operator arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto param_error = [&]() {
    auto *errstr = "expect yamal file and a tuple of symbols as "
                   "parameters";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!fm_type_is_tuple(ptype)) {
    return param_error();
  }
  auto arg_count = fm_type_tuple_size(ptype);
  if (arg_count == 3) {
    if (!fm_type_is_bool(fm_type_tuple_arg(ptype, 2))) {
      return param_error();
    }
  } else if (arg_count != 2) {
    return param_error();
  }
  if (!fm_type_is_cstring(fm_type_tuple_arg(ptype, 0)) ||
      !fm_type_is_tuple(fm_type_tuple_arg(ptype, 1))) {
    return param_error();
  }

  auto cl = make_unique<oss_op_cl>();
  cl->fname = STACK_POP(plist, const char *);

  auto split_param = fm_type_tuple_arg(ptype, 1);
  unsigned split_count = fm_type_tuple_size(split_param);
  for (unsigned i = 0; i < split_count; ++i) {
    if (!fm_type_is_cstring(fm_type_tuple_arg(split_param, i))) {
      return param_error();
    }
    cl->symbols.emplace_back(STACK_POP(plist, const char *));
  }

  cl->time_msg = arg_count == 3 && STACK_POP(plist, BOOL);

  auto rec_t =
      fm_record_type_get(sys, "fm::book::message", sizeof(book::message));

  auto type = fm_frame_type_get(sys, 1, 1, "update", rec_t, 1);
  if (!type) {
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_volatile_set(def, split_count + cl->time_msg);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_ore_sim_split_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_ore_sim_split_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {

  if (auto *ctx_cl = (oss_op_cl *)fm_ctx_def_closure(def); ctx_cl) {
    delete ctx_cl;
  }
}
