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
 * @file frame_ytp_encode.cpp
 * @author Federico Ravchina
 * @date 7 Jan 2022
 * @brief File contains C++ definitions of the frame encode object
 *
 * This file contains declarations of the ytp record object
 * @see http://www.featuremine.com
 */

extern "C" {
#include "frame_ytp_encode.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "mp_util.hpp"
#include "ytp.h"

#include "fmc++/memory.hpp"
#include "fmc/time.h"
#include "ytp/api.h"

#include <optional>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

static ytp_sequence_api_v1 *ytp_; // ytp_api

struct frame_ytp_encode_cl {
  std::vector<fm_frame_writer_p> writers;
  shared_sequence *seq;
  ytp_peer_t peer;
  ytp_channel_t channel;
  cmp_str_t buffer;

  frame_ytp_encode_cl(std::vector<fm_frame_writer_p> writers,
                      shared_sequence *seq, ytp_peer_t peer,
                      ytp_channel_t channel)
      : writers(std::move(writers)), seq(seq), peer(peer), channel(channel) {
    ytp_->sequence_shared_inc(seq);
  }

  ~frame_ytp_encode_cl() {
    fmc_error_t *error;
    ytp_->sequence_shared_dec(seq, &error);
  }
};

bool fm_comp_frame_ytp_encode_call_stream_init(fm_frame_t *result, size_t args,
                                               const fm_frame_t *const argv[],
                                               fm_call_ctx_t *ctx,
                                               fm_call_exec_cl *cl) {
  auto &exec_cl = *(frame_ytp_encode_cl *)ctx->comp;

  cmp_str_init(&exec_cl.buffer);

  return true;
}

bool fm_comp_frame_ytp_encode_stream_exec(fm_frame_t *result, size_t,
                                          const fm_frame_t *const argv[],
                                          fm_call_ctx_t *ctx,
                                          fm_call_exec_cl cl) {
  auto &exec_cl = *(frame_ytp_encode_cl *)ctx->comp;

  cmp_str_reset(&exec_cl.buffer);

  for (auto &writer : exec_cl.writers) {
    writer(exec_cl.buffer.ctx, argv[0], 0);
  }

  std::string_view buffer{(char *)cmp_str_data(&exec_cl.buffer),
                          cmp_str_size(&exec_cl.buffer)};

  fmc_error_t *error;
  auto *ptr = ytp_->sequence_reserve(exec_cl.seq, buffer.size() + 1, &error);
  if (error) {
    auto errstr = std::string("unable to reserve in the sequence: ") +
                  fmc_error_msg(error);
    fm_exec_ctx_error_set(ctx->exec, errstr.c_str());
    return false;
  }

  memcpy(ptr, buffer.data(), buffer.size());
  ptr[buffer.size()] = 'D';

  ytp_->sequence_commit(exec_cl.seq, exec_cl.peer, exec_cl.channel,
                        fmc_cur_time_ns(), ptr, &error);
  if (error) {
    auto errstr = std::string("unable to commit in the sequence: ") +
                  fmc_error_msg(error);
    fm_exec_ctx_error_set(ctx->exec, errstr.c_str());
    return false;
  }

  return true;
}

fm_call_def *fm_comp_frame_ytp_encode_stream_call(fm_comp_def_cl comp_cl,
                                                  const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_frame_ytp_encode_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_frame_ytp_encode_stream_exec);
  return def;
}

