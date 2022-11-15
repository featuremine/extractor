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
 * @file ore_ytp_decode.cpp
 * @author Ivan Gonzalez
 * @date 5 Jan 2022
 * @brief File contains C++ definitions of the ore ytp sequence decode object
 *
 * This file contains declarations of the ytp record object
 * @see http://www.featuremine.com
 */

extern "C" {
#include "ore_ytp_decode.h"
#include "extractor/arg_stack.h"  // fm_arg_stack_t
#include "extractor/comp_def.h"   // fm_ctx_def_cl
#include "extractor/comp_sys.h"   // fm_type_sys_get
#include "extractor/stream_ctx.h" // fm_stream_ctx_queue
#include "fmc/error.h"            // fmc_error_t
}

#include "extractor/book/ore.hpp"     // parser
#include "extractor/book/updates.hpp" // fm::book::updates::announce
#include "fmc++/serialization.hpp"
#include "ytp.h"     // ytp_channel_wrapper
#include "ytp/api.h" // ytp_sequence_t

#include <deque>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant> // get_if

static ytp_sequence_api_v1 *ytp_; // ytp_api

struct ore_ytp_decode_cl {
  fm_frame_alloc_t *alloc;
  fm_frame_t *frame;
  shared_sequence *shared_seq;
  ytp_channel_t channel;
  fm_stream_ctx *exec_ctx;
  fm_call_ctx *call_ctx;
  cmp_mem_t cmp;
  std::deque<std::pair<std::string_view, uint64_t>> queue;
  uint64_t last_time;

  fm::book::ore::imnt_infos_t imnts;
  fm::book::ore::parser parser;

  ore_ytp_decode_cl(fm_type_decl_cp type, shared_sequence *shared_seq,
                    ytp_channel_t channel)
      : alloc(fm_frame_alloc_new()), frame(fm_frame_from_type(alloc, type)),
        shared_seq(shared_seq), channel(channel), parser(imnts) {
    fmc_error_t *error;
    ytp_->sequence_shared_inc(shared_seq);
    ytp_->sequence_indx_cb(shared_seq, channel, static_data_cb, this, &error);
  }

  ~ore_ytp_decode_cl() {
    fm_frame_alloc_del(alloc);
    fmc_error_t *error;
    ytp_->sequence_indx_cb_rm(shared_seq, channel, static_data_cb, this,
                              &error);
    ytp_->sequence_shared_dec(shared_seq, &error);
  }

  static void static_data_cb(void *closure, ytp_peer_t peer,
                             ytp_channel_t channel, uint64_t time, size_t sz,
                             const char *data) {
    reinterpret_cast<ore_ytp_decode_cl *>(closure)->data_cb(
        std::string_view(data, sz), time);
  }

  void data_cb(std::string_view data, uint64_t time) {
    queue.push_front({data, time});
    fm_stream_ctx_queue(exec_ctx, call_ctx->handle);
  }
};

bool fm_comp_ore_ytp_decode_call_stream_init(fm_frame_t *result, size_t args,
                                             const fm_frame_t *const argv[],
                                             fm_call_ctx_t *ctx,
                                             fm_call_exec_cl *cl) {
  auto &exec_cl = *(ore_ytp_decode_cl *)ctx->comp;

  cmp_mem_init(&exec_cl.cmp);

  exec_cl.call_ctx = ctx;
  exec_cl.exec_ctx = (fm_stream_ctx *)ctx->exec;

  auto &d = *(ytp_msg_decoded *)fm_frame_get_ptr1(result, 0, 0);
  d.frame = exec_cl.frame;
  *(fm::book::message *)fm_frame_get_ptr1(d.frame, 0, 0) =
      fm::book::message(fm::book::updates::none());
  return true;
}

