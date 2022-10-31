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
 * @file window.hpp
 * @author Andres Rangel
 * @date 24 Dec 2018
 * @brief File contains C++ definitions of the window operators
 *
 * This file contains declarations of the window operators
 * @see http://www.featuremine.com
 */

#include "field_util.hpp"
#include "window_util.hpp"

template <class T> struct sma_base_comp_cl {
  sma_base_comp_cl() : sum_(0.0) {}
  T init(const T &val) { return val; }
  T push(const T &val, const uint64_t &count) {
    if (count == 1)
      sum_ = val;
    else
      sum_ += val;
    return sum_ / T(count);
  }
  T pop(const T &val, const uint64_t &count) {
    sum_ -= val;

    if (count)
      return sum_ / T(count);

    return std::numeric_limits<T>::quiet_NaN();
  }

  T sum_;
};

template <class T>
using sma_comp_cl = queued_field_exec_cl<T, sma_base_comp_cl>;

template <class T> struct stdev_base_comp_cl {
  stdev_base_comp_cl() : sum_(0.0), m_(0.0) {}
  T init(const T &val) { return T(0.0); }
  T push(const T &val, const uint64_t &count) {
    if (count > 1) {
      auto prev = sum_;
      sum_ += val;
      m_ += (val - prev / T(count - 1)) * (val - sum_ / T(count));
      return sqrt(m_ / T(count - 1));
    }

    sum_ = val;
    return T(0.0);
  }
  T pop(const T &val, const uint64_t &count) {
    auto prev = sum_;

    sum_ -= val;
    m_ -= (val - prev / T(count + 1)) * (val - sum_ / T(count));

    if (count > 1) {
      return sqrt(m_ / T(count - 1));
    }

    return m_ = T(0.0);
  }

  T sum_;
  T m_;
};

template <class T>
using stdev_comp_cl = queued_field_exec_cl<T, stdev_base_comp_cl>;

template <class T> struct median_base_comp_cl {
  T init(const T &val) { return val; }
  T push(const T &val, const uint64_t &count) {
    vals_.insert(std::upper_bound(vals_.begin(), vals_.end(), val), val);

    auto len = vals_.size();
    if (len) {
      auto len_half = len / 2;
      return len % 2 ? vals_[len_half]
                     : vals_[len_half - 1] / 2.0 + vals_[len_half] / 2.0;
    }

    return std::numeric_limits<T>::quiet_NaN();
  }
  T pop(const T &val, const uint64_t &count) {
    vals_.erase(lower_bound(vals_.begin(), vals_.end(), val));

    auto len = vals_.size();
    if (len) {
      auto len_half = len / 2;
      return len % 2 ? vals_[(len_half)]
                     : vals_[len_half - 1] / 2.0 + vals_[len_half] / 2.0;
    }

    return std::numeric_limits<T>::quiet_NaN();
  }

  std::vector<T> vals_;
};

template <class T>
using median_comp_cl = queued_field_exec_cl<T, median_base_comp_cl>;

template <class T> struct ema_exp_base_comp_cl {
  ema_exp_base_comp_cl(fmc_time64_t size) : window_size_(size) {}
  T init(const T &val) { return buff_ = val; };
  void set(const T &val, const fmc_time64_t &prev, const fmc_time64_t &now) {
    if (isnan(buff_)) {
      buff_ = val;
      return;
    }
    T e = exp(-(T(fmc_time64_raw(now)) - T(fmc_time64_raw(prev))) /
              T(fmc_time64_raw(window_size_)));
    buff_ = (1.0 - e) * val + e * buff_;
  };
  T asof(const T &val, const fmc_time64_t &prev, const fmc_time64_t &now) {
    if (prev == now || isnan(val)) {
      return buff_;
    }
    T e = exp(-(T(fmc_time64_raw(now)) - T(fmc_time64_raw(prev))) /
              T(fmc_time64_raw(window_size_)));
    return (1.0 - e) * val + e * buff_;
  };
  T buff_;
  fmc_time64_t window_size_;
};

template <class T>
using ema_exp_comp_cl = exp_field_exec_cl<T, ema_exp_base_comp_cl>;

template <class T> struct stdev_exp_base_comp_cl {
  stdev_exp_base_comp_cl(fmc_time64_t size) : var_(0.0), window_size_(size) {}
  T init(const T &val) {
    buff_ = val;
    return T(0.0);
  };
  void set(const T &val, const fmc_time64_t &prev, const fmc_time64_t &now) {
    if (isnan(buff_)) {
      buff_ = val;
      var_ = T(0.0);
      return;
    }
    T delta = val - buff_;
    T e = exp(-(T(fmc_time64_raw(now)) - T(fmc_time64_raw(prev))) /
              T(fmc_time64_raw(window_size_)));
    buff_ = buff_ + (1.0 - e) * delta;
    var_ = e * (var_ + (1.0 - e) * delta * delta);
  };
  T asof(const T &val, const fmc_time64_t &prev, const fmc_time64_t &now) {
    if (prev == now || isnan(val)) {
      return sqrt(var_);
    }

    T delta = val - buff_;
    T e = exp(-(T(fmc_time64_raw(now)) - T(fmc_time64_raw(prev))) /
              T(fmc_time64_raw(window_size_)));
    return sqrt(e * (var_ + (1.0 - e) * delta * delta));
  };
  T buff_;
  T var_;
  fmc_time64_t window_size_;
};

template <class T>
using stdev_exp_comp_cl = exp_field_exec_cl<T, stdev_exp_base_comp_cl>;

bool fm_comp_window_add(fm_comp_sys_t *sys) {
  return fm_comp_sample_add<fm_comp_tick_window<sma_comp_cl>>(sys, "sma_tick_"
                                                                   "mw") &&
         fm_comp_sample_add<fm_comp_time_window<sma_comp_cl>>(sys, "sma_time_"
                                                                   "mw") &&
         fm_comp_sample_add<fm_comp_tick_window<stdev_comp_cl>>(
             sys, "stdev_tick_mw") &&
         fm_comp_sample_add<fm_comp_time_window<stdev_comp_cl>>(
             sys, "stdev_time_mw") &&
         fm_comp_sample_add<fm_comp_tick_window<median_comp_cl>>(
             sys, "median_tick_mw") &&
         fm_comp_sample_add<fm_comp_time_window<median_comp_cl>>(
             sys, "median_time_mw") &&
         fm_comp_sample_add<fm_comp_exp_window<ema_exp_comp_cl>>(sys,
                                                                 "ema_exp") &&
         fm_comp_sample_add<fm_comp_exp_window<stdev_exp_comp_cl>>(sys,
                                                                   "stdev_exp");
}