fm_ctx_def_t *
fm_comp_frame_ytp_encode_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                             unsigned argc, fm_type_decl_cp argv[],
                             fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  ytp_ = get_ytp_api_v1();
  if (!ytp_) {
    auto *errstr = "ytp api is not set";
    fm_comp_sys_error_set(csys, errstr);
    return nullptr;
  }

  auto *sys = fm_type_sys_get(csys);
  if (argc != 1) {
    auto *errstr = "expected one operator";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto param_error = [&]() {
    auto *errstr = "expect a ytp stream object";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
  };

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) != 1) {
    param_error();
    return nullptr;
  }

  auto stream_arg = fm_type_tuple_arg(ptype, 0);
  if (!fm_type_is_record(stream_arg)) {
    param_error();
    return nullptr;
  }

  auto ytp_rec_t =
      fm_record_type_get(sys, "ytp_stream_wrapper", sizeof(ytp_stream_wrapper));

  if (!fm_type_equal(ytp_rec_t, stream_arg)) {
    param_error();
    return nullptr;
  }

  auto stream = STACK_POP(plist, ytp_stream_wrapper);

  std::vector<fm_frame_writer_p> writers;
  auto frame = argv[0];
  auto nfields = fm_type_frame_nfields(frame);
  for (auto i = 0u; i < nfields; ++i) {
    auto ftype = fm_type_frame_field_type(frame, i);
    auto writer = fm_type_to_mp_writer(ftype, i);
    if (writer) {
      writers.push_back(writer);
    } else {
      fmc::autofree<char> type_ptr(fm_type_to_str(ftype));
      std::string type = type_ptr.get();
      auto errstr = std::string("unable to encode type: ") + type;
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr.c_str());
      return nullptr;
    }
  }

  fmc_error_t *error;
  std::optional<std::string_view> header;

  struct find_header_cl_t {
    fmc_error_t *&error;
    shared_sequence *seq;
    ytp_peer_t peer;
    ytp_channel_t channel;
    std::optional<std::string_view> &header;
    ytp_iterator_t current_it;

    find_header_cl_t(fmc_error_t *&error, shared_sequence *seq, ytp_peer_t peer,
                     ytp_channel_t channel,
                     std::optional<std::string_view> &header)
        : error(error), seq(seq), peer(peer), channel(channel), header(header) {
      current_it = ytp_->sequence_get_it(seq);
      ytp_->sequence_indx_cb(seq, channel, &find_header_cl_t::cb_static, this,
                             &error);
    }

    ~find_header_cl_t() {
      ytp_->sequence_indx_cb_rm(seq, channel, &find_header_cl_t::cb_static,
                                this, &error);
      ytp_->sequence_set_it(seq, current_it);
    }

    static void cb_static(void *closure, ytp_peer_t peer, ytp_channel_t channel,
                          uint64_t time, size_t sz, const char *data) {
      static_cast<find_header_cl_t *>(closure)->cb(time,
                                                   std::string_view(data, sz));
    }

    void cb(uint64_t time, std::string_view data) { header = data; }

  } find_header_cl{
      error, stream.sequence, stream.peer, stream.channel, header,
  };

  while (true) {
    while (!error && !header.has_value() &&
           ytp_->sequence_poll(stream.sequence, &error))
      ;
    if (error) {
      auto errstr =
          std::string("unable to poll the sequence: ") + fmc_error_msg(error);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN, errstr.c_str());
      return nullptr;
    }

    fmc::autofree<char> type_ptr(fm_type_to_str(frame));
    std::string_view type = type_ptr.get();
    if (header.has_value()) {
      auto &header_str = header.value();
      if (header_str.empty() || header_str[header_str.size() - 1] != 'H' ||
          header_str.substr(0, header_str.size() - 1) != type) {
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN,
                               "channel already using a different frame type");
        return nullptr;
      }
      break;
    }

    auto *ptr =
        ytp_->sequence_reserve(stream.sequence, type.size() + 1, &error);
    if (error) {
      auto errstr =
          std::string("unable to reserve to sequence: ") + fmc_error_msg(error);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN, errstr.c_str());
      return nullptr;
    }

    memcpy(ptr, type.data(), type.size());
    ptr[type.size()] = 'H';

    ytp_->sequence_commit(stream.sequence, stream.peer, stream.channel,
                          fmc_cur_time_ns(), ptr, &error);
    if (error) {
      auto errstr =
          std::string("unable to commit to sequence: ") + fmc_error_msg(error);
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN, errstr.c_str());
      return nullptr;
    }
  }

  auto *cl = new frame_ytp_encode_cl(std::move(writers), stream.sequence,
                                     stream.peer, stream.channel);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, true);
  fm_ctx_def_closure_set(def, (void *)cl);
  fm_ctx_def_type_set(def, frame);
  fm_ctx_def_stream_call_set(def, &fm_comp_frame_ytp_encode_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_frame_ytp_encode_destroy(fm_comp_def_cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (frame_ytp_encode_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
