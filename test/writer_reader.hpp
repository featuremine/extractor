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
 * @file writer_reader.hpp
 * @author Andres Rangel
 * @date 30 Jul 2018
 * @brief File contains tests for reader and writer
 *
 * Uses reader_read and writer_write utilities
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/comp_sys.h"
#include "extractor/frame.h"
#include "extractor/std_comp.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_sys.h"
#include "frame_serial.h"
}

#include "fmc++/gtestwrap.hpp"
#include "fmc/platform.h"
#include <iostream>

using namespace fmc;

static bool read_bytes(void *data, size_t sz, FILE *fh) {
  return fread(data, sizeof(uint8_t), sz, fh) == (sz * sizeof(uint8_t));
}

static bool file_reader(void *data, size_t count, void *closure) {
  std::cout << "File reader reading " << count << "bytes" << std::endl;
  return read_bytes(data, count, (FILE *)closure);
}

static size_t file_writer(const void *data, size_t count, void *closure) {
  return fwrite(data, sizeof(uint8_t), count, (FILE *)closure);
}

TEST(writer_reader, check) {
  using namespace std;

  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new(&errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  fm_comp_sys_std_comp(sys);

  auto *g = fm_comp_graph_get(sys);
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

  ASSERT_NE(comp_A, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);
  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);

    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  auto *f = fm_data_get(fm_result_ref_get(comp_A));
  std::cout << "f obtained" << std::endl;
  auto type = fm_frame_type(f);

  std::cout << "type of f is: " << fm_type_to_str(type);
  auto *other_f_container = fm_frame_alloc_new();

  auto *other_f = fm_frame_from_type(other_f_container, type);
  std::cout << "other_f created" << std::endl;
  std::cout << "type of other_f is: " << fm_type_to_str(fm_frame_type(other_f));

  FILE *file =
      fopen((src_dir + "/data/frame_read_write_f.test.mp").c_str(), "wb");
  auto *writer = fm_frame_writer_new(type, &file_writer, file);

  std::cout << "after new writer" << std::endl;
  ASSERT_TRUE(fm_frame_writer_write(writer, f));

  std::cout << "after writer write" << std::endl;
  fm_frame_writer_del(writer);

  fclose(file);

  FILE *reader_file =
      fopen((src_dir + "/data/frame_read_write_f.test.mp").c_str(), "rb");
  auto *reader = fm_frame_reader_new(type, &file_reader, reader_file);

  std::cout << "after new reader" << std::endl;
  ASSERT_TRUE(fm_frame_reader_read(reader, other_f));

  fm_frame_reader_del(reader);
  fclose(reader_file);

  FILE *other_file =
      fopen((src_dir + "/data/frame_read_write_other_f.test.mp").c_str(), "wb");
  writer = fm_frame_writer_new(type, &file_writer, other_file);

  std::cout << "after new writer" << std::endl;
  ASSERT_TRUE(fm_frame_writer_write(writer, other_f));

  fm_frame_writer_del(writer);
  fclose(other_file);

  EXPECT_BASE((src_dir + "/data/frame_read_write.base.mp").c_str(),
              (src_dir + "/data/frame_read_write_f.test.mp").c_str());
  EXPECT_BASE((src_dir + "/data/frame_read_write.base.mp").c_str(),
              (src_dir + "/data/frame_read_write_other_f.test.mp").c_str());

  fm_frame_alloc_del(other_f_container);
}
