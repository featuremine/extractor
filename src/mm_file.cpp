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
 * @file mm_file.cpp
 * @author Andres Rangel
 * @date 13 Jun 2018
 * @brief File contains implementation of mm_file structure and API
 *
 * This file contains implementation of mm_file structure and API
 * @see http://www.featuremine.com
 */

extern "C" {
#include "src/mm_file.h"
}

#include <fcntl.h>
#include <fmc/platform.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef FM_SYS_WIN
#include <sys/mman.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>

#include <cstring>
#include <fmc++/counters.hpp>
#include <fmc/files.h>
#include <iostream>
#include <string_view>

struct mm_file {
  void *data_;
  size_t offset_;
  size_t max_len_;
  fmc_fd fd_;
  fmc_fview_t view_;
};

mm_file_t *mm_file_open(const char *file_path) {
  fmc_error_t *err = nullptr;
  fmc_fd fd = fmc_fopen(file_path, fmc_fmode::READ, &err);
  if (err || !fmc_fvalid(fd)) {
    return NULL;
  }

  size_t max_len = fmc_fsize(fd, &err);
  if (err) {
    return NULL;
  }

  fmc_fview_t view;
  fmc_fview_init(&view, max_len, fd, 0, &err);
  if (err) {
    fmc_error_t *err = nullptr;
    fmc_fclose(fd, &err);
    return NULL;
  }

  void *data = fmc_fview_data(&view);
  if (!data) {
    fmc_fview_destroy(&view, max_len, &err);
    fmc_fclose(fd, &err);
    return NULL;
  }

  return new mm_file_t{data, 0, max_len, fd, view};
};

size_t mm_file_read(mm_file *m, size_t limit, void **buf) {
  if (m->offset_ == m->max_len_)
    return 0;

  *buf = (void *)((char *)m->data_ + m->offset_);

  if (m->offset_ + limit > m->max_len_) {
    size_t ret = m->max_len_ - m->offset_;
    m->offset_ += ret;
    return ret;
  }
  m->offset_ += limit;
  return limit;
}

void mm_file_rewind(mm_file_t *m) { m->offset_ = 0; };

void mm_file_close(mm_file_t *m) {
  fmc_error_t *err = nullptr;
  fmc_fview_destroy(&m->view_, m->max_len_, &err);
  fmc_fclose(m->fd_, &err);
  delete m;
};
