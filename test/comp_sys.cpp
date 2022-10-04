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

extern "C" {
#include "comp_sys.h"
#include "comp_def_simp.h"
#include "frame.h"
#include "src/comp.h"
#include "stream_ctx.h"
#include "type_sys.h"
}

#include <fmc++/gtestwrap.hpp>

#include <string>

std::string src_dir;

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

fm_frame_t *fm_frame_from_type(fm_frame_alloc_t *alloc, fm_type_decl_cp type) {
  auto *obj = new fm_frame;
  alloc->array.push_back(obj);
  return obj;
}

extern "C" {
fm_type_decl_cp char_comp_type(fm_type_sys_t *, unsigned, fm_type_decl_cp[]);
fm_type_decl_cp char_comp_type_1(fm_type_sys_t *, unsigned, fm_type_decl_cp[]);
fm_type_decl_cp char_comp_type_2(fm_type_sys_t *, unsigned, fm_type_decl_cp[]);
bool char_context_new(fm_ctx_def_cl *cl, fm_type_sys_t *sys,
                      fm_type_decl_cp ptype, fm_arg_stack_t plist);
void char_context_del(fm_ctx_def_cl cxl);
bool timer_init(fm_frame_t *, size_t, const fm_frame_t *const argv[],
                fm_call_ctx_t *ctx, fm_call_exec_cl *);
bool timer_exec(fm_frame_t *, size_t, const fm_frame_t *const argv[],
                fm_call_ctx_t *ctx, fm_call_exec_cl closure);
bool append_exec(fm_frame_t *frame, size_t argc, const fm_frame_t *const *argv,
                 fm_call_ctx_t *ctx, fm_call_exec_cl cl);
bool combine_exec(fm_frame_t *frame, size_t argc, const fm_frame_t *const *argv,
                  fm_call_ctx_t *ctx, fm_call_exec_cl cl);
bool mirror_exec(fm_frame_t *frame, size_t argc, const fm_frame_t *const *argv,
                 fm_call_ctx_t *ctx, fm_call_exec_cl cl);
void mirror_clbck(const fm_frame_t *frame, fm_frame_clbck_cl cl,
                  fm_call_ctx_t *);
} // extern "C"

fm_type_decl_cp char_comp_type(fm_type_sys_t *sys, unsigned argc,
                               fm_type_decl_cp argv[]) {
  auto ctype = fm_base_type_get(sys, FM_TYPE_CHAR);
  return fm_frame_type_get(sys, 1, 1, "name", ctype, 1);
}

fm_type_decl_cp char_comp_type_0(fm_type_sys_t *sys, unsigned argc,
                                 fm_type_decl_cp argv[]) {
  if (argc != 0)
    return nullptr;
  return char_comp_type(sys, argc, argv);
}

fm_type_decl_cp char_comp_type_1(fm_type_sys_t *sys, unsigned argc,
                                 fm_type_decl_cp argv[]) {
  if (argc != 1)
    return nullptr;
  return char_comp_type(sys, argc, argv);
}

bool char_context_new(fm_ctx_def_cl *cl, fm_type_sys_t *sys,
                      fm_type_decl_cp ptype, fm_arg_stack_t plist) {
  if (ptype == nullptr) {
    return false;
  }
  auto type = fm_base_type_get(sys, FM_TYPE_CHAR);
  if (!fm_type_equal(ptype, type)) {
    return false;
  }
  char tag = STACK_POP(plist, char);
  *cl = new std::string(1, tag);
  return true;
}

void char_context_del(fm_ctx_def_cl closure) { delete (std::string *)closure; }

bool timer_init(fm_frame_t *, size_t, const fm_frame_t *const argv[],
                fm_call_ctx_t *ctx, fm_call_exec_cl *) {
  auto *exec = (fm_stream_ctx_t *)ctx->exec;
  fm_stream_ctx_queue(exec, ctx->handle);
  return true;
}

