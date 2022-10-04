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
 * @file frame_ytp_decode.cpp
 * @author Federico Ravchina
 * @date 7 Jan 2022
 * @brief File contains C++ definitions of the frame decode object
 *
 * This file contains declarations of the ytp record object
 * @see http://www.featuremine.com
 */

extern "C" {
#include "frame_ytp_decode.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include "mp_util.hpp"
#include "ytp.h"

#include "fmc++/memory.hpp"
#include "fmc/time.h"
#include "ytp/sequence.h"

#include <chrono>
#include <optional>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

struct frame_ytp_decode_cl {
  std::vector<fm_frame_reader_p> readers;
  ytp_sequence_shared_t *shared_seq;
  ytp_channel_t channel;
  fm_call_ctx_t *ctx;
  fm_frame_alloc_t *alloc;
  fm_frame_t *frame;
  uint64_t time;
  cmp_mem_t buffer;

  frame_ytp_decode_cl(std::vector<fm_frame_reader_p> readers,
                      ytp_sequence_shared_t *shared_seq, ytp_channel_t channel,
                      fm_type_decl_cp frame_type)
      : readers(std::move(readers)), shared_seq(shared_seq), channel(channel),
        alloc(fm_frame_alloc_new()),
        frame(fm_frame_from_type(alloc, frame_type)) {
    fmc_error_t *error;

    ytp_sequence_shared_inc(shared_seq);
    auto seq = ytp_sequence_shared_get(shared_seq);
    ytp_sequence_indx_cb(seq, channel, static_data_cb, this, &error);
  }

  ~frame_ytp_decode_cl() {
    fm_frame_alloc_del(alloc);
    fmc_error_t *error;
    auto seq = ytp_sequence_shared_get(shared_seq);
    ytp_sequence_indx_cb_rm(seq, channel, static_data_cb, this, &error);
    ytp_sequence_shared_dec(shared_seq, &error);
  }

  static void static_data_cb(void *closure, ytp_peer_t peer,
                             ytp_channel_t channel, uint64_t msg_time,
                             size_t sz, const char *data) {
    if (sz > 0 && data[sz - 1] == 'D') {
      reinterpret_cast<frame_ytp_decode_cl *>(closure)->data_cb(
          peer, msg_time, std::string_view(data, sz - 1));
    }
  }

  void data_cb(ytp_peer_t peer, uint64_t msg_time, std::string_view data) {
    cmp_mem_set(&buffer, data.size(), data.data());
    for (auto &reader : readers) {
      reader(buffer.ctx, frame, 0);
    }
    time = msg_time;
    fm_stream_ctx_queue((fm_stream_ctx *)ctx->exec, ctx->handle);
  }
};

bool fm_comp_frame_ytp_decode_call_stream_init(fm_frame_t *result, size_t args,
                                               const fm_frame_t *const argv[],
                                               fm_call_ctx_t *ctx,
                                               fm_call_exec_cl *cl) {
  auto &exec_cl = *(frame_ytp_decode_cl *)ctx->comp;

  cmp_mem_init(&exec_cl.buffer);

  exec_cl.ctx = ctx;

  auto &d = *(ytp_msg_decoded *)fm_frame_get_ptr1(result, 0, 0);
  d.time = 0;
  d.frame = exec_cl.frame;
  return true;
}

bool fm_comp_frame_ytp_decode_stream_exec(fm_frame_t *result, size_t,
                                          const fm_frame_t *const argv[],
                                          fm_call_ctx_t *ctx,
                                          fm_call_exec_cl cl) {
  auto &exec_cl = *(frame_ytp_decode_cl *)ctx->comp;

  auto &d = *(ytp_msg_decoded *)fm_frame_get_ptr1(result, 0, 0);
  d.time = exec_cl.time;
  d.frame = exec_cl.frame;

  return true;
}

fm_call_def *fm_comp_frame_ytp_decode_stream_call(fm_comp_def_cl comp_cl,
                                                  const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_frame_ytp_decode_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_frame_ytp_decode_stream_exec);
  return def;
}

