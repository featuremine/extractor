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
 * @file seq_ore_live_split.cpp
 * @date 5 Oct 2022
 * @brief File contains C++ definitions for the "seq_ore_live_split" operator
 *
 * @see http://www.featuremine.com
 * This operator is intendend to stream book updates for certain ytp channels
 */

#include "seq_ore_live_split.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/process.h"
#include "fmc/time.h"
#include "ytp/peer.h"
#include "ytp/sequence.h"
#include "ytp/yamal.h"

#include "extractor/book/ore.hpp"
#include "extractor/book/updates.hpp"
#include "fmc++/mpl.hpp" // fmc_runtime_error_unless()
#include "fmc++/rprice.hpp"
#include "fmc++/serialization.hpp"
#include "fmc++/time.hpp"
#include "fmc/platform.h"

#include <algorithm>
#include <atomic>
#include <fcntl.h>
#include <memory>
#include <optional>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <thread>
#include <unordered_map>
#include <vector>

struct sols_op_cl {
  std::string fname;
  std::unordered_map<std::string, int> ytp_channels;
  fm::book::ore::imnt_infos_t info;
  std::optional<int> affinity;
};

struct ch_ctx_t {
  ch_ctx_t(struct sols_exe_cl *exe_cl, int32_t index)
      : exe_cl(exe_cl), parser(imnts), index(index) {}
  struct sols_exe_cl *exe_cl;
  fm::book::ore::imnt_infos_t imnts;
  fm::book::ore::parser parser;
  fm::book::ore::imnt_info info;
  int32_t index;
};

struct sols_exe_cl {
  ytp_sequence_t *seq = nullptr;
  cmp_mem_t cmp;
  fm::book::ore::imnt_infos_t imnts;
  std::unordered_map<std::string, std::unique_ptr<ch_ctx_t>> ch_cl;
  fm::book::ore::parser empty_parser;
  fm::book::ore::parser *current_parser;
  sols_op_cl &cfg;
  fm_stream_ctx *exec_ctx;
  fm_call_ctx *call_ctx;
  fm_frame_t *fres;
  std::thread thread;
  std::atomic<uint32_t> fidx = 1;
  fmc_fd fd = -1;
  uint32_t peek_err_cnt = 0;
  std::atomic<bool> thread_done = false;
  std::atomic<bool> next_file_available = false;
  bool next_file_exists = false;

