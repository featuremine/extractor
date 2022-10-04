extern "C" {
#include "comp_sys.h"
#include "frame.h"
#include "std_comp.h"
#include "stream_ctx.h"
#include "type_sys.h"
}

#include <cassert>
#include <fstream>
#include <iostream>
#include <vector>

#define EXPECT_TRUE(e) assert((e) == true)
#define ASSERT_TRUE(e) assert((e) == true)
#define ASSERT_FALSE(e) assert((e) == false)
#define ASSERT_NE(a, b) assert((a) != (b))
#define ASSERT_EQ(a, b) assert((a) == (b))

#include <fmc/test.h>

using namespace std;

void accumulate_data(const string &licence_filename) {
  string testout;

  char *errstring;
  auto *sys = fm_comp_sys_new(licence_filename.c_str(), &errstring);
  if (!sys) {
    cout << errstring << endl;
    free(errstring);
  }
  ASSERT_NE(sys, nullptr);

  fm_comp_sys_std_comp(sys);

  auto *g = fm_comp_graph_get(sys);

  auto *tsys = fm_type_sys_get(sys);

  auto *row_desc_t =
      fm_tuple_type_get(tsys, 3, fm_cstring_type_get(tsys),
                        fm_type_type_get(tsys), fm_cstring_type_get(tsys));

  auto *csv_play_param_t = fm_tuple_type_get(
      tsys, 2, fm_cstring_type_get(tsys),
      fm_tuple_type_get(tsys, 8, row_desc_t, row_desc_t, row_desc_t, row_desc_t,
                        row_desc_t, row_desc_t, row_desc_t, row_desc_t));

  auto *comp_A = fm_comp_decl(
      sys, g, "csv_play", 0, csv_play_param_t, "pandas_play_file.csv",
      "receive", fm_base_type_get(tsys, FM_TYPE_TIME64), "", "market",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16), "",
      "ticker",
      fm_array_type_get(tsys, fm_base_type_get(tsys, FM_TYPE_CHAR), 16), "",
      "type", fm_base_type_get(tsys, FM_TYPE_CHAR), "", "bidprice",
      fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "", "askprice",
      fm_base_type_get(tsys, FM_TYPE_DECIMAL64), "", "bidqty",
      fm_base_type_get(tsys, FM_TYPE_INT32), "", "askqty",
      fm_base_type_get(tsys, FM_TYPE_INT32), "");

  ASSERT_NE(comp_A, nullptr);
  auto *comp_B = fm_comp_decl(sys, g, "accumulate", 1, nullptr, comp_A);

  ASSERT_NE(comp_B, nullptr);

  auto *ctx = fm_stream_ctx_get(sys, g);

  ASSERT_NE(ctx, nullptr);

  fm_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    fm_stream_ctx_proc_one(ctx, now);
    now = fm_stream_ctx_next_time(ctx);
  } while (!fm_time64_is_end(now));

  auto *result = fm_data_get(fm_result_ref_get(comp_B));

  ASSERT_NE(result, nullptr);

  auto f_decl = fm_frame_type(result);

  ASSERT_NE(f_decl, nullptr);

  auto r_field = fm_type_frame_field_idx(f_decl, "receive");
  auto m_field = fm_type_frame_field_idx(f_decl, "market");
  auto tick_field = fm_type_frame_field_idx(f_decl, "ticker");
  auto t_field = fm_type_frame_field_idx(f_decl, "type");
  auto bp_field = fm_type_frame_field_idx(f_decl, "bidprice");
  auto ap_field = fm_type_frame_field_idx(f_decl, "askprice");
  auto bq_field = fm_type_frame_field_idx(f_decl, "bidqty");
  auto aq_field = fm_type_frame_field_idx(f_decl, "askqty");

  FILE *f = fopen("accumulate.test.csv", "w");
  fprintf(f, "%s",
          "receive,ticker,market,type,bidprice,askprice,bidqty,"
          "askqty\n");
  for (int i = 0; i < fm_frame_dim(result, 0); ++i) {
    fprintf(
        f, "%ld,%s,%s,%c,%g,%g,%d,%d\n",
        fm_time64_to_nanos(
            *(fm_time64_t *)fm_frame_get_cptr1(result, r_field, i)),
        string((char *)fm_frame_get_cptr1(result, tick_field, i), 16).c_str(),
        string((char *)fm_frame_get_cptr1(result, m_field, i), 16).c_str(),
        *(char *)fm_frame_get_cptr1(result, t_field, i),
        fm_decimal64_to_double(
            *(fm_decimal64_t *)fm_frame_get_cptr1(result, bp_field, i)),
        fm_decimal64_to_double(
            *(fm_decimal64_t *)fm_frame_get_cptr1(result, ap_field, i)),
        *(int32_t *)fm_frame_get_cptr1(result, bq_field, i),
        *(int32_t *)fm_frame_get_cptr1(result, aq_field, i));
  }
  fclose(f);

  fm_comp_sys_del(sys);

  EXPECT_BASE("accumulate.base.csv", "accumulate.test.csv");
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Usage:" << std::endl
              << " " << argv[0] << " <licence_file>" << std::endl;
    return -1;
  }
  accumulate_data(argv[1]);
  return 0;
}
