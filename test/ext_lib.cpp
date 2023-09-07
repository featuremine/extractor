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
 * @file ext_main.cpp
 * @author Maxim Trokhimtchouk
 * @date 29 Nov 2017
 * @brief Timer extension sample
 *
 * @see http://www.featuremine.com
 */

#include "extractor/api.h"
#include "extractor/common.hpp"
#include "extractor/comp_def.hpp"
#include "fmc++/time.hpp"

#include <tuple>
#include <vector>

namespace fm {

FRAME(timer_count_frame, 1);
FIELD(count, INT64);
FIELD(period, TIME64);
FIELDS(count, period);
END_FRAME(timer_count_frame);

class timer_count
    : public fm::computation<tuple<timer_frame>, timer_count_frame> {
  timer_count() = default;

public:
  static timer_count *create() { return new timer_count(); }
  bool init(fm::stream_context &ctx) {
    result().resize(1);
    auto res = result()[0];
    res.count() = 0;
    res.period() = fmc_time64_from_raw(0);
    prev_ = fmc_time64_end();
    return true;
  }
  bool exec(fm::stream_context &ctx) {
    auto now = get<0>(input())[0].actual();
    auto res = result()[0];
    auto count = res.count();
    res.count() += 1;
    if (!fmc_time64_is_end(prev_)) {
      auto period = res.period();
      res.period() = (count * period + (now - prev_)) / (count + 1);
    }
    prev_ = now;
    return true;
  }
  bool init(fm::query_context &ctx) { return false; }
  bool exec(fm::query_context &ctx) { return false; }

private:
  fmc_time64_t prev_;
};

class timer_count_avg
    : public fm::computation<vector<timer_count_frame>, timer_count_frame> {
  timer_count_avg() = default;

public:
  static timer_count_avg *create() { return new timer_count_avg(); }
  bool init(fm::stream_context &ctx) {
    result().resize(1);
    auto res = result()[0];
    res.count() = 0;
    res.period() = fmc_time64_from_raw(0);
    return true;
  }
  bool exec(fm::stream_context &ctx) {
    auto res = result()[0];
    int num = 0;
    int64_t total_count = 0;
    fmc_time64_t total_period = fmc_time64_from_raw(0);
    for (auto &in : input()) {
      ++num;
      total_count += in[0].count();
      total_period = total_period + in[0].period();
    }
    if (num) {
      res.count() = total_count / num;
      res.period() = total_period / num;
    }
    return true;
  }
  bool init(fm::query_context &ctx) { return false; }
  bool exec(fm::query_context &ctx) { return false; }
};

/**
 * Computation definition
 * It must provide the following information to describe a computation:
 * name: used by the computing system to identify the computation, this name
 * MUST be unique. generate: reference to the operator's generator function
 * destroy: reference to the operator's destruction function
 * closure: pointer to the closure that is shared between all operator instances
 */
fm_comp_def_t timer_count_def = {
    "timer_count",                      // name
    &fm_cpp_comp_generate<timer_count>, // generate
    &fm_cpp_comp_destroy<timer_count>,  // destroy
    NULL                                // closure
};

fm_comp_def_t timer_count_avg_def = {
    "timer_count_avg",                      // name
    &fm_cpp_comp_generate<timer_count_avg>, // generate
    &fm_cpp_comp_destroy<timer_count_avg>,  // destroy
    NULL                                    // closure
};

/**
 * Registers the operator definition in the computational system.
 * The name of this function MUST start with ExtractorInit_ to allow the system
 * extension loader to find the function in external modules.
 *
 * @param api extractor API functions
 * @param mod loaded module
 * @param error
 */
extern "C" FMMODFUNC void ExtractorInit_ext_lib(struct extractor_api_v1 *api,
                                                fm_comp_sys_t *sys,
                                                fmc_error_t **error) {
  fmc_error_clear(error);
  api->comp_type_add(sys, &timer_count_def);
  api->comp_type_add(sys, &timer_count_avg_def);
}

} // namespace fm
