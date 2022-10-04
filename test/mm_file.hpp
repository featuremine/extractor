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
 * @file mm_file.hpp
 * @author Andres Rangel
 * @date 13 Jun 2018
 * @brief File contains test for mm_file
 *
 * This file contains test for mm_file
 */

#include <fmc++/gtestwrap.hpp>

extern "C" {
#include "src/mm_file.h"
}

//@TODO improve test to properly test rewind + close and data integrity

TEST(fm_file, main) {
  const char *test_file = "../test/data/sip_small_20171018.csv";

  mm_file *m = mm_file_open(test_file);

  ASSERT_TRUE(m != NULL);

  void *data = NULL;

  ASSERT_NE(mm_file_read(m, sizeof(char), &data), 0UL);

  ASSERT_TRUE(data != NULL);

  mm_file_rewind(m);

  mm_file_close(m);
}
