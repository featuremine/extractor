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
 * @file frame_pool.cpp
 * @author Andres Rangel
 * @date Jan 28 2019
 * @brief A pool to allocate Extractor Frames
 *
 * File contains a C++ implementation of a frame pool
 * @see http://www.featuremine.com
 */

extern "C" {
#include "frame_base.h"
}

#include <vector>

class frame_pool {
public:
  frame_pool() { alloc_ = fm_frame_alloc_new(); }
  ~frame_pool() { fm_frame_alloc_del(alloc_); }
  fm_frame_t *get(const fm_frame_t *f) {
    if (avail_.empty()) {
      return fm_frame_alloc_clone(alloc_, f);
    }
    fm_frame_t *ret = avail_.back();
    avail_.pop_back();
    return ret;
  }
  void release(fm_frame_t *f) { avail_.push_back(f); }

private:
  std::vector<fm_frame_t *> avail_;
  fm_frame_alloc_t *alloc_;
};
