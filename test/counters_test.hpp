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
 * @file counters_test.hpp
 * @author Andres Rangel
 * @date 4 Jun 2018
 * @brief File contains test for counters mechanism
 *
 * This file contains test for counters mechanism
 */

#include "fmc++/counters.hpp"
#include "fmc++/gtestwrap.hpp"
#include "fmc++/mpl.hpp"

#include <algorithm>
#include <list>
#include <string>
#include <string_view>

using namespace fmc;
using namespace std;

TEST(counters, ewma) {
  auto t = chrono::microseconds(100);

  // nano test

  counter::nano_record<counter::ewma<2>> nr;
  ASSERT_EQ(nr.value(), 0);
  for (int i = 0; i < 100; ++i) {
    counter::scoped_sampler scoped_s(nr);
    this_thread::sleep_for(t);
  }

  auto nr_m = nr.value();
  cout << "nano ewma 2 value " << nr_m << " ns" << endl;
  ASSERT_GT(nr_m, 0);

  // rdtsc test

  counter::rdtsc_record<counter::ewma<2>> rr;
  ASSERT_EQ(rr.value(), 0);

  for (int i = 0; i < 100; ++i) {
    counter::scoped_sampler scoped_s(rr);
    this_thread::sleep_for(t);
  }

  auto rr_m = rr.value();
  cout << "rdtsc ewma 2 value " << rr_m << " ns" << endl;
  ASSERT_GT(rr_m, 0);

  // samples container test

  counter::samples ss;

  auto &test = ss.get<counter::ewma<2>>("Test");
  ASSERT_EQ(test.value(), 0);

  auto it = ss.find("Test");
  ASSERT_NE(it, ss.end());

  auto nit = ss.find("Invalid_Test");
  ASSERT_EQ(nit, ss.end());
}

TEST(counters, avg) {
  auto t = chrono::microseconds(100);

  // nano test

  counter::nano_record<counter::avg> nr;
  for (int i = 0; i < 100; ++i) {
    counter::scoped_sampler scoped_s(nr);
    this_thread::sleep_for(t);
  }

  auto nr_m = nr.value();
  cout << "nano avg value " << nr_m << " ns" << endl;
  ASSERT_GT(nr_m, 0);

  // rdtsc test

  counter::rdtsc_record<counter::ewma<2>> rr;
  for (int i = 0; i < 100; ++i) {
    counter::scoped_sampler scoped_s(rr);
    this_thread::sleep_for(t);
  }

  auto rr_m = rr.value();
  cout << "rdtsc avg value " << rr_m << " ns" << endl;
  ASSERT_GT(rr_m, 0);

  // samples container test

  counter::samples ss;

  auto &test = ss.get<counter::ewma<2>>("Test");
  (void)test;
  auto it = ss.find("Test");
  ASSERT_NE(it, ss.end());

  auto nit = ss.find("Invalid_Test");
  ASSERT_EQ(nit, ss.end());
}
