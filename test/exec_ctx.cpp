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
 * @file exec_ctx.cpp
 * @author Maxim Trokhimtchouk
 * @date 18 Aug 2017
 * @brief File contains tests for execution context
 *
 * @see http://www.featuremine.com
 */

#include "call_obj.h"
#include "comp.h"
#include "comp_base.h"
#include "comp_graph.h"
#include "extractor/frame_base.h"
#include "stream_ctx.h"

#include "fmc++/gtestwrap.hpp"
#include <string>

// @note implement test frame base interface
struct fm_frame {
  std::string data;
};

struct fm_frame_alloc {
  ~fm_frame_alloc() {
    for (auto *item : array)
      delete item;
  }
  std::vector<fm_frame_t *> array;
};

void fm_frame_clone_copy(fm_frame_t *dest, const fm_frame_t *src) {

  dest->data = src->data;
}

fm_frame_alloc_t *fm_frame_alloc_new() { return new fm_frame_alloc(); }

void fm_frame_alloc_del(fm_frame_alloc_t *obj) { delete obj; }

fm_frame_t *fm_frame_alloc_clone(fm_frame_alloc_t *alloc,
                                 const fm_frame_t *src) {
  auto *frame = new fm_frame_t;
  frame->data = src->data;
  alloc->array.push_back(frame);
  return alloc->array.back();
}

fm_frame_t *fm_frame_alloc_get(fm_frame_alloc_t *alloc) {
  alloc->array.push_back(new fm_frame_t());
  return alloc->array.back();
}

// @note comp_obj test implementation
struct fm_comp {
  fm_call_exec_p exec = nullptr;
  std::vector<fm_comp_clbck_t> clbcks;
  std::string ctx;
  int16_t inplace = false;
  int16_t volatile_result = false;
  bool required = false;
  bool schedule = false;
};

const char *fm_comp_type(const fm_comp_t *comp) { return "computation"; }

fm_call_obj_t *fm_stream_call_obj_new(fm_comp_t *comp, fm_exec_ctx_p ctx,
                                      unsigned argc) {
  auto *call = fm_call_obj_new(argc);
  fm_call_obj_exec_ctx_set(call, ctx);
  fm_call_obj_comp_ctx_set(call, &comp->ctx);
  fm_call_obj_exec_set(call, comp->exec, nullptr);
  for (auto &clbck : comp->clbcks)
    fm_call_obj_clbck_set(call, clbck.clbck, clbck.cl);
  return call;
}

bool fm_comp_inplace(const fm_comp_t *obj) { return obj->inplace; }

bool fm_comp_volatile(const fm_comp_t *obj) { return obj->volatile_result; }

bool fm_comp_data_required(const fm_comp_t *obj) { return obj->required; }

fm_frame_t *fm_comp_frame_mk(const fm_comp_t *obj, fm_frame_alloc_t *alloc) {
  return fm_frame_alloc_get(alloc);
}

bool fm_comp_call_init(fm_comp_t *comp, fm_call_obj_t *call) {
  if (comp->schedule) {
    auto *exec = (fm_stream_ctx *)fm_call_obj_exec_ctx(call);
    auto handle = fm_call_obj_handle(call);
    fm_stream_ctx_queue(exec, handle);
  }
  return true;
}

void fm_comp_call_destroy(fm_comp_t *) {}

void fm_comp_del(fm_comp_t *obj) { delete obj; }

bool timer_exec(fm_frame_t *frame, size_t argc, const fm_frame_t *const *argv,
                fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *exec = (fm_stream_ctx *)ctx->exec;
  auto &comp = *(std::string *)ctx->comp;
  auto now = fm_stream_ctx_now(exec);
  auto time = fmc_time64_from_raw(fmc_time64_raw(now) + 1);
  fm_stream_ctx_schedule(exec, ctx->handle, time);
  frame->data = comp + std::to_string(fmc_time64_raw(time));
  return true;
}

