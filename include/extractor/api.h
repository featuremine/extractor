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

typedef void *fm_frame_clbck_cl;

typedef struct fm_comp_sys fm_comp_sys_t;
typedef struct fm_type_sys fm_type_sys_t;

/**
 * @brief defines stream execution context object
 * Execution context object is responsible for execution of
 * the compute graph. A specific execution context is first created
 * from a given compute graph. In the process it initializes
 * various operations and creates a call stack for efficient
 * computation of the compute graph. It is also used to communicate
 * the operation call the current execution context of the system,
 * such as the query range or the current time of execution. It
 * also provides ability to access event loop methods in the case
 * of stream execution context.
 */
typedef struct fm_stream_ctx fm_stream_ctx_t;
typedef struct fm_exec_ctx fm_exec_ctx_t;
typedef struct fm_frame fm_frame_t;
typedef struct fm_call_ctx fm_call_ctx_t;
typedef struct fm_ctx_def fm_ctx_def_t;
typedef struct fm_result_ref fm_result_ref_t;
typedef struct fm_comp_graph fm_comp_graph_t;
typedef struct fm_comp fm_comp_t;

typedef void (*fm_frame_clbck_p)(const fm_frame_t *, fm_frame_clbck_cl,
                                 fm_call_ctx_t *);

typedef const struct fm_type_decl *fm_type_decl_cp;

typedef int fm_field_t;
typedef void *fm_comp_def_cl;

typedef struct {
  size_t size;
  char *cursor;
} fm_arg_stack_header_t;

typedef struct {
  fm_arg_stack_header_t header;
  char buffer[1];
} fm_arg_stack_t;

typedef fm_ctx_def_t *(*fm_comp_def_gen)(fm_comp_sys_t *sys, fm_comp_def_cl,
                                         unsigned, fm_type_decl_cp[],
                                         fm_type_decl_cp, fm_arg_stack_t);
typedef void (*fm_comp_def_destroy)(fm_comp_def_cl, fm_ctx_def_t *);

typedef struct {
  const char *name;
  fm_comp_def_gen generate;
  fm_comp_def_destroy destroy;
  fm_comp_def_cl closure;
} fm_comp_def_t;

struct extractor_api_v1 {
  // Clean up system
  void (*comp_sys_cleanup) (fm_comp_sys_t *);
  // Get type sys
  fm_type_sys_t* (*type_sys_get) (fm_comp_sys_t *);
  // Get graph
  fm_comp_graph_t* (*comp_graph_get) (fm_comp_sys_t *);
  // Get stream context
  fm_stream_ctx_t* (*stream_ctx_get) (fm_comp_sys_t *, fm_comp_graph_t *);
  // Get recorded stream context
  fm_stream_ctx_t* (*stream_ctx_recorded) (fm_comp_sys_t *, fm_comp_graph_t *, fm_writer, void *);
  // Get replayed stream context
  fm_stream_ctx_t* (*stream_ctx_replayed) (fm_comp_sys_t *, fm_comp_graph_t *, fm_writer, void *);
  // Get error msg
  char* (*comp_sys_error_msg) (fm_comp_sys_t *);
  // Add computation definition to computational system
  bool (*comp_type_add) (fm_comp_sys_t *, const fm_comp_def_t *def);

  // Find computation node in graph
  fm_comp_t * (*comp_find) (fm_comp_graph_t *, const char*);

  // Stream context, process one
  bool (*stream_ctx_proc_one) (fm_stream_ctx_t *, fmc_time64_t);
  // Stream context, next time
  fmc_time64_t (*stream_ctx_next_time) (fm_stream_ctx_t *);

  // Execution context, is error
  bool (*exec_ctx_is_error) (fm_exec_ctx_t *);
  // Execution context, error msg
  const char * (*exec_ctx_error_msg) (fm_exec_ctx_t *);

  // Obtain result frame reference from computation
  fm_result_ref_t *(*result_ref_get)(fm_comp_t *);
  // Set callback to computation
  void (*comp_clbck_set)(fm_comp_t *, fm_frame_clbck_p,
                         fm_frame_clbck_cl);

  // Get computation result type
  fm_type_decl_cp (*comp_result_type)(const fm_comp_t *);
  // Get type number of fields
  unsigned (*type_frame_nfields)(fm_type_decl_cp);
  // Get type field name
  const char *(*type_frame_field_name)(fm_type_decl_cp, int);
  // Get frame data
  const void *(*frame_get_cptr1)(const fm_frame_t *, fm_field_t, int);
  // Get frame field
  fm_field_t (*frame_field)(const fm_frame_t *, const char *);
  // Get result reference data
  fm_frame_t *(*data_get)(fm_result_ref_t *);
};

// function that you can call to return the actual sequence api structure
// pointer
struct extractor_api_v1 *extractor_api_v1_get();

#ifdef __cplusplus
}
#endif
