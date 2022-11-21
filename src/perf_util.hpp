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
 * @file sample.hpp
 * @author Maxim Trokhimtchouk
 * @date 20 Apr 2018
 * @brief File contains C++ definitions of the sample operators
 *
 * This file contains declarations of the sample operators
 * @see http://www.featuremine.com
 */

#include "fmc++/counters.hpp"

#include <string>

#pragma once

struct perf_sampler_t {
  using counter_t = fmc::counter::nanoseconds;
  using sampler_t = fmc::counter::precision_sampler;

  perf_sampler_t(const char *name) : name_(name) {
    auto perf_flag = getenv("ENABLE_COUNTERS");
    enabled_ = perf_flag && std::string_view(perf_flag) == "1";
  }

  ~perf_sampler_t() {
    if (enabled_) {
      std::vector<double> percentiles{25.0, 50.0, 75.0, 90.0,
                                      95.0, 99.0, 100.0};
      std::cout << name_ << std::endl;
      for (double &percentile : percentiles) {
        std::cout << "  " << percentile
                  << "% percentile: " << record_.percentile(percentile)
                  << " nanoseconds" << std::endl;
      }
      std::cout << std::endl;
    }
  }

  void start() {
    if (enabled_) {
      record_.start();
    }
  }

  void stop() {
    if (enabled_) {
      record_.stop();
    }
  }

  std::string name_;
  fmc::counter::record<counter_t, sampler_t> record_;
  bool enabled_;
};

template <typename Sampler> struct perf_scoped_sampler_t {
  perf_scoped_sampler_t(Sampler &inst) : inst_(inst) { inst_.start(); }
  ~perf_scoped_sampler_t() { inst_.stop(); }
  Sampler &inst_;
};
