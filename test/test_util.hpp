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
 * @file test_util.cpp
 * @author Andres Rangel
 * @date 22 Jan 2019
 * @brief String reader and writer for serialization tests
 *
 * @see http://www.featuremine.com
 */

#pragma once

#include <fmc/platform.h>
#include <string>

using namespace std;

static size_t string_writer(const void *data, size_t count, void *closure) {
  auto *str = (string *)closure;
  str->append((const char *)data, count);
  return count;
}

static bool string_view_reader(void *data, size_t limit, void *closure) {
  auto *view = (string_view *)closure;
  if (view->size() < limit)
    return false;
  memcpy(data, view->data(), limit);
  *view = view->substr(limit);
  return true;
}
