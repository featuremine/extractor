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
 * @file updates.hpp
 * @date 19 Dec 2022
 * @brief File contains C++ implementation of the book updates interface
 *
 * @see http://www.featuremine.com
 */

#include "extractor/book/updates.hpp"
#include "extractor/book/book.h"

namespace fm::book {

bool update_from_message(fmc_time64_t now, message &box, fm_book_t *inst) {
  if (!std::visit(fmc::overloaded{
                      [inst, now](const book::updates::add &msg) {
                        fm_book_add(inst, now, msg.vendor, msg.seqn, msg.id,
                                    msg.price, msg.qty, msg.is_bid);
                        return true;
                      },
                      [inst, now](const book::updates::insert &msg) {
                        fm_book_ins(inst, now, msg.vendor, msg.seqn, msg.id,
                                    msg.prio, msg.price, msg.qty, msg.is_bid);
                        return true;
                      },
                      [inst, now](const book::updates::position &msg) {
                        fm_book_pos(inst, now, msg.vendor, msg.seqn, msg.id,
                                    msg.pos, msg.price, msg.qty, msg.is_bid);
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
                      [](const book::updates::heartbeat &msg) { return false; },
                      [](const book::updates::none &msg) { return false; },
                  },
                  box)) {
    return false;
  }
  return std::visit(
      fmc::overloaded{
          [&](const auto &msg) { return msg.batch != 1; },
          [&](const book::updates::time &msg) { return false; },
          [&](const book::updates::heartbeat &msg) { return false; },
          [&](const book::updates::none &msg) { return false; },
      },
      box);
}

} // namespace fm::book
