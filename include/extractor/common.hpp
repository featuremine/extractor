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
 * @file common.hpp
 * @author Maxim Trokhimtchouk
 * @date 24 Oct 2017
 * @brief Some of the commonly used features within the platform
 *
 * Here we define commonly used features
 */

#pragma once

#include "extractor/frame.hpp"
#include <string_view>

namespace fm {
using namespace std;

FRAME(price_frame, 1);
FIELD(price, FLOAT64);
FIELDS(price);
END_FRAME(price_frame);

FRAME(nbbo_frame, 1);
FIELD(receive, TIME64);
FIELD(bidprice, DECIMAL64);
FIELD(askprice, DECIMAL64);
FIELD(bidqty, INT32);
FIELD(askqty, INT32);
FIELDS(receive, bidprice, askprice, bidqty, askqty);
END_FRAME(nbbo_frame);

FRAME(bbo_frame, 1);
FIELD(receive, TIME64);
FIELD(bidprice, DECIMAL128);
FIELD(askprice, DECIMAL128);
FIELD(bidqty, INT32);
FIELD(askqty, INT32);
FIELDS(receive, bidprice, askprice, bidqty, askqty);
END_FRAME(bbo_frame);

FRAME(short_trade_frame, 1);
FIELD(receive, TIME64);
FIELD(price, DECIMAL128);
FIELD(qty, INT32);
FIELDS(receive, price, qty);
END_FRAME(short_trade_frame);

FRAME(trade_frame, 1);
FIELD(receive, TIME64);
FIELD(price, DECIMAL64);
FIELD(qty, INT32);
FIELD(side, INT32);
FIELDS(receive, price, qty, side);
END_FRAME(trade_frame);

FRAME(trade_bar_frame, 1);
FIELD(start_time, TIME64);
FIELD(end_time, TIME64);
FIELD(first_px, DECIMAL64);
FIELD(first_qty, INT32);
FIELD(first_mkt, char[32]);
FIELD(first_trade_time, TIME64);
FIELD(high_px, DECIMAL64);
FIELD(last_px, DECIMAL64);
FIELD(last_qty, INT32);
FIELD(last_mkt, char[32]);
FIELD(last_trade_time, TIME64);
FIELD(low_px, DECIMAL64);
FIELD(trades, INT32);
FIELD(vwap, FLOAT64);
FIELD(volume, INT32);
FIELDS(start_time, end_time, first_px, first_qty, first_mkt, first_trade_time,
       high_px, last_px, last_qty, last_mkt, last_trade_time, low_px, trades,
       vwap, volume);
END_FRAME(trade_bar_frame);

FRAME(cum_trade_frame, 1);
FIELD(shares, INT64);
FIELD(notional, FLOAT64);
FIELDS(shares, notional);
END_FRAME(cum_trade_frame);

FRAME(timer_frame, 1);
FIELD(scheduled, fmc_time64_t);
FIELD(actual, fmc_time64_t);
FIELDS(scheduled, actual);
END_FRAME(timer_frame);

} // namespace fm