bool fm_comp_ore_ytp_decode_stream_exec(fm_frame_t *result, size_t args,
                                        const fm_frame_t *const argv[],
                                        fm_call_ctx_t *ctx,
                                        fm_call_exec_cl cl) {
  bool updated = false;
  auto *exec_ctx = (fm_stream_ctx *)ctx->exec;
  auto *exe_cl = (ore_ytp_decode_cl *)ctx->comp;

  auto &parser = exe_cl->parser;
  auto &cmp = exe_cl->cmp;
  auto &queue = exe_cl->queue;

  auto commit_msg = [&]() {
    auto &d = *(ytp_msg_decoded *)fm_frame_get_ptr1(result, 0, 0);
    auto &box = *(fm::book::message *)fm_frame_get_ptr1(d.frame, 0, 0);
    box = parser.msg;
    d.time = exe_cl->last_time;
    updated = true;
  };

  if (parser.expand) {
    parser.msg = parser.expanded;
    parser.expand = false;
    commit_msg();
  } else {
    auto ytpmsg = queue.back();
    std::string_view data = ytpmsg.first;
    exe_cl->last_time = ytpmsg.second;
    cmp_mem_set(&cmp, data.size(), data.data());
    fm::book::ore::result res = parser.parse(&cmp.ctx);
    queue.pop_back();
    if (res.is_success()) {
      commit_msg();
    } else if (res.is_announce()) {
      auto *msg = std::get_if<fm::book::updates::announce>(&parser.msg);
      exe_cl->imnts.try_emplace(msg->imnt_idx,
                                fm::book::ore::imnt_info{
                                    .px_denum = msg->tick,
                                    .qty_denum = msg->qty_tick,
                                    .index = (int32_t)msg->imnt_idx
                                    // .orders;
                                });
    } else if (!res.is_skip()) {
      fm_exec_ctx_error_set(ctx->exec, "error reading ytp channel: %s",
                            parser.error.c_str());
      return false;
    }
  }

  if (!queue.empty() || parser.expand) {
    fm_stream_ctx_schedule(exec_ctx, ctx->handle, fm_stream_ctx_now(exec_ctx));
  }

  return updated;
}

fm_call_def *fm_comp_ore_ytp_decode_stream_call(fm_comp_def_cl comp_cl,
                                                const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_ore_ytp_decode_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_ore_ytp_decode_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_ore_ytp_decode_gen(fm_comp_sys_t *csys,
                                         fm_comp_def_cl closure, unsigned argc,
                                         fm_type_decl_cp argv[],
                                         fm_type_decl_cp ptype,
                                         fm_arg_stack_t plist) {
  ytp_ = get_ytp_api_v1();
  if (!ytp_) {
    auto *errstr = "ytp api is not set";
    fm_comp_sys_error_set(csys, errstr);
    return nullptr;
  }

  auto *sys = fm_type_sys_get(csys);
  if (argc != 0) {
    auto *errstr = "expect no operator arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto param_error = [&]() {
    auto *errstr = "expect a ytp channel object";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
  };

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    param_error();
    return nullptr;
  }

  auto channel_arg = fm_type_tuple_arg(ptype, 0);
  if (!fm_type_is_record(channel_arg)) {
    param_error();
    return nullptr;
  }

  auto ytp_rec_t = fm_record_type_get(sys, "ytp_channel_wrapper",
                                      sizeof(ytp_channel_wrapper));

  if (!fm_type_equal(ytp_rec_t, channel_arg)) {
    param_error();
    return nullptr;
  }

  auto stream = STACK_POP(plist, ytp_channel_wrapper);
  auto *shared_seq = stream.sequence;

  auto rec_t =
      fm_record_type_get(sys, "fm::book::message", sizeof(fm::book::message));

  auto book_msg_ftype = fm_frame_type_get(sys, 1, 1, "update", rec_t, 1);
  if (!book_msg_ftype) {
    return nullptr;
  }

  std::string msg_encoded_type_name = "ytp_msg_decoded(";
  msg_encoded_type_name += fm_type_to_str(book_msg_ftype);
  msg_encoded_type_name += ')';

  auto msg_decoded_type = fm_record_type_get(sys, msg_encoded_type_name.c_str(),
                                             sizeof(ytp_msg_decoded));

  auto msg_decoded_frame_type =
      fm_frame_type_get(sys, 1, 1, "decoded", msg_decoded_type, 1);
  if (!msg_decoded_frame_type) {
    return nullptr;
  }

  auto *cl = new ore_ytp_decode_cl(book_msg_ftype, shared_seq, stream.channel);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_closure_set(def, (void *)cl);
  fm_ctx_def_type_set(def, msg_decoded_frame_type);
  fm_ctx_def_stream_call_set(def, &fm_comp_ore_ytp_decode_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_ore_ytp_decode_destroy(fm_comp_def_cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (ore_ytp_decode_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