fm_ctx_def_t *
fm_comp_frame_ytp_decode_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                             unsigned argc, fm_type_decl_cp argv[],
                             fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  if (argc != 0) {
    auto *errstr = "expect no operator arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  auto param_error = [&]() {
    auto *errstr = "expect a ytp channel object and optionally a timeout";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
  };

  if (!ptype || !fm_type_is_tuple(ptype) || fm_type_tuple_size(ptype) < 1 ||
      fm_type_tuple_size(ptype) > 2) {
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

  if (!fm_type_is_record(channel_arg) ||
      !fm_type_equal(ytp_rec_t, channel_arg)) {
    param_error();
    return nullptr;
  }

  auto stream = STACK_POP(plist, ytp_channel_wrapper);
  auto *shared_seq = stream.sequence;
  auto *seq = ytp_sequence_shared_get(shared_seq);

  fmc_error_t *error;
  std::optional<std::string_view> header_opt;

  int64_t timeout_expires = std::numeric_limits<int64_t>::max();
  if (fm_type_tuple_size(ptype) >= 2) {
    auto timeout_arg = fm_type_tuple_arg(ptype, 1);
    fm_time64_t timeout;
    if (!fm_arg_try_time64(timeout_arg, &plist, &timeout)) {
      param_error();
      return nullptr;
    }
    timeout_expires = fmc_cur_time_ns() + timeout.value;
  }

  {
    struct find_header_cl_t {
      fmc_error_t *&error;
      ytp_sequence_t *seq;
      ytp_channel_t channel;
      std::optional<std::string_view> &header_opt;
      ytp_iterator_t current_it;

      find_header_cl_t(fmc_error_t *&error, ytp_sequence_t *seq,
                       ytp_channel_t channel,
                       std::optional<std::string_view> &header_opt)
          : error(error), seq(seq), channel(channel), header_opt(header_opt) {
        current_it = ytp_sequence_get_it(seq);
        ytp_sequence_indx_cb(seq, channel, &find_header_cl_t::cb_static, this,
                             &error);
      }

      ~find_header_cl_t() {
        ytp_sequence_indx_cb_rm(seq, channel, &find_header_cl_t::cb_static,
                                this, &error);
        ytp_sequence_set_it(seq, current_it);
      }

      static void cb_static(void *closure, ytp_peer_t peer,
                            ytp_channel_t channel, uint64_t time, size_t sz,
                            const char *data) {
        static_cast<find_header_cl_t *>(closure)->cb(
            time, std::string_view(data, sz));
      }

      void cb(uint64_t time, std::string_view data) { header_opt = data; }

    } find_header_cl{
        error,
        seq,
        stream.channel,
        header_opt,
    };

    while (true) {
      auto poll = ytp_sequence_poll(find_header_cl.seq, &error);
      if (error) {
        auto errstr =
            std::string("unable to poll ytp object: ") + fmc_error_msg(error);
        fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN, errstr.c_str());
        return nullptr;
      }

      if (poll) {
        if (find_header_cl.header_opt.has_value()) {
          break;
        }
      } else {
        if (timeout_expires < fmc_cur_time_ns()) {
          auto *errstr = "header ytp message not found";
          fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN, errstr);
          return nullptr;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
      }
    }
  }

  auto &header = header_opt.value();

  if (header.empty() || header[header.size() - 1] != 'H') {
    auto *errstr = "channel not using frame encoding";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN, errstr);
    return nullptr;
  }

  auto type_str = header.substr(0, header.size() - 1);
  auto encoding_type = fm_type_from_str(sys, type_str.data(), type_str.size());

  if (!fm_type_is_frame(encoding_type)) {
    auto *errstr = "encoding type is not using a frame type";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN, errstr);
    return nullptr;
  }

  std::vector<fm_frame_reader_p> readers;
  auto nfields = fm_type_frame_nfields(encoding_type);
  for (auto i = 0u; i < nfields; ++i) {
    auto ftype = fm_type_frame_field_type(encoding_type, i);
    auto reader = fm_type_to_mp_reader(ftype, i);
    if (reader) {
      readers.push_back(reader);
    } else {
      fmc::autofree<char> field_type_ptr(fm_type_to_str(ftype));
      std::string type = field_type_ptr.get();
      auto errstr = std::string("unable to decode type: ") + type;
      fm_type_sys_err_custom(sys, FM_TYPE_ERROR_UNKNOWN, errstr.c_str());
      return nullptr;
    }
  }

  std::string msg_encoded_type_name = "ytp_msg_decoded(";
  msg_encoded_type_name.append(type_str.data(), type_str.size());
  msg_encoded_type_name += ')';

  auto msg_decoded_type = fm_record_type_get(sys, msg_encoded_type_name.c_str(),
                                             sizeof(ytp_msg_decoded));

  auto msg_decoded_frame_type =
      fm_frame_type_get(sys, 1, 1, "decoded", msg_decoded_type, 1);
  if (!msg_decoded_frame_type) {
    return nullptr;
  }

  auto *cl = new frame_ytp_decode_cl(std::move(readers), shared_seq,
                                     stream.channel, encoding_type);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_closure_set(def, (void *)cl);
  fm_ctx_def_type_set(def, msg_decoded_frame_type);
  fm_ctx_def_stream_call_set(def, &fm_comp_frame_ytp_decode_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_frame_ytp_decode_destroy(fm_comp_def_cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (frame_ytp_decode_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
