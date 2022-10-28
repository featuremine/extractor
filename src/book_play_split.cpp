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
 * @file book_play_split.cpp
 * @authors Maxim Trokhimtchouk
 * @date 28 Oct 2018
 * @brief File contains C++ definitions for the "book_play_split" operator
 *
 * @see http://www.featuremine.com
 * This operator is intendend to replay book updates for certain symbols
 */

extern "C" {
#include "book_play_split.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "extractor/book/ore.hpp"
#include "extractor/book/updates.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/serialization.hpp"
#include "fmc++/time.hpp"

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;
using namespace fm;
using namespace book;

struct bps_op_cl {
  string fname;
  vector<string> symbols;
};

struct bps_exe_cl {
  bps_exe_cl() : parser(imnts) {}
  ~bps_exe_cl() { cmp_file_close(&cmp); }
  bool read_msg(fm_call_ctx_t *ctx, fm_call_exec_cl cl);
  cmp_file_t cmp;
  ore::imnt_infos_t imnts;
  ore::parser parser;
  bool update = false;
  unordered_map<string, int> symbols;
};

bool bps_exe_cl::read_msg(fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  using ore::result;
  result res;
  errno = 0;
  if (parser.expand) {
    parser.msg = parser.expanded;
    parser.expand = false;
    res = result::value_success();
  } else {
    res = parser.parse(&cmp.ctx);
  }
  if (res.is_continue()) {
    if (res.is_announce()) {
      auto *msg = std::get_if<updates::announce>(&parser.msg);
      auto *exe_cl = (bps_exe_cl *)cl;
      auto where = exe_cl->symbols.find(msg->symbol);
      if (where != exe_cl->symbols.end()) {
        auto &info = exe_cl->imnts[msg->imnt_idx];
        info.index = where->second;
        info.px_denum = msg->tick;
        info.qty_denum = msg->qty_tick;
      }
    }
    auto *stream_ctx = (fm_stream_ctx *)ctx->exec;
    fm_stream_ctx_schedule(stream_ctx, ctx->handle, parser.time);
  } else if (res.is_error()) {
    auto *op_cl = (bps_op_cl *)ctx->comp;
    if (errno) {
      fm_exec_ctx_error_set(ctx->exec,
                            "error reading FM Ore file %s, parsing error (%s) "
                            "and system error [%d](%s) occurred",
                            op_cl->fname.c_str(), parser.get_error().c_str(),
                            errno, strerror(errno));
    } else {
      fm_exec_ctx_error_set(
          ctx->exec,
          "error reading FM Ore file %s, parsing error (%s) occurred.",
          op_cl->fname.c_str(), parser.get_error().c_str());
    }
    return false;
  }
  update = res.is_success();
  return true;
}

bool fm_comp_book_play_split_call_stream_init(fm_frame_t *result, size_t args,
                                              const fm_frame_t *const argv[],
                                              fm_call_ctx_t *ctx,
                                              fm_call_exec_cl *cl) {
  using namespace fm;

  auto *op_cl = (bps_op_cl *)ctx->comp;
  auto exe_cl = make_unique<bps_exe_cl>();

  auto *fname = op_cl->fname.c_str();
  if (!cmp_file_init(&exe_cl->cmp, fname)) {
    auto *msg = fmc::ends_with_pipe(fname).first ? "cannot run command %s"
                                                 : "cannot open file %s";
    fm_exec_ctx_error_set(ctx->exec, msg, fname);
    return false;
  }

  auto *cmp_ctx = &exe_cl->cmp.ctx;
  uint16_t ver[3] = {0};
  if (!ore::read_version(cmp_ctx, ver)) {
    fm_exec_ctx_error_set(ctx->exec, "could not read file version");
    return false;
  }

  if (!ore::validate_version(ver)) {
    auto pver = ore::version;
    fm_exec_ctx_error_set(ctx->exec,
                          "FeatureMine Ore file version %d.%d.%d does not "
                          "match Ore parser version %d.%d.%d",
                          ver[0], ver[1], ver[0], pver[0], pver[1], pver[0]);
    return false;
  }

  ore::header hdr;
  if (!ore::read_hdr(cmp_ctx, hdr)) {
    fm_exec_ctx_error_set(ctx->exec, "could not read header of the file %s",
                          fname);
    return false;
  }
  uint64_t index = 0;
  for (auto symbol : op_cl->symbols) {
    auto where = hdr.find(symbol);
    if (where == hdr.end()) {
      fm_exec_ctx_error_set(ctx->exec,
                            "could find symbol %s in the "
                            "header of file %s",
                            symbol.c_str(), fname);
      return false;
    }
    auto &imnt = exe_cl->imnts[where->second.index];
    imnt.px_denum = where->second.px_denum;
    imnt.qty_denum = where->second.qty_denum;
    exe_cl->symbols[symbol] = index;
    imnt.index = index++;
  }

  *(book::message *)fm_frame_get_ptr1(result, 0, 0) =
      book::message(book::updates::none());
  if (!exe_cl->read_msg(ctx, cl)) {
    return false;
  }

  *cl = exe_cl.release();
  return true;
}

bool fm_comp_book_play_split_stream_exec(fm_frame_t *result, size_t args,
                                         const fm_frame_t *const argv[],
                                         fm_call_ctx_t *ctx,
                                         fm_call_exec_cl cl) {
  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exe_cl = (bps_exe_cl *)cl;

  if (exe_cl->update) {
    auto &box = *(book::message *)fm_frame_get_ptr1(result, 0, 0);
    box = exe_cl->parser.msg;
    fm_stream_ctx_queue(exec_ctx, ctx->deps[exe_cl->parser.imnt->index]);
  }

  exe_cl->read_msg(ctx, cl);
  return false;
}

void fm_comp_book_play_split_stream_destroy(fm_call_exec_cl cl) {
  if (auto *exe_cl = (bps_exe_cl *)cl; exe_cl) {
    delete exe_cl;
  }
}

fm_call_def *fm_comp_book_play_split_stream_call(fm_comp_def_cl comp_cl,
                                                 const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_book_play_split_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_book_play_split_stream_exec);
  fm_call_def_destroy_set(def, fm_comp_book_play_split_stream_destroy);
  return def;
}

