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
 * @file api.h
 * @date 4 Oct 2022
 * @brief File contains C declaration of yamal sequence API
 *
 * File contains C declaration of yamal sequence API
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/serial.h"
#include "fmc/time.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fm_comp_sys_t fm_comp_sys_t;
typedef struct fm_type_sys_t fm_type_sys_t;
typedef struct fm_comp_graph_t fm_comp_graph_t;
typedef struct fm_stream_ctx_t fm_stream_ctx_t;
typedef struct fm_comp_t fm_comp_t;
typedef void *fm_frame_clbck_cl;

typedef void (*compsys_unaryfunc) (fm_comp_sys_t *);
typedef fm_type_sys_t* (*compsys_typesys) (fm_comp_sys_t *);
typedef fm_comp_graph_t* (*compsys_graph) (fm_comp_sys_t *);

typedef fm_stream_ctx_t* (*compsys_streamctx) (fm_comp_sys_t *, fm_comp_graph_t *);
typedef fm_stream_ctx_t* (*compsys_streamctx_rec) (fm_comp_sys_t *, fm_comp_graph_t *, fm_writer, void *);

typedef char* (*compsys_errormsg) (fm_comp_sys_t *);

fm_comp_t * (*graph_compfind) (fm_comp_graph_t *, const char*)

bool (*streamctx_procone) (fm_stream_ctx_t *, fmc_time64_t);
fmc_time64_t (*streamctx_nexttime) (fm_stream_ctx_t *);
bool (*streamctx_iserror) (fm_stream_ctx_t *);
const char * (*streamctx_errormsg) (fm_stream_ctx_t *);

typedef void (*fm_frame_clbck_p)(const fm_frame_t *, fm_frame_clbck_cl,
                                 fm_call_ctx_t *);

fm_result_ref_t *(*comp_getref)(fm_comp_t *) {
void (*comp_clbckset)(fm_comp_t *, fm_frame_clbck_p, fm_frame_clbck_cl) {

typedef const struct fm_type_decl *fm_type_decl_cp;
typedef struct fm_frame fm_frame_t;

fm_type_decl_cp (*comp_resulttype)(const fm_comp_t *);
unsigned (*typeframe_nfields)(fm_type_decl_cp);
const char * (*typeframe_fieldname)(fm_type_decl_cp, int);
const void * (*frame_getcptr1) (const fm_frame_t *, fm_field_t, int);

struct extractor_api_v1 {
  // Clean up system
  compsys_unaryfunc cleanup;
  // Get type sys
  compsys_typesys type_sys;
  // Get graph
  compsys_graph graph;
  // Get stream context
  compsys_streamctx stream_ctx;
  // Get recorded stream context
  compsys_streamctx_rec stream_ctx_recorded;
  // Get replayed stream context
  compsys_streamctx_rec stream_ctx_replayed;
  // Get error msg
  compsys_errormsg error_msg;

  // Find computation node in graph
  graph_compfind comp_find;

  // Stream context, process one
  streamctx_procone stream_ctx_proc_one;
  // Stream context, next time
  streamctx_nexttime stream_ctx_next_time;
  // Stream context, is error
  streamctx_iserror stream_ctx_is_error;
  // Stream context, error msg
  streamctx_errormsg stream_ctx_error_msg;

  // Obtain result frame reference from computation
  comp_getref comp_get_ref;

  // Set callback to computation
  comp_clbckset comp_clbck_set;

  // Get computation result type
  comp_resulttype comp_result_type;
  // Get type number of fields
  typeframe_nfields typeframe_nfields;
  // Get type field name
  typeframe_fieldname typeframe_field_name;
  // Get frame data
  frame_getcptr1 frame_get_cptr1;

private:
  fm_comp_t *obj_ = nullptr;

};

// function that you can call to return the actual sequence api structure
// pointer
struct extractor_api_v1 *extractor_api_v1_get();

#ifdef __cplusplus
}
#endif