  sols_exe_cl(sols_op_cl &op_cl)
      : empty_parser(imnts), current_parser(&empty_parser), cfg(op_cl) {
    fmc_error_t *err = nullptr;
    cmp_mem_init(&cmp);
    {
      // find first file
      uint32_t i = 1;
      for (; i < 10000; ++i) {
        std::string file_new = file_name(i);
        bool exists = fmc_fexists(file_new.c_str(), &err);
        fmc_runtime_error_unless(!err)
            << "Unable to check if file " << file_new
            << " exists, error message: " << fmc_error_msg(err);
        if (exists) {
          fidx = i;
          break;
        }
      }
      fmc_runtime_error_unless(i < 10000)
          << "unable to find the first ytp sequence from file " << cfg.fname;
    }
    seq = seq_new(fidx, fd, &err);
    fmc_runtime_error_unless(!err)
        << "unable to initialize ytp sequence from file " << cfg.fname + ".0001"
        << ", error message: " << fmc_error_msg(err);

    thread = std::thread([this]() {
      fmc_error_t *err;
      if (cfg.affinity) {
        fmc_runtime_error_unless(!err)
            << "could not set CPU affinity in seq_ore_live_split";
      }
      fmc_set_cur_affinity(*cfg.affinity, &err);
      while (!thread_done) {
        if (!next_file_available) {
          std::string next_file = file_name(fidx + 1);
          next_file_available = fmc_fexists(next_file.c_str(), &err);
          fmc_runtime_error_unless(!err)
              << "Unable to check if file " << cfg.fname
              << " exists, error message: " << fmc_error_msg(err);
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
      }
    });
  }

  ~sols_exe_cl() {
    fmc_error_t *err;
    if (seq) {
      ytp_sequence_del(seq, &err);
    }
    if (fmc_fvalid(fd)) {
      fmc_fclose(fd, &err);
    }
    if (thread.joinable()) {
      thread_done = true;
      thread.join();
    }
  }

  std::string file_name(uint32_t idx) {
    char str[6];
    snprintf(str, sizeof(str), ".%04u", idx);
    return cfg.fname + str;
  }

  ytp_sequence_t *seq_new(uint32_t idx, fmc_fd &f, fmc_error_t **err) {
    std::string file_new = file_name(idx);
    f = fmc_fopen(file_new.c_str(), fmc_fmode::READ, err);
    if (*err) {
      return nullptr;
    }
    ytp_sequence_t *s = ytp_sequence_new_2(f, false, err);
    if (*err) {
      return nullptr;
    }
    ytp_sequence_ch_cb(s, static_ch_cb, this, err);
    return s;
  }

  static void static_ch_cb(void *closure, ytp_peer_t peer,
                           ytp_channel_t channel, uint64_t time, size_t sz,
                           const char *name) {
    reinterpret_cast<sols_exe_cl *>(closure)->ch_cb(std::string_view(name, sz),
                                                    channel);
  }

  void ch_cb(std::string_view ch_name_sv, ytp_channel_t ch) {
    std::string ch_name(ch_name_sv.begin(), ch_name_sv.end());
    fmc_error_t *err = nullptr;
    if (auto where = cfg.ytp_channels.find(ch_name);
        where != cfg.ytp_channels.end()) {
      auto index = where->second;
      auto &cl = ch_cl[ch_name];
      if (!cl) {
        cl = std::make_unique<ch_ctx_t>(this, index);
      }
      ytp_sequence_indx_cb(seq, ch, static_data_cb, cl.get(), &err);
    }
  }

  static void static_data_cb(void *closure, ytp_peer_t peer,
                             ytp_channel_t channel, uint64_t time, size_t sz,
                             const char *data) {
    auto &ctx = *reinterpret_cast<
        decltype(ch_cl)::value_type::second_type::element_type *>(closure);
    ctx.exe_cl->data_cb(std::string_view(data, sz), &ctx.info, ctx.parser,
                        ctx.index);
  }

  void data_cb(std::string_view data, fm::book::ore::imnt_info *info,
               fm::book::ore::parser &parser, int32_t index) {
    cmp_mem_set(&cmp, data.size(), (void *)data.data());
    auto res = parser.parse(&cmp.ctx, info);
    if (res.is_success()) {
      auto &box = *(fm::book::message *)fm_frame_get_ptr1(fres, 0, 0);
      box = parser.msg;
      current_parser = &parser;
      fm_stream_ctx_queue(exec_ctx, call_ctx->deps[index]);
    } else if (res.is_time()) {
      auto &box = *(fm::book::message *)fm_frame_get_ptr1(fres, 0, 0);
      box = parser.msg;
      current_parser = &parser;
      fm_stream_ctx_queue(exec_ctx, call_ctx->deps[index]);
    } else if (res.is_announce()) {
      auto *msg = std::get_if<fm::book::updates::announce>(&parser.msg);
      info->index = index;
      info->px_denum = msg->tick;
      info->qty_denum = msg->qty_tick;
    } else if (!res.is_skip()) {
      fm_exec_ctx_error_set(call_ctx->exec,
                            "error reading FM Ore file %s, format incorrect",
                            cfg.fname.c_str());
    }
  }

  bool swap_seq(fmc_error_t **err) {
    if (fidx + 1 > 9999) {
      FMC_ERROR_REPORT(err, "Maximum number of ytp files exceeded.");
      return false;
    }
    fmc_fd next_fd = -1;
    ytp_sequence_t *next_seq = seq_new(fidx + 1, next_fd, err);
    if (*err) {
      // Ignore errors here because the writer could
      // not write the proper yamal file yet
      if (peek_err_cnt++ > 10000) {
        return false;
      }
      fmc_error_clear(err);
      if (fmc_fvalid(next_fd)) {
        fmc_fclose(next_fd, err);
        next_fd = -1;
      }
      return false;
    }
    peek_err_cnt = 0;

    // swap
    ytp_sequence_del(seq, err);
    if (*err) {
      return false;
    }
    fmc_fclose(fd, err);
    if (*err) {
      return false;
    }
    fidx++;
    seq = next_seq;
    fd = next_fd;
    return true;
  }
};

bool fm_comp_seq_ore_live_split_call_stream_init(fm_frame_t *result,
                                                 size_t args,
                                                 const fm_frame_t *const argv[],
                                                 fm_call_ctx_t *ctx,
                                                 fm_call_exec_cl *cl) {
  sols_op_cl *op_cl = (sols_op_cl *)ctx->comp;
  try {
    auto exe_cl = std::make_unique<sols_exe_cl>(*op_cl);
    *(fm::book::message *)fm_frame_get_ptr1(result, 0, 0) =
        fm::book::message(fm::book::updates::none());
    auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
    fm_stream_ctx_queue(exec_ctx, ctx->handle);
    *cl = exe_cl.release();
  } catch (const std::exception &e) {
    fm_exec_ctx_error_set(ctx->exec, "%s", e.what());
    return false;
  }
  return true;
}

bool fm_comp_seq_ore_live_split_stream_exec(fm_frame_t *fres, size_t args,
                                            const fm_frame_t *const argv[],
                                            fm_call_ctx_t *ctx,
                                            fm_call_exec_cl cl) {
  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exe_cl = (sols_exe_cl *)cl;
  auto *op_cl = (sols_op_cl *)ctx->comp;

  auto &parser = *exe_cl->current_parser;
  exe_cl->fres = fres;
  exe_cl->exec_ctx = exec_ctx;
  exe_cl->call_ctx = ctx;

  if (parser.expand) {
    parser.msg = parser.expanded;
    parser.expand = false;

    auto &box = *(fm::book::message *)fm_frame_get_ptr1(fres, 0, 0);
    box = parser.msg;
    fm_stream_ctx_queue(exec_ctx, ctx->deps[parser.imnt->index]);
  } else {
    fmc_error_t *err = nullptr;
    bool poll = ytp_sequence_poll(exe_cl->seq, &err);
    if (err) {
      fm_exec_ctx_error_set(
          ctx->exec, "Unable to poll the ytp sequence %s, error message: %s",
          op_cl->fname.c_str(), fmc_error_msg(err));
      return false;
    }
    if (!poll && !exe_cl->next_file_exists) {
      exe_cl->next_file_exists = exe_cl->next_file_available;
    } else if (!poll) { //  && exe_cl->next_file_exists
      bool swaped = exe_cl->swap_seq(&err);
      if (err) {
        fm_exec_ctx_error_set(
            ctx->exec,
            "Unable to peek the next ytp sequence %s, error message: %s",
            op_cl->fname.c_str(), fmc_error_msg(err));
        return false;
      }
      if (swaped) {
        exe_cl->next_file_exists = false;
        exe_cl->next_file_available = false;
      }
    }
  }

  fm_stream_ctx_schedule(exec_ctx, ctx->handle, fm_stream_ctx_now(exec_ctx));

  return false;
}

void fm_comp_seq_ore_live_split_stream_destroy(fm_call_exec_cl cl) {
  if (sols_exe_cl *exe_cl = (sols_exe_cl *)cl; exe_cl) {
    delete exe_cl;
  }
}

fm_call_def *
fm_comp_seq_ore_live_split_stream_call(fm_comp_def_cl comp_cl,
                                       const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_seq_ore_live_split_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_seq_ore_live_split_stream_exec);
  fm_call_def_destroy_set(def, fm_comp_seq_ore_live_split_stream_destroy);
  return def;
}