fm_ctx_def_t *fm_comp_book_play_split_gen(fm_comp_sys_t *csys,
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
    auto *errstr = "expect a ore file and a tuple of symbols as "
                   "parameters";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  bool good_args = fm_type_is_tuple(ptype) && fm_type_tuple_size(ptype) == 2 &&
                   fm_type_is_cstring(fm_type_tuple_arg(ptype, 0)) &&
                   fm_type_is_tuple(fm_type_tuple_arg(ptype, 1));

  if (!good_args) {
    return param_error();
  }

  auto cl = make_unique<bps_op_cl>();
  cl->fname = STACK_POP(plist, const char *);

  auto split_param = fm_type_tuple_arg(ptype, 1);
  unsigned split_count = fm_type_tuple_size(split_param);
  for (unsigned i = 0; i < split_count; ++i) {
    if (!fm_type_is_cstring(fm_type_tuple_arg(split_param, i))) {
      return param_error();
    }
    cl->symbols.emplace_back(STACK_POP(plist, const char *));
  }

  auto rec_t =
      fm_record_type_get(sys, "fm::book::message", sizeof(book::message));

  auto type = fm_frame_type_get(sys, 1, 1, "update", rec_t, 1);
  if (!type) {
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_volatile_set(def, split_count);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_book_play_split_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_book_play_split_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {

  if (auto *ctx_cl = (bps_op_cl *)fm_ctx_def_closure(def); ctx_cl) {
    delete ctx_cl;
  }
}
