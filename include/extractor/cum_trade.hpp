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
 * @file cum_trade.hpp
 * @author Maxim Trokhimtchouk
 * @date 22 Nov 2017
 * @brief Cumulative trade feature
 */

#pragma once

#include "fmc++/rprice.hpp"
#include "feature/common.hpp"

#include <comp_def.hpp>
#include <fmc++/mpl.hpp>
#include <frame.hpp>

namespace jubilee {

/**
 * @brief cum trade frame definition
 *
 * defines frame for cum trade
 */
FRAME(cum_trade_frame, 1);
FIELD(shares, INT64);
FIELD(notional, FLOAT64);
FIELDS(shares, notional);
END_FRAME(cum_trade_frame);

/**
 * @brief cum trade computation class
 */
class cum_trade : public fm::computation<tuple<trade_frame>, cum_trade_frame> {
  cum_trade() = default;

public:
  /**
   * @brief create method
   *
   * creates cum trade computation
   */
  static cum_trade *create() { return new cum_trade(); }

  /**
   * @brief init method
   *
   * initializes cum trade computation for input stream context
   */
  bool init(fm::stream_context &ctx) {
    result().resize(1);
    auto res = result()[0];
    res.shares() = 0;
    res.notional() = 0;
    return true;
  }

  /**
   * @brief exec method
   *
   * executes cum trade computation for input stream context
   */
  bool exec(fm::stream_context &ctx) {
    auto trade = get<0>(input())[0];
    auto res = result()[0];
    res.shares() += trade.qty();
    res.notional() += trade.qty() * to<double>(trade.price());
    return true;
  }

  /**
   * @brief init method
   *
   * initializes cum trade computation for input query context
   */
  bool init(fm::query_context &ctx) { return false; }

  /**
   * @brief exec method
   *
   * executes cum trade computation for input query context
   */
  bool exec(fm::query_context &ctx) { return false; }
};

/**
 * @brief cum trade total computation class
 */
class cum_trade_total
    : public fm::computation<vector<cum_trade_frame>, cum_trade_frame> {
  cum_trade_total() = default;

public:
  /**
   * @brief create method
   *
   * creates cum trade total computation
   */
  static cum_trade_total *create() { return new cum_trade_total(); }

  /**
   * @brief init method
   *
   * initializes cum trade total computation for input stream context
   */
  bool init(fm::stream_context &ctx) {
    result().resize(1);
    auto res = result()[0];
    res.shares() = 0;
    res.notional() = 0;
    return true;
  }

  /**
   * @brief exec method
   *
   * executes cum trade total computation for input query context
   */
  bool exec(fm::stream_context &ctx) {
    auto res = result()[0];
    res.shares() = 0;
    res.notional() = 0;
    for (auto &inp : input()) {
      auto in = inp[0];
      res.shares() += in.shares();
      res.notional() += in.notional();
    }
    return true;
  }

  /**
   * @brief init method
   *
   * initializes cum trade total computation for input query context
   */
  bool init(fm::query_context &ctx) { return false; }

  /**
   * @brief exec method
   *
   * executes cum trade total computation for input query context
   */
  bool exec(fm::query_context &ctx) { return false; }
};

} // namespace jubilee
