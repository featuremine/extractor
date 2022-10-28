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
 * @file book.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the book interface
 *
 * This file contains declarations of the book interface
 * @see http://www.featuremine.com
 */

#ifndef __FM_BOOK_H__
#define __FM_BOOK_H__

#include "fmc/platform.h"

#include "fmc/decimal128.h"
#include "fmc/time.h"

typedef struct fm_book_pos fm_book_pos_t;
typedef struct fm_book fm_book_t;
typedef struct fm_book_shared fm_book_shared_t;
typedef struct fm_levels fm_levels_t;
typedef struct fm_level fm_level_t;
typedef struct fm_order fm_order_t;

#ifdef __cplusplus
extern "C" {
#endif

FMMODFUNC fm_book_t *fm_book_new();

FMMODFUNC void fm_book_del(fm_book_t *book);

FMMODFUNC void fm_book_uncross_set(fm_book_t *book, bool set);

FMMODFUNC void fm_book_add(fm_book_t *book, fmc_time64_t rec, fmc_time64_t ven,
                           uint64_t seq, uint64_t id, fmc_decimal128_t price,
                           fmc_decimal128_t qty, bool is_bid);

FMMODFUNC void fm_book_ins(fm_book_t *book, fmc_time64_t rec, fmc_time64_t ven,
                           uint64_t seq, uint64_t id, uint64_t prio,
                           fmc_decimal128_t price, fmc_decimal128_t qty,
                           bool is_bid);

FMMODFUNC void fm_book_pos(fm_book_t *book, fmc_time64_t rec, fmc_time64_t ven,
                           uint64_t seq, uint64_t id, uint32_t pos,
                           fmc_decimal128_t price, fmc_decimal128_t qty,
                           bool is_bid);

FMMODFUNC bool fm_book_mod(fm_book_t *book, uint64_t id, fmc_decimal128_t price,
                           fmc_decimal128_t qty, bool is_bid);

FMMODFUNC bool fm_book_exe(fm_book_t *book, uint64_t id, fmc_decimal128_t price,
                           fmc_decimal128_t qty, bool is_bid);

FMMODFUNC bool fm_book_pla(fm_book_t *book, fmc_time64_t rec, fmc_time64_t ven,
                           uint64_t seq, fmc_decimal128_t price,
                           fmc_decimal128_t qty, bool is_bid);

FMMODFUNC void fm_book_clr(fm_book_t *book);

FMMODFUNC fm_levels_t *fm_book_levels(fm_book_t *book, bool is_bid);
FMMODFUNC unsigned fm_book_levels_size(fm_levels_t *lvls);
FMMODFUNC fm_level_t *fm_book_level(fm_levels_t *lvls, unsigned idx);
FMMODFUNC fmc_decimal128_t fm_book_level_prx(fm_level_t *lvl);
FMMODFUNC fmc_decimal128_t fm_book_level_shr(fm_level_t *lvl);
FMMODFUNC uint32_t fm_book_level_ord(fm_level_t *lvl);
FMMODFUNC fm_order_t *fm_book_level_order(fm_level_t *lvl, unsigned idx);
FMMODFUNC uint64_t fm_book_order_prio(fm_order_t *lvl);
FMMODFUNC uint64_t fm_book_order_id(fm_order_t *lvl);
FMMODFUNC fmc_decimal128_t fm_book_order_qty(fm_order_t *lvl);
FMMODFUNC fmc_time64_t fm_book_order_rec(fm_order_t *lvl);
FMMODFUNC fmc_time64_t fm_book_order_ven(fm_order_t *lvl);
FMMODFUNC uint64_t fm_book_order_seq(fm_order_t *lvl);

FMMODFUNC fm_book_shared_t *fm_book_shared_new();
FMMODFUNC void fm_book_shared_inc(fm_book_shared_t *shared_book);
FMMODFUNC void fm_book_shared_dec(fm_book_shared_t *shared_book);
FMMODFUNC fm_book_t *fm_book_shared_get(fm_book_shared_t *shared_book);

#ifdef __cplusplus
}
#endif

#endif // __FM_BOOK_H__
