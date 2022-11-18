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
 * @file std_comp.cpp
 * @author Maxim Trokhimtchouk
 * @date 20 Apr 2018
 * @brief File contains C++ implementation of the computational system
 *
 * This file contains implementation of the computational system
 * @see http://www.featuremine.com
 */

#include "extractor/std_comp.h"
#include "accumulate.h"
#include "add.h"
#include "bbo_aggr.h"
#include "book_build.h"
#include "book_msg.h"
#include "book_play_split.h"
#include "book_trades.h"
#include "combine.h"
#include "cond.h"
#include "constant.h"
#include "convert.h"
#include "count.h"
#include "csv_play.h"
#include "csv_record.h"
#include "csv_tail.h"
#include "cumulative.h"
#include "decode_data.h"
#include "decode_receive.h"
#include "delayed.h"
#include "diff.h"
#include "divide.h"
#include "equal.h"
#include "exp.h"
#include "extractor/perf_timer.h"
#include "field.h"
#include "fields.h"
#include "filter_if.h"
#include "filter_unless.h"
#include "frame_ytp_decode.h"
#include "frame_ytp_encode.h"
#include "greater.h"
#include "greater_equal.h"
#include "heartbeat.h"
#include "identity.h"
#include "is_inf.h"
#include "is_nan.h"
#include "is_zero.h"
#include "join.h"
#include "less.h"
#include "less_equal.h"
#include "ln.h"
#include "log.h"
#include "logical_and.h"
#include "logical_not.h"
#include "logical_or.h"
#include "max.h"
#include "min.h"
#include "mp_play.h"
#include "mp_record.h"
#include "mult.h"
#include "nan.h"
#include "nano.h"
#include "not_equal.h"
#include "ore_live_split.h"
#include "ore_sim_split.h"
#include "ore_ytp_decode.h"
#include "pow.h"
#include "roundop.h"
#include "split.h"
#include "split_by.h"
#include "seq_ore_live_split.h"
#include "substr.h"
#include "sum.h"
#include "tick_lag.h"
#include "time_lag.h"
#include "timeout.h"
#include "trigger.h"
#include "unique.h"
#include "ytp_sequence.h"
#include "zero.h"

#include "activated_timer.hpp"
#include "ar.hpp"
#include "average_tw.hpp"
#include "comp_sys.hpp"
#include "data_bar.hpp"
#include "delta.hpp"
#include "extractor/comp_def.hpp"
#include "percentile.hpp"
#include "sample.hpp"
#include "timer.hpp"
#include "window.hpp"

