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
 * @file storage_util.hpp
 * @date 7 Nov 2022
 * @brief File contains storage type helpers
 *
 * @see http://www.featuremine.com
 */

#include "fmc++/decimal128.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/rational64.hpp"

template <class T> struct storage { using type = T; };

template <> struct storage<fmc_decimal128_t> {
  using type = typename fmc::decimal128;
};

template <> struct storage<fmc_rprice_t> {
  using type = typename fmc::rprice;
};

template <> struct storage<fmc_rational64_t> {
  using type = typename fmc::rational64;
};
