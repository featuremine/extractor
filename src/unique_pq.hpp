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
 * @file unique_pq.hpp
 * @author Maxim Trokhimtchouk
 * @date 8 Aug 2017
 * @brief File contains C++ implementation of the call stack object
 *
 * This file contains declarations of the call stack interface
 * @see http://www.featuremine.com
 */

#pragma once

#include <functional>
#include <vector>

namespace fm {
template <class T, class Container = std::vector<T>,
          class Compare = std::less<typename Container::value_type>>
struct unique_pq {
  using value_type = typename Container::value_type;
  struct Rev_Compare {
    bool operator()(const value_type &a, const value_type &b) {
      return Compare()(b, a);
    }
  };
  T pop() {
    T x = queue.back();
    queue.pop_back();
    return x;
  }

  T &top() { return queue.back(); }

  void push(T x) {
    Compare cmp;
    if (queue.empty() || cmp(x, queue.back())) {
      queue.push_back(x);
      return;
    } else if (!cmp(queue.back(), x)) {
      return;
    } else {
      Rev_Compare rcmp;
      auto where = std::lower_bound(queue.begin(), queue.end(), x, rcmp);
      if (!rcmp(x, *where))
        return;
      queue.insert(where, x);
    }
  }

  bool empty() { return queue.empty(); }

  Container queue;
};
} // namespace fm
