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
 * and "seq_ore_sim_split" operator
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
#include "seq_ore_sim_split.h"
#include "ytp/sequence.h"
#include "ytp/yamal.h"

#include "extractor/book/ore.hpp"
#include "extractor/book/updates.hpp"
#include "fmc++/mpl.hpp" // fmc_runtime_error_unless()
#include "fmc++/serialization.hpp"
#include "fmc++/time.hpp"
#include "fmc/platform.h"

#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>

struct sols_op_cl {
  std::string fname;
  std::unordered_map<std::string, int> ytp_channels;
  fm::book::ore::imnt_infos_t info;
  std::optional<int> affinity;

  std::string file_name(uint32_t idx) const {
    char str[6];
    snprintf(str, sizeof(str), ".%04u", idx);
    return fname + str;
  }
};

struct ch_ctx_t {
  ch_ctx_t(void *cl, int32_t index) : cl(cl), parser(imnts), index(index) {}
  void *cl;
  fm::book::ore::imnt_infos_t imnts;
  fm::book::ore::parser parser;
  fm::book::ore::imnt_info info;
  int32_t index;

  bool parse_one(cmp_mem_t &cmp, std::string &filename, fmc_error_t **error) {
    fmc_error_clear(error);
    auto res = parser.parse(&cmp.ctx, &info);
    if (res.is_success() || res.is_time()) {
      return true;
    } else if (res.is_announce()) {
      auto *msg = std::get_if<fm::book::updates::announce>(&parser.msg);
      info.index = index;
      info.px_denum = msg->px_tick;
      info.qty_denum = msg->qty_tick;
    } else if (!res.is_skip()) {
      fmc_error_set(error, "error reading FM Ore file %s: %s", filename.c_str(),
                    parser.get_error().c_str());
    }
    return false;
  }
};

struct sim_mode {
  sim_mode(sols_op_cl &cfg, std::atomic<uint32_t> &fidx)
      : cfg(cfg), fidx(fidx) {}

  inline void set_msg_time(uint64_t msg_time) {
    notify_time = fmc_time64_from_nanos(msg_time);
  }
  inline bool should_notify(fm_stream_ctx *exec_ctx) {
    return fmc_time64_greater_or_equal(fm_stream_ctx_now(exec_ctx),
                                       notify_time);
  }
  fmc_time64 next_schedule(fm_stream_ctx *exec_ctx) { return notify_time; }
  void swapped() { next_file_available = false; }
  inline bool wait_for_new_files() { return false; }
  inline bool should_poll_at_least_one() { return true; }

  bool is_next_file_available() {
    if (!next_file_available) {
      std::string next_file = cfg.file_name(fidx + 1);

      fmc_error_t *err;
      next_file_available = fmc_fexists(next_file.c_str(), &err);
      fmc_runtime_error_unless(!err)
          << "Unable to check if file " << cfg.fname
          << " exists, error message: " << fmc_error_msg(err);
    }
    return next_file_available;
  }

  sols_op_cl &cfg;
  std::atomic<uint32_t> &fidx;
  fmc_time64_t notify_time;
  bool next_file_available = false;
};

struct live_mode {
  live_mode(sols_op_cl &cfg, std::atomic<uint32_t> &fidx)
      : cfg(cfg), fidx(fidx) {
    thread = std::thread([&]() {
      fmc_error_t *err;
      if (cfg.affinity) {
        fmc_set_cur_affinity(*cfg.affinity, &err);
        fmc_runtime_error_unless(!err)
            << "could not set CPU affinity in seq_ore_live_split";
      }
      while (!thread_done) {
        if (!next_file_available) {
          std::string next_file = cfg.file_name(fidx + 1);
          next_file_available = fmc_fexists(next_file.c_str(), &err);
          fmc_runtime_error_unless(!err)
              << "Unable to check if file " << cfg.fname
              << " exists, error message: " << fmc_error_msg(err);
        }
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
      }
    });
  }
  ~live_mode() {
    if (thread.joinable()) {
      thread_done = true;
      thread.join();
    }
  }

  inline void set_msg_time(uint64_t msg_time) {}
  inline bool should_notify(fm_stream_ctx *exec_ctx) { return true; }
  fmc_time64 next_schedule(fm_stream_ctx *exec_ctx) {
    return fm_stream_ctx_now(exec_ctx);
  }
  void swapped() { next_file_available = false; }
  inline bool wait_for_new_files() { return true; }
  inline bool should_poll_at_least_one() { return false; }

  bool is_next_file_available() const { return next_file_available.load(); }

  sols_op_cl &cfg;
  std::atomic<uint32_t> &fidx;
  std::thread thread;
  std::atomic<bool> thread_done = false;
  std::atomic<bool> next_file_available = false;
};

