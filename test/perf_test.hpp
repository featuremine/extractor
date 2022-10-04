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
 * @file perf_test.hpp
 * @author Andres Rangel
 * @date 13 Jun 2018
 * @brief File contains test for mm_file
 *
 * This file contains test for mm_file
 */

#include <cstring>
#include <fcntl.h>
#include <fmc++/counters.hpp>
#include <fmc/platform.h>
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
extern "C" {
#include "src/mm_file.h"
}

#include <fmc++/gtestwrap.hpp>

using namespace fmc;

TEST(perf_test, main) {
  const char *test_file = "../test/data/sip_small_20171018.csv";

  FILE *pFile;
  char *buffer;

  pFile = fopen(test_file, "r");

  fseek(pFile, 0, SEEK_END);
  int flen = ftell(pFile);

  fclose(pFile);

  mm_file *m = mm_file_open(test_file);

  void *data;

  for (int i = 0; i < flen; ++i) {
    FMC_NANO_EWMA(mm_file_read);
    ASSERT_EQ(mm_file_read(m, sizeof(char), &data), sizeof(char));
  }

  mm_file_rewind(m);

  for (int i = 0; i < flen; ++i) {
    FMC_NANO_AVG(mm_file_read);
    ASSERT_EQ(mm_file_read(m, sizeof(char), &data), sizeof(char));
  }

  mm_file_close(m);

  buffer = (char *)malloc(sizeof(char));

  pFile = fopen(test_file, "r");

  for (int i = 0; i < flen; ++i) {
    FMC_NANO_EWMA(fread);
    ASSERT_EQ(fread(buffer, 1, 1, pFile), 1UL);
  }

  rewind(pFile);

  for (int i = 0; i < flen; ++i) {
    FMC_NANO_AVG(fread);
    ASSERT_EQ(fread(buffer, 1, 1, pFile), 1UL);
  }

  fclose(pFile);
  free(buffer);
}