bool fm_comp_sys_std_comp(fm_comp_sys_t *sys) {
  return fm_comp_type_add(sys, &fm_comp_csv_play) &&
         fm_comp_type_add(sys, &fm_comp_csv_record) &&
         fm_comp_type_add(sys, &fm_comp_csv_tail) &&
         fm_comp_type_add(sys, &fm_comp_book_build) &&
         fm_comp_type_add(sys, &fm_comp_bbo_aggr) &&
         fm_comp_type_add(sys, &fm_comp_bbo_book_aggr) &&
         fm_comp_type_add(sys, &fm_comp_split) &&
         fm_comp_type_add(sys, &fm_comp_identity) &&
         fm_comp_type_add(sys, &fm_comp_mp_play) &&
         fm_comp_type_add(sys, &fm_comp_mp_record) &&
         fm_comp_type_add(sys, &fm_comp_ore_live_split) &&
         fm_comp_type_add(sys, &fm_comp_seq_ore_live_split) &&
         fm_comp_type_add(sys, &fm_comp_ore_sim_split) &&
         fm_comp_type_add(sys, &fm_comp_book_play_split) &&
         fm_comp_type_add(sys, &fm_comp_book_msg) &&
         fm_comp_type_add(sys, &fm_comp_book_trades) &&
         fm_comp_type_add(sys, &fm_comp_book_header) &&
         fm_comp_type_add(sys, &fm_comp_book_vendor_time) &&
         fm_comp_type_add(sys, &fm_comp_tick_lag) &&
         fm_comp_type_add(sys, &fm_comp_time_lag) &&
         fm_comp_type_add(sys, &fm_comp_field) &&
         fm_comp_type_add(sys, &fm_comp_combine) &&
         fm_comp_type_add(sys, &fm_comp_diff) &&
         fm_comp_type_add(sys, &fm_comp_divide) &&
         fm_comp_type_add(sys, &fm_comp_convert) &&
         fm_comp_type_add(sys, &fm_comp_join) &&
         fm_comp_type_add(sys, &fm_comp_trigger) &&
         fm_comp_type_add(sys, &fm_comp_logical_and) &&
         fm_comp_type_add(sys, &fm_comp_filter_unless) &&
         fm_comp_type_add(sys, &fm_comp_greater) &&
         fm_comp_type_add(sys, &fm_comp_greater_equal) &&
         fm_comp_type_add(sys, &fm_comp_less_equal) &&
         fm_comp_type_add(sys, &fm_comp_less) &&
         fm_comp_type_add(sys, &fm_comp_equal) &&
         fm_comp_type_add(sys, &fm_comp_not_equal) &&
         fm_comp_type_add(sys, &fm_comp_filter_if) &&
         fm_comp_type_add(sys, &fm_comp_logical_or) &&
         fm_comp_type_add(sys, &fm_comp_find_substr) &&
         fm_comp_type_add(sys, &fm_comp_substr) &&
         fm_comp_type_add(sys, &fm_comp_constant) &&
         fm_comp_type_add(sys, &fm_comp_logical_not) &&
         fm_comp_type_add(sys, &fm_comp_cond) &&
         fm_comp_type_add(sys, &fm_comp_cumulative) &&
         fm_comp_type_add(sys, &fm_comp_unique) &&
         fm_comp_type_add(sys, &fm_comp_sum) &&
         fm_comp_type_add(sys, &fm_comp_add) &&
         fm_comp_type_add(sys, &fm_comp_mult) &&
         fm_comp_type_add(sys, &fm_comp_ln) &&
         fm_comp_type_add(sys, &fm_comp_log) &&
         fm_comp_type_add(sys, &fm_comp_exp) &&
         fm_comp_type_add(sys, &fm_comp_pow) &&
         fm_comp_type_add(sys, &fm_comp_max) &&
         fm_comp_type_add(sys, &fm_comp_min) &&
         fm_comp_type_add(sys, &fm_comp_count) &&
         fm_comp_type_add(sys, &fm_comp_nano) &&
         fm_comp_type_add(sys, &fm_comp_accumulate) &&
         fm_comp_type_add(sys, &fm_comp_fields) &&
         fm_comp_type_add(sys, &fm_comp_zero) &&
         fm_comp_type_add(sys, &fm_comp_is_zero) &&
         fm_comp_type_add(sys, &fm_comp_is_inf) &&
         fm_comp_type_add(sys, &fm_comp_frame_ytp_encode) &&
         fm_comp_type_add(sys, &fm_comp_frame_ytp_decode) &&
         fm_comp_type_add(sys, &fm_comp_ore_ytp_decode) &&
         fm_comp_type_add(sys, &fm_comp_decode_data) &&
         fm_comp_type_add(sys, &fm_comp_decode_receive) &&
         fm_comp_type_add(sys, &fm_comp_timeout) &&
         fm_comp_type_add(sys, &fm_comp_delayed) &&
         fm_comp_type_add(sys, &fm_comp_heartbeat) &&
         fm_comp_type_add(sys, &fm_comp_ytp_sequence) &&
         fm_comp_type_add(sys, &fm_comp_is_nan) && fm_comp_split_by_add(sys) &&
         fm_comp_type_add(sys, &fm_comp_nan) && fm_comp_sample_add_all(sys) &&
         fm_comp_average_tw_add(sys) && fm_comp_delta_add(sys) &&
         fm_comp_window_add(sys) && fm_comp_percentile_add(sys) &&
         fm_comp_ar_add(sys) && fm_comp_type_add(sys, &fm_comp_round) &&
         fm_comp_perf_timer_add(sys, (void *)&sys->samples_) &&
         fm_comp_activated_timer_add(sys) && fm_comp_data_bar_add(sys) &&
         fm::fm_cpp_comp_type_add<fm::timer>(sys, "timer") &&
         fm::fm_cpp_comp_type_add<fm::clock_timer>(sys, "clock_timer");
}
