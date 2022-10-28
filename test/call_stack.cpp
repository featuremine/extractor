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
 * @file call_stack.cpp
 * @author Maxim Trokhimtchouk
 * @date 8 Aug 2017
 * @brief File contains tests for call stack object
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "call_stack.h"
}

#include "fmc++/gtestwrap.hpp"
#include "fmc/alignment.h"
#include "unique_pq.hpp"

#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

// Buffer for storing call output for testing
using call_buffer = std::string;

// For the test we implement the call object that simply stores a string
// and prints a string and appends it to buffer
struct fm_call_obj {
  bool execute;
  size_t size;
  call_buffer *buf;
  char str[];
};

void fm_call_obj_copy(void *ptr, fm_call_obj_t *obj) {
  auto *ptr_obj = new (ptr) fm_call_obj(*obj);
  memcpy((void *)ptr_obj->str, obj->str, ptr_obj->size);
}

size_t fm_call_obj_size(fm_call_obj_t *obj) {
  return sizeof(fm_call_obj) + obj->size;
}

bool fm_call_obj_exec(fm_call_obj_t *obj) {
  auto &buf = *obj->buf;
  if (!obj->execute)
    return false;
  buf.insert(buf.size(), obj->str, obj->size);
  return true;
}

void fm_call_obj_deps_queue(fm_call_obj_t *obj) {}

fm_call_obj *fm_call_obj_new(const char *cstr, call_buffer &buf) {
  auto slen = strlen(cstr);
  auto *obj = (fm_call_obj *)malloc(sizeof(fm_call_obj) + slen);
  if (!obj)
    return nullptr;
  obj->execute = true;
  obj->size = slen;
  obj->buf = &buf;
  memcpy(obj->str, cstr, slen);

  return obj;
}

void fm_call_obj_cleanup(fm_call_obj_t *obj) {}

void fm_call_obj_del(fm_call_obj *obj) { free(obj); }

TEST(call_stack, wordceil) {
  size_t s = sizeof(int *);
  for (size_t i = 0; i < (1 << 5); ++i) {
    auto r1 = size_t(ceil(double(i) / double(s)) * s);
    ASSERT_EQ(fmc_wordceil(i), r1);
  }
}

TEST(call_stack, unique_pq) {
  using namespace fm;
  using namespace fm;
  using namespace std;
  unique_pq<int> pq;
  int n = 50;
  std::vector<int> v(n);
  std::iota(begin(v), end(v), 0);
  random_device rd;
  mt19937 g(rd());
  shuffle(v.begin(), v.end(), g);
  for (auto x : v)
    pq.push(x);
  for (int i = 0; i < n; ++i) {
    ASSERT_FALSE(pq.empty());
    auto x = pq.pop();
    ASSERT_EQ(i, x);
  }
  ASSERT_TRUE(pq.empty());
}

TEST(call_stack, main) {
  using namespace std;
  call_buffer buf;
  unordered_map<string, fm_call_obj *> objs;
  objs.emplace("A", fm_call_obj_new("A", buf));
  objs.emplace("B", fm_call_obj_new("B", buf));
  objs.emplace("C", fm_call_obj_new("C", buf));
  objs.emplace("D", fm_call_obj_new("D", buf));
  objs.emplace("E", fm_call_obj_new("E", buf));
  objs.emplace("F", fm_call_obj_new("F", buf));
  objs.emplace("G", fm_call_obj_new("G", buf));

  auto *s = fm_call_stack_new();

  fm_call_handle_t dep_A[2];
  fm_call_handle_t dep_B[2];
  fm_call_handle_t dep_C[1];
  fm_call_handle_t dep_D[1];
  fm_call_handle_t dep_E[1];
  fm_call_handle_t dep_F[1];
  fm_call_handle_t dep_G[1];

  auto offset_G = fm_call_stack_push(s, objs["G"], 0, dep_G);

  dep_F[0] = offset_G;
  auto offset_F = fm_call_stack_push(s, objs["F"], 1, dep_F);

  dep_E[0] = offset_G;
  auto offset_E = fm_call_stack_push(s, objs["E"], 1, dep_E);

  dep_D[0] = offset_G;
  auto offset_D = fm_call_stack_push(s, objs["D"], 1, dep_D);

  dep_C[0] = offset_D;
  auto offset_C = fm_call_stack_push(s, objs["C"], 1, dep_C);

  dep_B[0] = offset_E;
  dep_B[1] = offset_F;
  auto offset_B = fm_call_stack_push(s, objs["B"], 2, dep_B);

  dep_A[0] = offset_B;
  dep_A[1] = offset_C;
  auto offset_A = fm_call_stack_push(s, objs["A"], 2, dep_A);

  unordered_map<string, fm_call_handle_t> offsets = {
      {"A", offset_A}, {"B", offset_B}, {"C", offset_C}, {"D", offset_D},
      {"E", offset_E}, {"F", offset_F}, {"G", offset_G}};

  unordered_map<string, string> correct_results = {
      {"A", "ABCDEFG"}, {"B", "BEFG"}, {"C", "CDG"}, {"D", "DG"},
      {"E", "EG"},      {"F", "FG"},   {"G", "G"}};

  auto *q = fm_call_queue_new();

  for (auto &p : offsets) {
    buf.clear();
    fm_call_queue_push(q, p.second);
    fm_call_stack_exec(s, q);
    EXPECT_EQ(buf, correct_results[p.first]);
  }

  fm_call_queue_del(q);
  fm_call_stack_del(s);
  for (auto &p : objs)
    fm_call_obj_del(p.second);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
