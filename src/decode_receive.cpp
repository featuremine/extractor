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
 * @file decode_receive.cpp
 * @author Federico Ravchina
 * @date 7 Jan 2022
 * @brief File contains C++ definitions of the frame decode object
 *
 * This file contains declarations of the ytp record object
 * @see http://www.featuremine.com
 */

extern "C" {
#include "decode_receive.h"
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

#include <array>
#include <chrono>
#include <optional>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <thread>
#include <unordered_map>

bool fm_comp_decode_receive_call_stream_init(fm_frame_t *result, size_t args,
                                             const fm_frame_t *const argv[],
                                             fm_call_ctx_t *ctx,
                                             fm_call_exec_cl *cl) {
  return true;
}

bool fm_comp_decode_receive_stream_exec(fm_frame_t *result, size_t,
                                        const fm_frame_t *const argv[],
                                        fm_call_ctx_t *ctx,
                                        fm_call_exec_cl cl) {
  auto &d = *(ytp_msg_decoded *)fm_frame_get_cptr1(argv[0], 0, 0);
  auto dest_f = fm_frame_get_ptr1(result, 0, 0);
  memcpy(dest_f, &d.time, sizeof(fm_time64_t));
  return true;
}

fm_call_def *fm_comp_decode_receive_stream_call(fm_comp_def_cl comp_cl,
                                                const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_decode_receive_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_decode_receive_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_decode_receive_gen(fm_comp_sys_t *csys,
                                         fm_comp_def_cl closure, unsigned argc,
                                         fm_type_decl_cp argv[],
                                         fm_type_decl_cp ptype,
                                         fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);

  auto arg_error = [&]() {
    auto *errstr = "expect a ytp decoded argument";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
  };

  if (argc != 1) {
    arg_error();
    return nullptr;
  }

  if (!fm_type_is_frame(argv[0])) {
    arg_error();
    return nullptr;
  }

  auto decoded_type_idx = fm_type_frame_field_idx(argv[0], "decoded");
  auto decoded_type = decoded_type_idx < 0
                          ? nullptr
                          : fm_type_frame_field_type(argv[0], decoded_type_idx);

  if (!fm_type_is_record(decoded_type)) {
    arg_error();
    return nullptr;
  }

  fmc::autofree<char> type_ptr(fm_type_to_str(decoded_type));
  std::string_view type_str = type_ptr.get();
  std::string_view record_prefix = "record(ytp_msg_decoded(";
  std::string record_suffix =
      ")," + std::to_string(sizeof(ytp_msg_decoded)) + ")";
  if (type_str.size() < record_prefix.size() + record_suffix.size() ||
      type_str.substr(0, record_prefix.size()) != record_prefix ||
      type_str.substr(type_str.size() - record_suffix.size()) !=
          record_suffix) {
    arg_error();
    return nullptr;
  }

  std::array<const char *, 1> names = {"time"};
  std::array<fm_type_decl_cp, 1> types = {
      fm_base_type_get(sys, FM_TYPE_TIME64)};

  std::array<int, 1> dims = {1};
  auto type = fm_frame_type_get1(sys, names.size(), names.data(), types.data(),
                                 dims.size(), dims.data());

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_stream_call_set(def, &fm_comp_decode_receive_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_decode_receive_destroy(fm_comp_def_cl, fm_ctx_def_t *def) {}
