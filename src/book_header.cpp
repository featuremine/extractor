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
 * @file book_header.cpp
 * @author Andres Rangel
 * @date 10 Jan 2020
 * @brief File contains C implementation of the book header operator
 *
 * This file contains implementation of the book header operator, which
 * take book updates as input and returns a frame that is updated with
 * the header information for each update.
 */

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "book/book.h"
#include "book_msg.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
}

#include "extractor/book/updates.hpp"
#include "extractor/time64.hpp"
#include "fmc++/mpl.hpp"

#include <string>
#include <variant>
#include <vector>

using namespace std;
using namespace fm;
using namespace book;

class header_op_cl {
public:
  header_op_cl(fm_type_decl_cp type) {
    receive_field_ = fm_type_frame_field_idx(type, "receive");
    vendor_field_ = fm_type_frame_field_idx(type, "vendor");
    seqn_field_ = fm_type_frame_field_idx(type, "seqn");
    batch_field_ = fm_type_frame_field_idx(type, "batch");
  }
  ~header_op_cl() {}
  void init(fm_frame_t *result) {
    *(fm_time64_t *)fm_frame_get_ptr1(result, receive_field_, 0) =
        fm_time64_start();
    *(fm_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fm_time64_start();
    *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = 0UL;
    *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = 0;
  }
  bool exec(const book::message &msg, fm_frame_t *result, fm_stream_ctx *ctx) {
    return std::visit(
        fmc::overloaded{
            [](const book::updates::announce &m) { return false; },
            [](const book::updates::time &m) { return false; },
            [](const book::updates::none &m) { return false; },
            [&](const auto &m) {
              *(fm_time64_t *)fm_frame_get_ptr1(result, receive_field_, 0) =
                  fm_stream_ctx_now(ctx);
              *(fm_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
                  m.vendor;
              *(uint64_t *)fm_frame_get_ptr1(result, seqn_field_, 0) = m.seqn;
              *(uint16_t *)fm_frame_get_ptr1(result, batch_field_, 0) = m.batch;
              return true;
            }},
        msg);
  }

  fm_field_t receive_field_, vendor_field_, seqn_field_, batch_field_;
};

bool fm_comp_book_header_call_stream_init(fm_frame_t *result, size_t args,
                                          const fm_frame_t *const argv[],
                                          fm_call_ctx_t *ctx,
                                          fm_call_exec_cl *cl) {
  auto &comp = (*(header_op_cl *)ctx->comp);
  comp.init(result);
  return true;
}

bool fm_comp_book_header_stream_exec(fm_frame_t *result, size_t args,
                                     const fm_frame_t *const argv[],
                                     fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &box = *(book::message *)fm_frame_get_cptr1(argv[0], 0, 0);
  auto &comp = (*(header_op_cl *)ctx->comp);
  return comp.exec(box, result, (fm_stream_ctx *)ctx->exec);
}

fm_call_def *fm_comp_book_header_stream_call(fm_comp_def_cl comp_cl,
                                             const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_book_header_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_book_header_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_book_header_gen(fm_comp_sys_t *csys,
                                      fm_comp_def_cl closure, unsigned argc,
                                      fm_type_decl_cp argv[],
                                      fm_type_decl_cp ptype,
                                      fm_arg_stack_t plist) {
  auto *sys = fm_type_sys_get(csys);
  auto rec_t =
      fm_record_type_get(sys, "fm::book::message", sizeof(book::message));
  auto in_type = fm_frame_type_get(sys, 1, 1, "update", rec_t, 1);
  if (!in_type) {
    return nullptr;
  }

  if (argc != 1 || !fm_type_equal(argv[0], in_type)) {
    auto *errstr = "expect book updates as input";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return nullptr;
  }

  if (!fm_args_empty(ptype)) {
    auto *errstr = "expect no arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  }

  const int nf = 4;
  const char *names[nf] = {"receive", "vendor", "seqn", "batch"};
  fm_type_decl_cp types[nf] = {fm_base_type_get(sys, FM_TYPE_TIME64),
                               fm_base_type_get(sys, FM_TYPE_TIME64),
                               fm_base_type_get(sys, FM_TYPE_UINT64),
                               fm_base_type_get(sys, FM_TYPE_UINT16)};
  int dims[1] = {1};
  auto type = fm_frame_type_get1(sys, nf, names, types, 1, dims);
  if (!type) {
    return nullptr;
  }

  header_op_cl *cl = new header_op_cl(type);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, (void *)cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_book_header_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_book_header_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (header_op_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
