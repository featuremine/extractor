/******************************************************************************

        COPYRIGHT (c) 2022 by Featuremine Corporation.
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
 * @file api.cpp
 * @date 4 Oct 2022
 * @brief File contains implementation of yamal sequence API
 *
 * File contains implementation of yamal sequence API
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/api.h"
#include "comp.h"
#include "extractor/comp_sys.h"
#include "extractor/exec_ctx.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_sys.h"
#include "fmc/string.h"
#include <extractor/comp_sys_capture.h>
#include <extractor/frame.h>
}

static struct extractor_api_v1 api_v1 {
  fm_comp_sys_cleanup, fm_type_sys_get, fm_type_sys_err_custom,
      fm_comp_graph_get, fm_stream_ctx_get, fm_stream_ctx_recorded,
      fm_stream_ctx_replayed, fm_stream_ctx_queue, fm_stream_ctx_schedule,
      fm_stream_ctx_scheduled, fm_stream_ctx_idle, fm_stream_ctx_proc_one,
      fm_stream_ctx_next_time, fm_stream_ctx_now, fm_comp_sys_error_msg,
      fm_comp_type_add, fm_comp_find, fm_exec_ctx_is_error,
      fm_exec_ctx_error_msg, fm_exec_ctx_error_set, fm_result_ref_get,
      fm_comp_clbck_set, fm_comp_result_type, fm_type_frame_nfields,
      fm_type_frame_field_name, fm_type_is_base, fm_type_base_enum,
      fm_type_is_cstring, fm_type_tuple_arg, fm_args_empty, fm_type_is_tuple,
      fm_type_tuple_size, fm_frame_get_cptr1, fm_frame_get_ptr1, fm_frame_field,
      fm_data_get, fm_call_def_new, fm_call_def_init_set,
      fm_call_def_destroy_set, fm_call_def_exec_set, fm_ctx_def_new,
      fm_ctx_def_inplace_set, fm_ctx_def_type_set, fm_ctx_def_closure_set,
      fm_ctx_def_volatile_set, fm_ctx_def_queuer_set,
      fm_ctx_def_stream_call_set, fm_ctx_def_query_call_set, fm_ctx_def_closure,
      fm_base_type_get, fm_array_type_get, fm_type_is_array, fm_type_array_size,
      fm_type_array_of, fm_type_to_str, fm_type_frame_field_idx, fm_frame_type,
      fm_type_is_frame, fm_frame_reserve, fm_frame_type_get1,
      fm_type_frame_field_type,

      fm_cstring_type_get, fm_tuple_type_get,

      fm_comp_decl, fm_frame_type_get, fm_type_type_get, fm_type_sys_errmsg,

      fm_comp_sys_ext_path_list_get, fm_comp_sys_paths_set_default,
      fm_comp_sys_ext_path_list_set, fm_comp_sys_ext_path_list_add,
      fm_comp_sys_ext_path_list_del
};

struct extractor_api_v1 *extractor_api_v1_get() {
  return &api_v1;
}
