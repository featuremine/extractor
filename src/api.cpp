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
#include "extractor/comp_sys.h"
#include "extractor/type_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/exec_ctx.h"
#include <extractor/comp_sys_capture.h>
#include "comp.h"
}

static struct extractor_api_v1 api_v1 {
  fm_comp_sys_cleanup,
  fm_type_sys_get,
  fm_comp_graph_get,
  fm_stream_ctx_get,
  fm_stream_ctx_recorded,
  fm_stream_ctx_replayed,
  fm_comp_sys_error_msg,
  fm_comp_type_add,
  fm_comp_find,
  fm_stream_ctx_proc_one,
  fm_stream_ctx_next_time,
  fm_exec_ctx_is_error,
  fm_exec_ctx_error_msg,
  fm_result_ref_get,
  fm_comp_clbck_set,
  fm_comp_result_type,
  fm_type_frame_nfields,
  fm_type_frame_field_name,
  fm_frame_get_cptr1,
  fm_frame_field,
  fm_data_get
};

struct extractor_api_v1 *extractor_api_v1_get() {
  return &api_v1;
}
