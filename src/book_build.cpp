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
 * @file book_build.cpp
 * @author Maxim Trokhimtchouk
 * @date 31 Oct 2018
 * @brief File contains C implementation of the book build operator
 *
 * This file contains implementation of the book build operator, which
 * take book updates as input and returns frame of book levels
 * @see http://www.featuremine.com
 */

extern "C" {
#include "book_build.h"
#include "book/book.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "extractor/book/updates.hpp"
#include "fmc++/decimal128.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/time.hpp"

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;
using namespace fm;
using namespace book;

struct bb_exe_cl {
  bb_exe_cl(fm_book_shared_t *book) : book(book ? book : fm_book_shared_new()) {
    if (book) {
      fm_book_shared_inc(book);
    }
  }
  ~bb_exe_cl() { fm_book_shared_dec(book); }
  fm_book_shared_t *book;
  unsigned lvl_cnt = 0;
  vector<fm_field_t> fields;
};

bool fm_comp_book_build_call_stream_init(fm_frame_t *result, size_t args,
                                         const fm_frame_t *const argv[],
                                         fm_call_ctx_t *ctx,
                                         fm_call_exec_cl *cl) {
  auto exe_cl = (bb_exe_cl *)ctx->comp;
  ;
  auto frame_t = fm_frame_type(result);
  exe_cl->lvl_cnt = fm_type_frame_nfields(frame_t) / 6;
  auto &fields = exe_cl->fields;
  char buf[32] = {0};
  for (unsigned i = 0; i < exe_cl->lvl_cnt; ++i) {
    sprintf(buf, "bid_prx_%u", i);
    fields.push_back(fm_frame_field(result, buf));
    *(fmc_decimal128_t *)fm_frame_get_ptr1(result, fields.back(), 0) = fmc::decimal128(0);

    sprintf(buf, "bid_shr_%u", i);
    fields.push_back(fm_frame_field(result, buf));
    *(fmc_decimal128_t *)fm_frame_get_ptr1(result, fields.back(), 0) = fmc::decimal128(0);

    sprintf(buf, "bid_ord_%u", i);
    fields.push_back(fm_frame_field(result, buf));
    *(uint32_t *)fm_frame_get_ptr1(result, fields.back(), 0) = 0;
  }
  for (unsigned i = 0; i < exe_cl->lvl_cnt; ++i) {
    sprintf(buf, "ask_prx_%u", i);
    fields.push_back(fm_frame_field(result, buf));
    *(fmc_decimal128_t *)fm_frame_get_ptr1(result, fields.back(), 0) = fmc::decimal128(0);

    sprintf(buf, "ask_shr_%u", i);
    fields.push_back(fm_frame_field(result, buf));
    *(fmc_decimal128_t *)fm_frame_get_ptr1(result, fields.back(), 0) = fmc::decimal128(0);

    sprintf(buf, "ask_ord_%u", i);
    fields.push_back(fm_frame_field(result, buf));
    *(uint32_t *)fm_frame_get_ptr1(result, fields.back(), 0) = 0;
  }
  return true;
}

bool fm_comp_book_build_stream_exec(fm_frame_t *result, size_t args,
                                    const fm_frame_t *const argv[],
                                    fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *exe_ctx = (fm_stream_ctx *)ctx->exec;
  auto now = fm_stream_ctx_now(exe_ctx);
  auto exe_cl = (bb_exe_cl *)ctx->comp;
  auto *inst = fm_book_shared_get(exe_cl->book);
  auto &box = *(book::message *)fm_frame_get_cptr1(argv[0], 0, 0);
  //    bool show = false;
  //
  //    if (show && box) {
  //        std::visit(overloaded {
  //            [inst, now](const book::updates::add &msg) {
  //                cout << "add" << ',' << setprecision(15)
  //                     << msg.vendor << ','
  //                     << msg.seqn << ','
  //                     << msg.id << ','
  //                     << fmc_decimal128_to_double(msg.price) << ','
  //                     << msg.qty << ','
  //                     << msg.is_bid << ','
  //                     << msg.batch << endl;
  //            },
  //            [inst, now](const book::updates::insert &msg) {
  //                cout << "insert" << ',' << setprecision(15)
  //                     << msg.vendor << ','
  //                     << msg.seqn << ','
  //                     << msg.id << ','
  //                     << msg.prio << ','
  //                     << fmc_decimal128_to_double(msg.price) << ','
  //                     << msg.qty << ','
  //                     << msg.is_bid << ','
  //                     << msg.batch << endl;
  //            },
  //            [inst, now](const book::updates::cancel &msg) {
  //                cout << "cancel" << ',' << setprecision(15)
  //                     << msg.vendor << ','
  //                     << msg.seqn << ','
  //                     << msg.id << ','
  //                     << fmc_decimal128_to_double(msg.price) << ','
  //                     << msg.qty << ','
  //                     << msg.is_bid << ','
  //                     << msg.batch << endl;
  //            },
  //            [inst, now](const book::updates::execute &msg) {
  //                cout << "execute" << ',' << setprecision(15)
  //                     << msg.vendor << ','
  //                     << msg.seqn << ','
  //                     << msg.id << ','
  //                     << fmc_decimal128_to_double(msg.price) << ','
  //                     << fmc_decimal128_to_double(msg.trade_price) << ','
  //                     << msg.qty << ','
  //                     << msg.is_bid << ','
  //                     << msg.batch << endl;
  //            },
  //            [](const book::updates::trade &msg) {
  //                cout << "trade" << ',' << setprecision(15)
  //                     << msg.vendor << ','
  //                     << msg.seqn << ','
  //                     << fmc_decimal128_to_double(msg.trade_price) << ','
  //                     << msg.qty << ','
  //                     << msg.batch << endl;
  //            },
  //        }, *box);
  //    } else if (show) {
  //        cout << "no message\n";
  //    }
  bool update = true;
  if (!std::holds_alternative<book::updates::none>(box)) {
    std::visit(
        fmc::overloaded{[&](const auto &msg) { update = msg.batch != 1; },
                        [&](const book::updates::time &msg) {},
                        [&](const book::updates::none &msg) {}},
        box);
    if (!std::visit(
            fmc::overloaded{
                [inst, now](const book::updates::add &msg) {
                  fm_book_add(inst, now, msg.vendor, msg.seqn, msg.id,
                              msg.price, msg.qty, msg.is_bid);
                  return true;
                },
                [inst, now](const book::updates::insert &msg) {
                  fm_book_ins(inst, now, msg.vendor, msg.seqn, msg.id, msg.prio,
                              msg.price, msg.qty, msg.is_bid);
                  return true;
                },
                [inst, now](const book::updates::position &msg) {
                  fm_book_pos(inst, now, msg.vendor, msg.seqn, msg.id, msg.pos,
                              msg.price, msg.qty, msg.is_bid);
                  return true;
                },
                [inst](const book::updates::cancel &msg) {
                  return fm_book_mod(inst, msg.id, msg.price, msg.qty,
                                     msg.is_bid);
                },
                [inst](const book::updates::execute &msg) {
                  return fm_book_exe(inst, msg.id, msg.price, msg.qty,
                                     msg.is_bid);
                },
                [](const book::updates::trade &msg) { return false; },
                [](const book::updates::state &msg) { return false; },
                [inst](const book::updates::control &msg) {
                  fm_book_uncross_set(inst, msg.uncross);
                  if (msg.command == 'C') {
                    fm_book_clr(inst);
                  }
                  return true;
                },
                [inst, now](const book::updates::set &msg) {
                  fm_book_pla(inst, now, msg.vendor, msg.seqn, msg.price,
                              msg.qty, msg.is_bid);
                  return true;
                },
                [](const book::updates::announce &msg) { return false; },
                [](const book::updates::time &msg) { return false; },
                [](const book::updates::none &msg) { return false; },
            },
            box)) {
      return false;
    }
  }
  if (!update) {
    return false;
  }
  auto it = exe_cl->fields.begin();
  auto lvl_cnt = exe_cl->lvl_cnt;
  for (unsigned side = 0; side < 2; ++side) {
    auto *levels = fm_book_levels(inst, !side);
    unsigned size = fm_book_levels_size(levels);
    unsigned idx = 0;
    for (; idx < size && idx < lvl_cnt; ++idx) {
      auto *level = fm_book_level(levels, idx);
      *(fmc_decimal128_t *)fm_frame_get_ptr1(result, *(it++), 0) =
          fm_book_level_prx(level);
      *(fmc_decimal128_t *)fm_frame_get_ptr1(result, *(it++), 0) =
          fm_book_level_shr(level);
      *(uint32_t *)fm_frame_get_ptr1(result, *(it++), 0) =
          fm_book_level_ord(level);
    }
    for (; idx < lvl_cnt; ++idx) {
      *(fmc_decimal128_t *)fm_frame_get_ptr1(result, *(it++), 0) = fmc::decimal128(0);
      *(fmc_decimal128_t *)fm_frame_get_ptr1(result, *(it++), 0) = fmc::decimal128(0);
      *(uint32_t *)fm_frame_get_ptr1(result, *(it++), 0) = 0;
    }
  }
  return true;
}

fm_call_def *fm_comp_book_build_stream_call(fm_comp_def_cl comp_cl,
                                            const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_book_build_call_stream_init);
  fm_call_def_exec_set(def, fm_comp_book_build_stream_exec);
  return def;
}

