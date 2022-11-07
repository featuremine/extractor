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
 * @file tests.cpp
 * @author Andres Rangel
 * @date 9 Nov 2018
 * @brief File runs tests for platform
 *
 * Runs all imported tests for platform
 * @see http://www.featuremine.com
 */

#include <string>

std::string src_dir;

#include "accumulate.hpp"
#include "book_build.hpp"
#include "cond.hpp"
#include "constant.hpp"
#include "counters_test.hpp"
#include "csv_play.hpp"
#include "equal.hpp"
#include "filter_if.hpp"
#include "filter_unless.hpp"
#include "graph_serial.hpp"
#include "greater.hpp"
#include "greater_equal.hpp"
#include "less.hpp"
#include "less_equal.hpp"
#include "logical_and.hpp"
#include "logical_not.hpp"
#include "logical_or.hpp"
#include "module.hpp"
#include "module_serial.hpp"
#include "split_sample.hpp"
#include "split_sample_capture.hpp"
#include "string_utils_tests.hpp"
#include "substr.hpp"
#include "writer_reader.hpp"

//#include "mm_file.hpp"
//#include "perf_test.hpp"
//#include "split_by.hpp"

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  src_dir = argv[1];
  return RUN_ALL_TESTS();
}
