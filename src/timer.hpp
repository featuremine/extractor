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
 * @file timer.hpp
 * @author Maxim Trokhimtchouk
 * @date 22 Nov 2017
 * @brief C++ interface for computation definition
 *
 * Here we define C++ interface for computation definition
 */

#pragma once

#include "extractor/common.hpp"
#include "extractor/comp_def.hpp"
#include "extractor/frame.hpp"
#include "fmc++/time.hpp"
#include "fmc++/mpl.hpp"

namespace fm {
using namespace std;

class timer : public fm::computation<tuple<>, timer_frame> {
  timer(fmc_time64_t period) : period_(period) {}

public:
  static timer *create(fmc_time64_t period) { return new timer(period); }
  bool init(fm::stream_context &ctx) {
    result().resize(1);
    scheduled = fmc_time64_end();
    ctx.queue(this);
    return true;
  }
  bool exec(fm::stream_context &ctx) {
    auto now = ctx.now();
    auto needed = period_ * (now / period_);
    auto next = needed + period_;
    bool start = scheduled == fmc_time64_end();
    bool done = !start;
    if (start && needed == now) {
      done = true;
      scheduled = needed;
    }
    auto res = result()[0];
    res.scheduled() = scheduled;
    res.actual() = now;
    scheduled = next;
    ctx.schedule(this, scheduled);
    return done;
  }
  bool init(fm::query_context &ctx) { return false; }
  bool exec(fm::query_context &ctx) { return false; }

private:
  fmc_time64_t period_;
  fmc_time64_t scheduled;
};

class clock_timer : public fm::computation<tuple<>, timer_frame> {
  clock_timer(fmc_time64_t start, fmc_time64_t stop, fmc_time64_t period)
      : start_(start), stop_(stop), period_(period) {}

public:
  static clock_timer *create(fmc_time64_t start, fmc_time64_t stop,
                             fmc_time64_t period) {
    return new clock_timer(start, stop, period);
  }
  bool init(fm::stream_context &ctx) {
    result().resize(1);
    scheduled = start_;
    ctx.schedule(this, scheduled);
    return true;
  }
  bool exec(fm::stream_context &ctx) {
    auto now = ctx.now();
    auto needed = start_ + period_ * ((now - start_) / period_);
    auto next = needed + period_;
    auto done = needed <= now;
    auto res = result()[0];
    res.scheduled() = scheduled;
    res.actual() = now;
    scheduled = next;
    if (scheduled <= stop_) {
      ctx.schedule(this, scheduled);
    }
    return done;
  }
  bool init(fm::query_context &ctx) { return false; }
  bool exec(fm::query_context &ctx) { return false; }

private:
  fmc_time64_t start_;
  fmc_time64_t stop_;
  fmc_time64_t period_;
  fmc_time64_t scheduled;
};

} // namespace fm