template <typename mode_type> struct sols_exe_cl {
  ytp_sequence_t *seq = nullptr;
  cmp_mem_t cmp;
  fm::book::ore::imnt_infos_t imnts;
  std::unordered_map<std::string, std::unique_ptr<ch_ctx_t>> ch_cl;
  ch_ctx_t *current_ctx = nullptr;
  size_t data_size;
  sols_op_cl &cfg;
  fm_stream_ctx *exec_ctx;
  fm_call_ctx *call_ctx;
  fm_frame_t *fres;
  std::atomic<uint32_t> fidx = 1;
  mode_type mode;
  fmc_fd fd = -1;
  uint32_t peek_err_cnt = 0;
  bool next_file_exists = false;

  sols_exe_cl(sols_op_cl &op_cl)
      : cfg(op_cl), fidx(init_fidx()), mode(cfg, fidx) {
    fmc_error_t *err = nullptr;
    cmp_mem_init(&cmp);
    seq = seq_new(fidx, fd, &err);
    fmc_runtime_error_unless(!err)
        << "unable to initialize ytp sequence from file " << cfg.fname + ".0001"
        << ", error message: " << fmc_error_msg(err);
  }

  uint32_t init_fidx() {
    fmc_error_t *err = nullptr;
    // find first file
    uint32_t i = 1;
    for (; i < 10000; ++i) {
      std::string file_new = cfg.file_name(i);
      bool exists = fmc_fexists(file_new.c_str(), &err);
      fmc_runtime_error_unless(!err)
          << "Unable to check if file " << file_new
          << " exists, error message: " << fmc_error_msg(err);
      if (exists) {
        return i;
      }
    }
    fmc_runtime_error_unless(false)
        << "unable to find the first ytp sequence from file " << cfg.fname;
    return i;
  }

  ~sols_exe_cl() {
    fmc_error_t *err;
    if (seq) {
      ytp_sequence_del(seq, &err);
    }
    if (fmc_fvalid(fd)) {
      fmc_fclose(fd, &err);
    }
  }

  ytp_sequence_t *seq_new(uint32_t idx, fmc_fd &f, fmc_error_t **err) {
    std::string file_new = cfg.file_name(idx);
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
    reinterpret_cast<sols_exe_cl<mode_type> *>(closure)->ch_cb(
        std::string_view(name, sz), channel);
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
    auto &ctx = *reinterpret_cast<ch_ctx_t *>(closure);
    auto &self = *reinterpret_cast<sols_exe_cl<mode_type> *>(ctx.cl);
    self.data_cb(std::string_view(data, sz), time, ctx);
  }

  void data_cb(std::string_view data, uint64_t time, ch_ctx_t &ctx) {
    current_ctx = &ctx;
    data_size = data.size();
    cmp_mem_set(&cmp, data.size(), (void *)data.data());
    mode.set_msg_time(time);
  }

  bool proc_one(fmc_error_t **err) {
    fmc_error_clear(err);
    while (cmp.offset < cmp.size) {
      if (current_ctx->parse_one(cmp, cfg.fname, err)) {
        return true;
      }
      if (err) {
        return false;
      }
    }
    return false;
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

template <typename mode_type>
bool poll_one(sols_exe_cl<mode_type> *exe_cl, fm_call_ctx_t *ctx) {
  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *op_cl = (sols_op_cl *)ctx->comp;
  fmc_error_t *err;
  do {
    bool poll = ytp_sequence_poll(exe_cl->seq, &err);
    if (err) {
      fm_exec_ctx_error_set(
          ctx->exec, "Unable to poll the ytp sequence %s, error message: %s",
          op_cl->fname.c_str(), fmc_error_msg(err));
      return false;
    }
    if (exe_cl->current_ctx != nullptr) {
      return true;
    }
    if (!poll) {
      if (!exe_cl->next_file_exists) {
        exe_cl->next_file_exists = exe_cl->mode.is_next_file_available();
        if (!exe_cl->mode.wait_for_new_files() && !exe_cl->next_file_exists) {
          return false;
        }
      } else {
        bool swapped = exe_cl->swap_seq(&err);
        if (err) {
          fm_exec_ctx_error_set(
              ctx->exec,
              "Unable to peek the next ytp sequence %s, error message: %s",
              op_cl->fname.c_str(), fmc_error_msg(err));
          return false;
        }
        if (swapped) {
          exe_cl->next_file_exists = false;
          exe_cl->mode.swapped();
        }
      }
    } else if (exe_cl->mode.should_poll_at_least_one()) {
      continue;
    }

    fm_stream_ctx_schedule(exec_ctx, ctx->handle,
                           exe_cl->mode.next_schedule(exec_ctx));
    return false;
  } while (true);
}

template <typename mode_type>
bool fm_comp_seq_ore_split_call_stream_init(fm_frame_t *result, size_t args,
                                            const fm_frame_t *const argv[],
                                            fm_call_ctx_t *ctx,
                                            fm_call_exec_cl *cl) {
  sols_op_cl *op_cl = (sols_op_cl *)ctx->comp;
  try {
    auto exe_cl = std::make_unique<sols_exe_cl<mode_type>>(*op_cl);
    *(fm::book::message *)fm_frame_get_ptr1(result, 0, 0) =
        fm::book::message(fm::book::updates::none());
    auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
    if constexpr (std::is_same_v<mode_type, live_mode>) {
      fm_stream_ctx_queue(exec_ctx, ctx->handle);
    } else {
      if (poll_one(exe_cl.get(), ctx)) {
        fm_stream_ctx_schedule(exec_ctx, ctx->handle,
                               exe_cl->mode.next_schedule(exec_ctx));
      }
    }
    *cl = exe_cl.release();
  } catch (const std::exception &e) {
    fm_exec_ctx_error_set(ctx->exec, "%s", e.what());
    return false;
  }
  return true;
}

template <typename mode_type>
bool fm_comp_seq_ore_split_stream_exec(fm_frame_t *fres, size_t args,
                                       const fm_frame_t *const argv[],
                                       fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exe_cl = (sols_exe_cl<mode_type> *)cl;

  fmc_error_t *err = nullptr;
  auto proc_one = [&]() {
    if (!exe_cl->proc_one(&err)) {
      if (err) {
        fm_exec_ctx_error_set(exe_cl->call_ctx->exec, "%s", fmc_error_msg(err));
        return false;
      } else {
        exe_cl->current_ctx = nullptr;
        fm_stream_ctx_schedule(exec_ctx, ctx->handle,
                               exe_cl->mode.next_schedule(exec_ctx));
        return false;
      }
    }
    return true;
  };

  if (exe_cl->current_ctx == nullptr) {
    if (!poll_one(exe_cl, ctx)) {
      return false;
    }

    if (!proc_one()) {
      return false;
    }
  }

  auto &parser = exe_cl->current_ctx->parser;
  exe_cl->fres = fres;
  exe_cl->exec_ctx = exec_ctx;
  exe_cl->call_ctx = ctx;

  if (exe_cl->mode.should_notify(exec_ctx)) {
    std::visit(fmc::overloaded{[&](auto &m) {
                                 bool is_last =
                                     exe_cl->cmp.offset == exe_cl->data_size;
                                 m.batch = is_last ? m.batch : true;
                               },
                               [&](fm::book::updates::time &m) {},
                               [&](fm::book::updates::heartbeat &m) {},
                               [&](fm::book::updates::none &m) {}},
               parser.msg);
    auto &box = *(fm::book::message *)fm_frame_get_ptr1(fres, 0, 0);
    box = parser.msg;
    fm_stream_ctx_queue(exec_ctx, ctx->deps[exe_cl->current_ctx->index]);

    if (parser.expand) {
      parser.msg = parser.expanded;
      parser.expand = false;
    } else if (!proc_one()) {
      return false;
    }
  }

  fm_stream_ctx_schedule(exec_ctx, ctx->handle,
                         exe_cl->mode.next_schedule(exec_ctx));
  return false;
}

template <typename mode_type>
void fm_comp_seq_ore_split_stream_destroy(fm_call_exec_cl cl) {
  if (sols_exe_cl<mode_type> *exe_cl = (sols_exe_cl<mode_type> *)cl; exe_cl) {
    delete exe_cl;
  }
}

template <typename mode_type>
fm_call_def *fm_comp_seq_ore_split_stream_call(fm_comp_def_cl comp_cl,
                                               const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_seq_ore_split_call_stream_init<mode_type>);
  fm_call_def_exec_set(def, fm_comp_seq_ore_split_stream_exec<mode_type>);
  fm_call_def_destroy_set(def, fm_comp_seq_ore_split_stream_destroy<mode_type>);
  return def;
}

template <typename mode_type>
fm_ctx_def_t *
fm_comp_seq_ore_split_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
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
  if (arg_count < 2 || arg_count > 4) {
    return param_error();
  }
  if (has_time) {
    if (!fm_type_is_cstring(fm_type_tuple_arg(ptype, 1))) {
      return param_error();
    }
  }
  if (!fm_type_is_cstring(fm_type_tuple_arg(ptype, 0)) ||
      !fm_type_is_tuple(fm_type_tuple_arg(ptype, 1 + has_time))) {
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
    cl->affinity = affinity;
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
  fm_ctx_def_stream_call_set(def,
                             &fm_comp_seq_ore_split_stream_call<mode_type>);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

fm_ctx_def_t *
fm_comp_seq_ore_live_split_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                               unsigned argc, fm_type_decl_cp argv[],
                               fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  return fm_comp_seq_ore_split_gen<live_mode>(csys, closure, argc, argv, ptype,
                                              plist);
}

fm_ctx_def_t *
fm_comp_seq_ore_sim_split_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                              unsigned argc, fm_type_decl_cp argv[],
                              fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  return fm_comp_seq_ore_split_gen<sim_mode>(csys, closure, argc, argv, ptype,
                                             plist);
}

void fm_comp_seq_ore_split_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {

  if (auto *ctx_cl = (sols_op_cl *)fm_ctx_def_closure(def); ctx_cl) {
    delete ctx_cl;
  }
}
