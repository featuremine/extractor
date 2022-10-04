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
 * @file time64_serial.cpp
 * @author Maxim Trokhimtchouk
 * @date 29 Jul 2018
 * @brief File contains tests for computational graph object
 *
 * @see http://www.featuremine.com
 */

#include "time64.hpp"
#include <fmc++/gtestwrap.hpp>

#include <chrono>
#include <iostream>
#include <sstream>

TEST(time64, serial) {
  using namespace std;
  using namespace chrono;
  stringstream str;
  auto epoch = system_clock::now().time_since_epoch();
  auto now = fm_time64_from_nanos(duration_cast<nanoseconds>(epoch).count());
  str << now;
  fm_time64_t test = {};
  str >> test;
  EXPECT_EQ(now, test);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