bool timer_exec(fm_frame_t *frame, size_t argc, const fm_frame_t *const argv[],
                fm_call_ctx_t *ctx, fm_call_exec_cl closure) {
  auto *exec = (fm_stream_ctx *)ctx->exec;
  auto &comp = *(std::string *)ctx->comp;
  auto now = fm_stream_ctx_now(exec);
  auto time = fm_time64_from_raw(fm_time64_raw(now) + 1);
  fm_stream_ctx_schedule(exec, ctx->handle, time);
  frame->data = comp + std::to_string(fm_time64_raw(time));
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
  for (unsigned i = 0; i < argc; ++i) {
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

FM_COMP_DEF_SIMP(timer_comp, "timer", false, &char_comp_type_0, &timer_exec,
                 &timer_exec, &timer_init, nullptr, nullptr, &char_context_new,
                 &char_context_del);

FM_COMP_DEF_SIMP(append_comp, "append", true, &char_comp_type_1, &append_exec,
                 &append_exec, nullptr, nullptr, nullptr, &char_context_new,
                 &char_context_del);

FM_COMP_DEF_SIMP(combine_comp, "combine", false, &char_comp_type, &combine_exec,
                 &combine_exec, nullptr, nullptr, nullptr, nullptr, nullptr);

FM_COMP_DEF_SIMP(mirror_comp, "mirror", true, &char_comp_type_1, &mirror_exec,
                 &mirror_exec, nullptr, nullptr, nullptr, nullptr, nullptr);

TEST(comp_sys, main) {
  using namespace std;

  vector<string> baseval = {"A1CB1DF", "A2CB2DF",  "A3CB3DF", "A4CB4DF",
                            "A5CB5DF", "A6CB6DF",  "A7CB7DF", "A8CB8DF",
                            "A9CB9DF", "A10CB10DF"};

  string basestr;
  for (auto &str : baseval)
    basestr += str;

  std::string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new((src_dir + "/test.lic").c_str(), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  fm_comp_type_add(sys, &timer_comp);
  fm_comp_type_add(sys, &append_comp);
  fm_comp_type_add(sys, &combine_comp);
  fm_comp_type_add(sys, &mirror_comp);

  auto *g = fm_comp_graph_get(sys);
  auto *tsys = fm_type_sys_get(sys);
  auto ctype = fm_base_type_get(tsys, FM_TYPE_CHAR);

  auto *comp_A = fm_comp_decl(sys, g, "timer", 0, ctype, 'A');
  ASSERT_NE(comp_A, nullptr);
  auto *comp_B = fm_comp_decl(sys, g, "timer", 0, ctype, 'B');
  ASSERT_NE(comp_B, nullptr);
  auto *comp_C = fm_comp_decl(sys, g, "append", 1, ctype, comp_A, 'C');
  ASSERT_NE(comp_C, nullptr);
  auto *comp_D = fm_comp_decl(sys, g, "append", 1, ctype, comp_B, 'D');
  ASSERT_NE(comp_D, nullptr);
  auto *comp_E = fm_comp_decl(sys, g, "combine", 2, NULL, comp_C, comp_D);
  ASSERT_NE(comp_E, nullptr);
  auto *comp_F = fm_comp_decl(sys, g, "append", 1, ctype, comp_E, 'F');
  ASSERT_NE(comp_F, nullptr);
  auto *comp_G = fm_comp_decl(sys, g, "mirror", 1, NULL, comp_F);
  ASSERT_NE(comp_G, nullptr);

  fm_comp_clbck_set(comp_G, &mirror_clbck, &testout);
  auto *res_ref = fm_result_ref_get(comp_F);

  auto *ctx = fm_stream_ctx_get(sys, g);

  fm_time64_t now = fm_time64_from_raw(0);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    // @note add check for data correction
    auto *frame = fm_data_get(res_ref);
    EXPECT_EQ(frame->data, baseval[fm_time64_raw(now)]);

    now = fm_stream_ctx_next_time(ctx);
  } while (fm_time64_raw(now) < 10);

  fm_comp_sys_del(sys);

  ASSERT_EQ(testout, basestr);
}

GTEST_API_ int main(int argc, char **argv) {
  testing::InitGoogleTest(&argc, argv);
  src_dir = argv[1];
  return RUN_ALL_TESTS();
}