fm_ctx_def_t *fm_comp_book_build_gen(fm_comp_sys_t *csys,
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

  auto param_error = [&]() {
    auto *errstr = "expect an optional number of book levels and/or a python "
                   "book object as arguments";
    fm_type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return nullptr;
  };

  if (!fm_type_is_tuple(ptype)) {
    return param_error();
  }
  uint64_t lvl_cnt = 5;

  auto book_rec_t =
      fm_record_type_get(sys, "fm_book_shared_t*", sizeof(fm_book_shared_t *));

  fm_book_shared_t *book = nullptr;
  bool lvl_found = false;
  auto psz = fm_type_tuple_size(ptype);

  if (psz > 2) {
    return param_error();
  }

  auto proc_arg = [&](unsigned i) {
    auto param = fm_type_tuple_arg(ptype, i);
    if (fm_type_is_record(param)) {
      if (book || !fm_type_equal(book_rec_t, param)) {
        return false;
      }
      book = STACK_POP(plist, fm_book_shared_t *);
    } else {
      if (lvl_found || !fm_arg_try_uinteger(param, &plist, &lvl_cnt)) {
        return false;
      }
      lvl_found = true;
    }
    return true;
  };

  for (auto i = 0U; i < psz; ++i) {
    if (!proc_arg(i)) {
      return param_error();
    }
  }

  unsigned nf = 6 * lvl_cnt;
  vector<fm_type_decl_cp> types(nf);
  vector<string> buf(nf, string(32, '\0'));
  int dims[1];
  dims[0] = 1;

  unsigned idx = 0;
  for (unsigned i = 0; i < lvl_cnt; ++i) {
    types[idx] = fm_base_type_get(sys, FM_TYPE_DECIMAL128);
    sprintf(buf[idx++].data(), "bid_prx_%u", i);
    types[idx] = fm_base_type_get(sys, FM_TYPE_DECIMAL128);
    sprintf(buf[idx++].data(), "bid_shr_%u", i);
    types[idx] = fm_base_type_get(sys, FM_TYPE_UINT32);
    sprintf(buf[idx++].data(), "bid_ord_%u", i);
  }
  for (unsigned i = 0; i < lvl_cnt; ++i) {
    types[idx] = fm_base_type_get(sys, FM_TYPE_DECIMAL128);
    sprintf(buf[idx++].data(), "ask_prx_%u", i);
    types[idx] = fm_base_type_get(sys, FM_TYPE_DECIMAL128);
    sprintf(buf[idx++].data(), "ask_shr_%u", i);
    types[idx] = fm_base_type_get(sys, FM_TYPE_UINT32);
    sprintf(buf[idx++].data(), "ask_ord_%u", i);
  }

  vector<const char *> names(nf);
  for (unsigned i = 0; i < nf; ++i) {
    names[i] = buf[i].data();
  }

  auto *type = fm_frame_type_get1(sys, nf, names.data(), types.data(), 1, dims);
  if (!type) {
    return nullptr;
  }

  auto ctx_cl = make_unique<bb_exe_cl>(book);

  auto *def = fm_ctx_def_new();
  fm_ctx_def_closure_set(def, ctx_cl.release());
  fm_ctx_def_inplace_set(def, false);
  fm_ctx_def_type_set(def, type);
  fm_ctx_def_stream_call_set(def, &fm_comp_book_build_stream_call);
  fm_ctx_def_query_call_set(def, nullptr);
  return def;
}

void fm_comp_book_build_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *ctx_cl = (bb_exe_cl *)fm_ctx_def_closure(def);
  if (ctx_cl != nullptr)
    delete ctx_cl;
}
