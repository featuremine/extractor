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
 * @file split_sample_capture.hpp
 * @author Maxim Trokhimtchouk
 * @date 18 Aug 2017
 * @brief File contains tests for split operator
 *
 * Replays the content of messagepack encoded file
 * Validates the context
 * @see http://www.featuremine.com
 */

extern "C" {
#include "comp_sys.h"
#include "comp_sys_capture.h"
#include "frame.h"
#include "std_comp.h"
#include "stream_ctx.h"
#include "type_sys.h"
}

#include <fmc++/gtestwrap.hpp>
#include <fmc/platform.h>
#include <iostream>

using namespace fmc;

void split_sample_clbck(const fm_frame_t *frame, fm_frame_clbck_cl cl,
                        fm_call_ctx_t *) {
  auto *f_cl = (FILE *)cl;
  std::ostringstream s;
  s << fm_time64_to_nanos(*((fm_time64_t *)fm_frame_get_cptr1(
           frame, fm_frame_field(frame, "receive"), 0)))
    << std::endl;
  std::string s_s(s.str());
  fwrite(s_s.data(), sizeof(char), s_s.size(), f_cl);
}

static size_t record_file_writer(const void *data, size_t count,
                                 void *closure) {
  fwrite(data, sizeof(uint8_t), count, (FILE *)closure);
  return count;
}

static bool record_read_bytes(void *data, size_t n, FILE *fh) {
  return fread(data, sizeof(uint8_t), n, fh) == (n * sizeof(uint8_t));
}

static bool replay_file_reader(void *data, size_t limit, void *closure) {
  return record_read_bytes(data, limit, (FILE *)closure);
}

FILE *build_graph(fm_comp_sys_t *sys, fm_comp_graph_t *g, const char *file) {
  auto *tsys = fm_type_sys_get(sys);

  auto cstring_t = fm_cstring_type_get(tsys);
  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, cstring_t, fm_type_type_get(tsys), cstring_t);

  auto *mp_play_param_t = fm_tuple_type_get(
      tsys, 2, cstring_t,
      fm_tuple_type_get(tsys, 7, row_desc_t, row_desc_t, row_desc_t, row_desc_t,
                        row_desc_t, row_desc_t, row_desc_t));

  auto *chararray16 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16);
  auto *chararray32 =
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 32);

  auto *comp_A =
      fm_comp_decl(sys, g, "mp_play", 0, mp_play_param_t,
                   (src_dir + "/data/sip_quotes_20171018.mp").c_str(),
                   "receive", fm_base_type_get(tsys, FM_TYPE_TIME64), "",
                   "ticker", chararray16, "", "market", chararray32, "",
                   "bidprice", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "askprice", fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "",
                   "bidqty", fm_base_type_get(tsys, FM_TYPE_INT32), "",
                   "askqty", fm_base_type_get(tsys, FM_TYPE_INT32), "");

  if (!comp_A)
    return nullptr;

  auto *split_param_t = fm_tuple_type_get(
      tsys, 2, cstring_t,
      fm_tuple_type_get(tsys, 3, cstring_t, cstring_t, cstring_t));

  auto split_op = fm_comp_decl(sys, g, "split", 1, split_param_t, comp_A,
                               "market", "NYSEArca", "NASDAQOMX", "NYSEMKT");

  auto *id1_op = fm_comp_decl(sys, g, "identity", 1, nullptr, split_op);
  auto *id2_op = fm_comp_decl(sys, g, "identity", 1, nullptr, split_op);
  auto *id3_op = fm_comp_decl(sys, g, "identity", 1, nullptr, split_op);
  if (!id1_op || !id2_op || !id3_op)
    return nullptr;

  FILE *fptr_cl = fopen(file, "w");

  fm_comp_clbck_set(id1_op, &split_sample_clbck, (fm_frame_clbck_cl)fptr_cl);
  fm_comp_clbck_set(id2_op, &split_sample_clbck, (fm_frame_clbck_cl)fptr_cl);

  return fptr_cl;
}

TEST(split_sample_capture, record) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new((src_dir + "/test.lic").c_str(), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  fm_comp_sys_std_comp(sys);
  auto *g = fm_comp_graph_get(sys);
  FILE *fptr_cl = build_graph(
      sys, g, (src_dir + "/data/capture_record_clbcks.test.mp").c_str());
  ASSERT_NE(fptr_cl, nullptr);
  FILE *fptr = fopen((src_dir + "/data/capture_record.test.mp").c_str(), "wb");
  ASSERT_NE(fptr, nullptr);
  auto *ctx = fm_stream_ctx_recorded(sys, g, &record_file_writer, fptr);
  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fmc_fflush();
  fclose(fptr);
  fclose(fptr_cl);

  EXPECT_BASE((src_dir + "/data/capture_record_clbcks.test.mp").c_str(),
              (src_dir + "/data/capture_record_clbcks.base.mp").c_str());

  fm_comp_sys_del(sys);
}

TEST(split_sample_capture, replay) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new((src_dir + "/test.lic").c_str(), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  fm_comp_sys_std_comp(sys);
  auto *g = fm_comp_graph_get(sys);
  FILE *fptr_cl = build_graph(
      sys, g, (src_dir + "/data/capture_replay_clbcks.test.mp").c_str());
  ASSERT_NE(fptr_cl, nullptr);
  FILE *fptr = fopen((src_dir + "/data/capture_record.test.mp").c_str(), "rb");
  ASSERT_NE(fptr, nullptr);
  auto *ctx = fm_stream_ctx_replayed(sys, g, &replay_file_reader, fptr);
  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  fmc_fflush();
  fclose(fptr);
  fclose(fptr_cl);

  EXPECT_BASE((src_dir + "/data/capture_replay_clbcks.test.mp").c_str(),
              (src_dir + "/data/capture_record_clbcks.base.mp").c_str());

  fm_comp_sys_del(sys);
}