bool append_exec(fm_frame_t *frame, size_t argc, const fm_frame_t *const *argv,
                 fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto &comp = *(std::string *)ctx->comp;
  frame->data = argv[0]->data + comp;
  return true;
}

bool combine_exec(fm_frame_t *frame, size_t argc, const fm_frame_t *const *argv,
                  fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  frame->data.clear();
  for (size_t i = 0; i < argc; ++i) {
    frame->data += argv[i]->data;
  }
  return true;
}

bool mirror_exec(fm_frame_t *frame, size_t argc, const fm_frame_t *const *argv,
                 fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  return true;
}

void mirror_clbck(const fm_frame_t *frame, fm_frame_clbck_cl cl,
                  fm_call_ctx_t *) {
  std::string &str = *(std::string *)cl;
  str.append(frame->data);
}

fm_comp_t *timer_comp_obj(const std::string &name) {
  auto *obj = new fm_comp();
  obj->exec = &timer_exec;
  obj->ctx = name;
  obj->inplace = false;
  obj->required = false;
  obj->schedule = true;
  return obj;
}

fm_comp_t *append_comp_obj(const std::string &name, bool required) {
  auto *obj = new fm_comp();
  obj->exec = &append_exec;
  obj->ctx = name;
  obj->inplace = true;
  obj->required = required;
  obj->schedule = false;
  return obj;
}

fm_comp_t *combine_comp_obj() {
  auto *obj = new fm_comp();
  obj->exec = &combine_exec;
  obj->ctx = "";
  obj->inplace = false;
  obj->required = false;
  obj->schedule = false;
  return obj;
}

fm_comp_t *mirror_comp_obj(std::string &outstr) {
  auto *obj = new fm_comp();
  obj->exec = &mirror_exec;
  obj->clbcks.emplace_back(fm_comp_clbck_t{&mirror_clbck, &outstr});
  obj->ctx = "";
  obj->inplace = true;
  obj->required = false;
  obj->schedule = false;
  return obj;
}

TEST(exec_ctx, main) {
  using namespace std;

  std::string testout;
  auto *g = fm_comp_graph_new();

  fm_comp_node_t *inps_C[1];
  fm_comp_node_t *inps_D[1];
  fm_comp_node_t *inps_E[2];
  fm_comp_node_t *inps_F[1];
  fm_comp_node_t *inps_G[1];

  auto *node_A = fm_comp_graph_add(g, timer_comp_obj("A"), 0, NULL);

  auto *node_B = fm_comp_graph_add(g, timer_comp_obj("B"), 0, NULL);

  inps_C[0] = node_A;
  auto *node_C = fm_comp_graph_add(g, append_comp_obj("C", 0), 1, inps_C);

  inps_D[0] = node_B;
  auto *node_D = fm_comp_graph_add(g, append_comp_obj("D", 0), 1, inps_D);

  inps_E[0] = node_C;
  inps_E[1] = node_D;
  auto *node_E = fm_comp_graph_add(g, combine_comp_obj(), 2, inps_E);

  inps_F[0] = node_E;
  auto *node_F = fm_comp_graph_add(g, append_comp_obj("F", 1), 1, inps_F);

  inps_G[0] = node_F;
  fm_comp_graph_add(g, mirror_comp_obj(testout), 1, inps_G);

  ASSERT_TRUE(fm_comp_graph_stable_top_sort(g));

  auto *ctx = fm_stream_ctx_new(g);

  fmc_time64_t now = fmc_time64_from_raw(0);
  do {
    fm_stream_ctx_proc_one(ctx, now);
    now = fm_stream_ctx_next_time(ctx);
  } while (fmc_time64_raw(now) < 10);

  fm_stream_ctx_del(ctx);
  fm_comp_graph_del(g);

  auto *basestr = "A1CB1DFA2CB2DFA3CB3DFA4CB4DFA5CB5DF"
                  "A6CB6DFA7CB7DFA8CB8DFA9CB9DFA10CB10DF";

  ASSERT_EQ(testout, basestr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