fm_ctx_def_t *
fm_comp_seq_ore_live_split_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                               unsigned argc, fm_type_decl_cp argv[],
                               fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 0) {
    auto *errstr = "expect no operator arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto param_error = [&]() {
    auto *errstr =
        "expect yamal file, optional time channel, a tuple of security "
        "channels, "
        "and an optional CPU affinity for the auxillary thread as parameters; "
        "you must specify time channel if you specify affinity";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!fm_type_is_tuple(ptype)) {
    return param_error();
  }
  auto arg_count = fm_type_tuple_size(ptype);
  bool has_time = arg_count >= 3;
  bool has_affinity = arg_count == 4;
  if (has_time) {
    if (!fm_type_is_cstring(fm_type_tuple_arg(ptype, 1))) {
      return param_error();
    }
  }
  if (!fm_type_is_cstring(fm_type_tuple_arg(ptype, 0)) ||
      !fm_type_is_tuple(fm_type_tuple_arg(ptype, 1 + has_time))) {
    return param_error();
  }
  if (arg_count < 2 || arg_count > 4) {
    return param_error();
  }
  auto cl = std::make_unique<sols_op_cl>();
  cl->fname = STACK_POP(plist, const char *);

  int idx_cnt = 0;
  if (has_time) {
    cl->ytp_channels.emplace(STACK_POP(plist, const char *), idx_cnt++);
  }

  auto split_param = fm_type_tuple_arg(ptype, 1 + has_time);
  unsigned split_count = fm_type_tuple_size(split_param);
  for (unsigned i = 0; i < split_count; ++i) {
    if (!fm_type_is_cstring(fm_type_tuple_arg(split_param, i))) {
      return param_error();
    }
    cl->ytp_channels.emplace(STACK_POP(plist, const char *), idx_cnt++);
  }

  if (has_affinity) {
    uint64_t affinity;
    if (!fm_arg_try_uinteger(fm_type_tuple_arg(ptype, 3), &plist, &affinity)) {
      return param_error();
    }
    *(cl->affinity) = affinity;
  }

  auto rec_t =
      fm_record_type_get(sys, "fm::book::message", sizeof(fm::book::message));

  auto type = fm_frame_type_get(sys, 1, 1, "update", rec_t, 1);
  if (!type) {
    return nullptr;
  }

  auto *def = fm_ctx_def_new();
  fm_ctx_def_volatile_set(def, split_count + has_time);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, cl.release());
  fm_ctx_def_stream_call_set(def, &fm_comp_seq_ore_live_split_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_seq_ore_live_split_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {

  if (auto *ctx_cl = (sols_op_cl *)fm_ctx_def_closure(def); ctx_cl) {
    delete ctx_cl;
  }
}
