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
 * @file book_vendor_time.cpp
 * @author Maxim Trokhimtchouk
 * @date 4 Nov 2022
 * @brief File contains C implementation of the book header operator
 *
 * This file contains implementation of the book header operator, which
 * take book updates as input and returns a frame that is updated with
 * the header information for each update.
 */

extern "C" {
#include "book_msg.h"
#include "extractor/arg_stack.h"
#include "extractor/book/book.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "extractor/book/updates.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/time.hpp"

#include <string>
#include <variant>
#include <vector>

using namespace std;
using namespace fm;
using namespace book;

class book_vendor_time_op_cl {
public:
  book_vendor_time_op_cl(fm_type_decl_cp type) {
    vendor_field_ = fm_type_frame_field_idx(type, "vendor");
  }
  ~book_vendor_time_op_cl() {}
  void init(fm_frame_t *result) {
    *(fmc_time64_t *)fm_frame_get_ptr1(result, vendor_field_, 0) =
        fmc_time64_start();
  }
  bool exec(const book::message &msg, fm_frame_t *result, fm_stream_ctx *ctx) {
    return std::visit(
        fmc::overloaded{[](const book::updates::announce &m) { return false; },
                        [](const book::updates::time &m) { return false; },
                        [](const book::updates::none &m) { return false; },
                        [&](const book::updates::heartbeat &m) {
                          *(fmc_time64_t *)fm_frame_get_ptr1(
                              result, vendor_field_, 0) = m.vendor;
                          return true;
                        },
                        [&](const auto &m) {
                          if (m.batch)
                            return false;
                          *(fmc_time64_t *)fm_frame_get_ptr1(
                              result, vendor_field_, 0) = m.vendor;
                          return true;
                        }},
        msg);
  }

  fm_field_t vendor_field_;
};

bool fm_comp_book_vendor_time_call_stream_init(fm_frame_t *result, size_t args,
                                               const fm_frame_t *const argv[],
                                               fm_call_ctx_t *ctx,
                                               fm_call_exec_cl *cl) {
  auto &comp = (*(book_vendor_time_op_cl *)ctx->comp);
  comp.init(result);
  return true;
}

bool fm_comp_book_vendor_time_stream_exec(fm_frame_t *result, size_t args,
                                          const fm_frame_t *const argv[],
                                          fm_call_ctx_t *ctx,
                                          fm_call_exec_cl cl) {
  auto &box = *(book::message *)fm_frame_get_cptr1(argv[0], 0, 0);
  auto &comp = (*(book_vendor_time_op_cl *)ctx->comp);
  return comp.exec(box, result, (fm_stream_ctx *)ctx->exec);
}

fm_call_def *fm_comp_book_vendor_time_stream_call(fm_comp_def_cl comp_cl,
                                                  const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_book_vendor_time_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_book_vendor_time_stream_exec);
  return def;
}

fm_ctx_def_t *
fm_comp_book_vendor_time_gen(fm_comp_sys_t *csys, fm_comp_def_cl closure,
                             unsigned argc, fm_type_decl_cp argv[],
                             fm_type_decl_cp ptype, fm_arg_stack_t plist) {
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

  const int nf = 1;
  const char *names[nf] = {"vendor"};
  fm_type_decl_cp types[nf] = {fm_base_type_get(sys, FM_TYPE_TIME64)};
  int dims[1] = {1};
  auto type = fm_frame_type_get1(sys, nf, names, types, 1, dims);
  if (!type) {
    return nullptr;
  }

  book_vendor_time_op_cl *cl = new book_vendor_time_op_cl(type);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_closure_set(def, (void *)cl);
  fm_ctx_def_stream_call_set(def, &fm_comp_book_vendor_time_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_book_vendor_time_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (book_vendor_time_op_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
