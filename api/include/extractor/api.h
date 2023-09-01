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

#include "fmc/decimal128.h"
#include "fmc/rational64.h"
#include "fmc/rprice.h"
#include "fmc/time.h"
#include <fmc/extension.h>

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

/**
 * @brief enum for type systems errors
 */
typedef enum {
  FM_TYPE_ERROR_OK = 0,
  FM_TYPE_ERROR_CHILD,
  FM_TYPE_ERROR_DUPLICATE,
  FM_TYPE_ERROR_DIM,
  FM_TYPE_ERROR_ARGS,
  FM_TYPE_ERROR_PARAMS,
  FM_TYPE_ERROR_UNKNOWN,
  FM_TYPE_ERROR_LAST
} FM_TYPE_ERROR;

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

typedef struct fm_call_def fm_call_def_t;

typedef void *fm_call_exec_cl;

typedef bool (*fm_call_init_p)(fm_frame_t *, size_t,
                               const fm_frame_t *const argv[], fm_call_ctx_t *,
                               fm_call_exec_cl *);

typedef void (*fm_call_destroy_p)(fm_call_exec_cl);

typedef bool (*fm_call_exec_p)(fm_frame_t *, size_t,
                               const fm_frame_t *const argv[], fm_call_ctx_t *,
                               fm_call_exec_cl);

typedef const void *fm_ctx_def_cl;

/**
 * @brief pointer to a queuing callback
 */
typedef void (*fm_call_queuer_p)(size_t, fm_call_ctx_t *);

/**
 * @brief type definition of the context provided by computation generator
 */
typedef const void *fm_comp_ctx_p;

/**
 * @brief pointer to a generic execution context
 */
typedef fm_exec_ctx_t *fm_exec_ctx_p;

typedef size_t (*fm_writer)(const void *data, size_t count, void *closure);
typedef bool (*fm_reader)(void *data, size_t limit, void *closure);

/**
 * @brief execution point object
 *
 * Execution point object represents a reference to call stack item
 */
typedef size_t fm_call_handle_t;

/**
 * @brief enum for base types
 *
 * Defines based types for type declaration
 */

// NOTE: Move these definitions to the common library
typedef int8_t INT8;
typedef int16_t INT16;
typedef int32_t INT32;
typedef int64_t INT64;
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;
typedef float FLOAT32;
typedef double FLOAT64;
typedef fmc_rational64_t RATIONAL64;
typedef fmc_rprice_t RPRICE;
typedef fmc_decimal128_t DECIMAL128;
typedef fmc_time64_t TIME64;
typedef char CHAR;
typedef wchar_t WCHAR;
#ifndef FM_SYS_WIN
typedef bool BOOL;
#else
#include <windows.h>
#endif

typedef enum {
  FM_TYPE_INT8 = 0,
  FM_TYPE_INT16,
  FM_TYPE_INT32,
  FM_TYPE_INT64,
  FM_TYPE_UINT8,
  FM_TYPE_UINT16,
  FM_TYPE_UINT32,
  FM_TYPE_UINT64,
  FM_TYPE_FLOAT32,
  FM_TYPE_FLOAT64,
  FM_TYPE_RATIONAL64,
  FM_TYPE_RPRICE,
  FM_TYPE_DECIMAL128,
  FM_TYPE_TIME64,
  FM_TYPE_CHAR,
  FM_TYPE_WCHAR,
  FM_TYPE_BOOL,
  FM_TYPE_LAST
} FM_BASE_TYPE;

#define STACK_OFF(stack, what)                                                 \
  (char *)((size_t)(stack).header.cursor &                                     \
           (~((FMC_WORDSIZE - 1) & (sizeof(what) - 1))))

#define STACK_CHECK(stack, what)                                               \
  (STACK_OFF((stack), what) >= &(stack).buffer[0] + sizeof(what))

#define STACK_PUSH(stack, what)                                                \
  ((stack).header.cursor = STACK_OFF((stack), (what)) - sizeof(what),          \
   memcpy((stack).header.cursor, (void *)&(what), sizeof(what)))

#define STACK_SAFE_PUSH(stack, what)                                           \
  (STACK_CHECK((stack), (what)) ? (STACK_PUSH((stack), (what)), true) : false)

#define HEAP_STACK_PUSH(stack, what)                                           \
  (STACK_CHECK((*stack), (what))                                               \
       ? (STACK_PUSH((*stack), (what)), true)                                  \
       : (fm_arg_stack_double(&stack) ? STACK_SAFE_PUSH((*stack), (what))      \
                                      : false))

#define STACK_POP(stack, what)                                                 \
  ((stack).header.cursor = STACK_OFF((stack), what) - sizeof(what),            \
   *(what *)(stack).header.cursor)

/**
 * @brief defines context structure for a call
 */
typedef struct fm_call_ctx {
  fm_comp_ctx_p comp;
  fm_exec_ctx_p exec;
  fm_call_handle_t handle;
  size_t depc;
  const fm_call_handle_t *deps;
} fm_call_ctx_t;

struct fm_comp_sys_ext_path_list {
  struct fm_comp_sys_ext_path_list *next, *prev;
  char path[]; // FAM
};

struct fm_comp_sys_module {
  struct fm_comp_sys *sys; // the system that owns the module
  fmc_ext_t handle;        // module handle. Return of dlopen()
  char *name;              // module name (e.g. "fmtron")
  char *file;              // file full path of the library
  struct fm_comp_sys_module *next, *prev;
};

struct extractor_api_v1;

typedef void (*fm_comp_sys_module_init_v1)(struct extractor_api_v1 *,
                                           fm_comp_sys_t *,
                                           fmc_error_t **);

struct extractor_api_v1 {
  // Clean up system
  void (*comp_sys_cleanup)(fm_comp_sys_t *);
  // Get type sys
  fm_type_sys_t *(*type_sys_get)(fm_comp_sys_t *);
  void (*type_sys_err_custom)(fm_type_sys_t *, FM_TYPE_ERROR, const char *);

  // Get graph
  fm_comp_graph_t *(*comp_graph_get)(fm_comp_sys_t *);
  // Get stream context
  fm_stream_ctx_t *(*stream_ctx_get)(fm_comp_sys_t *, fm_comp_graph_t *);
  // Get recorded stream context
  fm_stream_ctx_t *(*stream_ctx_recorded)(fm_comp_sys_t *, fm_comp_graph_t *,
                                          fm_writer, void *);
  // Get replayed stream context
  fm_stream_ctx_t *(*stream_ctx_replayed)(fm_comp_sys_t *, fm_comp_graph_t *,
                                          fm_reader, void *);
  // Queue call handle for execution on stream context
  void (*stream_ctx_queue)(fm_stream_ctx_t *ctx, fm_call_handle_t handle);
  // Schedule call handle for execution on stream context
  void (*stream_ctx_schedule)(fm_stream_ctx_t *ctx, fm_call_handle_t handle,
                              fmc_time64_t time);
  // Verify if there are any handles scheduled by the stream context
  bool (*stream_ctx_scheduled)(fm_stream_ctx_t *ctx);
  // Verify if stream context is idle
  bool (*stream_ctx_idle)(fm_stream_ctx_t *ctx);
  // Stream context, process one
  bool (*stream_ctx_proc_one)(fm_stream_ctx_t *, fmc_time64_t);
  // Stream context, next time
  fmc_time64_t (*stream_ctx_next_time)(fm_stream_ctx_t *);
  // Stream context, next time
  fmc_time64_t (*stream_ctx_now)(fm_stream_ctx_t *);

  // Get error msg
  const char *(*comp_sys_error_msg)(fm_comp_sys_t *);
  // Add computation definition to computational system
  bool (*comp_type_add)(fm_comp_sys_t *, const fm_comp_def_t *def);

  // Find computation node in graph
  fm_comp_t *(*comp_find)(fm_comp_graph_t *, const char *);

  // Execution context, is error
  bool (*exec_ctx_is_error)(fm_exec_ctx_t *);
  // Execution context, error msg
  const char *(*exec_ctx_error_msg)(fm_exec_ctx_t *);
  // Execution context, error set
  void (*exec_ctx_error_set)(fm_exec_ctx_t *ctx, const char *msg, ...);

  // Obtain result frame reference from computation
  fm_result_ref_t *(*result_ref_get)(fm_comp_t *);
  // Set callback to computation
  void (*comp_clbck_set)(fm_comp_t *, fm_frame_clbck_p, fm_frame_clbck_cl);

  // Get computation result type
  fm_type_decl_cp (*comp_result_type)(const fm_comp_t *);

  // Get type number of fields
  unsigned (*type_frame_nfields)(fm_type_decl_cp);
  // Get type field name
  const char *(*type_frame_field_name)(fm_type_decl_cp, int);
  // Checks whether type declaration is of base type
  bool (*type_is_base)(fm_type_decl_cp td);
  // Return the FM_BASE enum corresponding to base type
  FM_BASE_TYPE (*type_base_enum)(fm_type_decl_cp td);
  // Checks whether type declaration is of cstring type
  bool (*type_is_cstring)(fm_type_decl_cp td);
  // Returns an item from the tuple object
  fm_type_decl_cp (*type_tuple_arg)(fm_type_decl_cp, unsigned);
  // Returns true if type is null or empty tuple
  bool (*args_empty)(fm_type_decl_cp);
  // Checks whether type declaration is of tuple type
  bool (*type_is_tuple)(fm_type_decl_cp td);
  // Returns size of tuple object
  unsigned (*type_tuple_size)(fm_type_decl_cp);

  // Get frame data
  const void *(*frame_get_cptr1)(const fm_frame_t *, fm_field_t, int);
  // Get frame data
  void *(*frame_get_ptr1)(fm_frame_t *, fm_field_t, int);
  // Get frame field
  fm_field_t (*frame_field)(const fm_frame_t *, const char *);
  // Get result reference data
  fm_frame_t *(*data_get)(fm_result_ref_t *);

  fm_call_def_t *(*call_def_new)();
  void (*call_def_init_set)(fm_call_def_t *, fm_call_init_p init);
  void (*call_def_destroy_set)(fm_call_def_t *, fm_call_destroy_p d);
  void (*call_def_exec_set)(fm_call_def_t *obj, fm_call_exec_p exec);

  fm_ctx_def_t *(*ctx_def_new)();
  void (*ctx_def_inplace_set)(fm_ctx_def_t *, bool);
  void (*ctx_def_type_set)(fm_ctx_def_t *, fm_type_decl_cp);
  void (*ctx_def_closure_set)(fm_ctx_def_t *, fm_ctx_def_cl closure);
  void (*ctx_def_volatile_set)(fm_ctx_def_t *, unsigned);
  void (*ctx_def_queuer_set)(fm_ctx_def_t *, fm_call_queuer_p q);
  void (*ctx_def_stream_call_set)(
      fm_ctx_def_t *,
      fm_call_def_t *(*stream)(fm_comp_def_cl, const fm_ctx_def_cl));
  void (*ctx_def_query_call_set)(fm_ctx_def_t *,
                                 fm_call_def_t *(*query)(fm_comp_def_cl,
                                                         const fm_ctx_def_cl));
  fm_ctx_def_cl (*ctx_def_closure)(fm_ctx_def_t *);

  fm_type_decl_cp (*base_type_get)(fm_type_sys_t *ts, FM_BASE_TYPE t);
  fm_type_decl_cp (*array_type_get)(fm_type_sys_t *ts, fm_type_decl_cp td,
                                    unsigned s);
  bool (*type_is_array)(fm_type_decl_cp td);
  size_t (*type_array_size)(fm_type_decl_cp);
  fm_type_decl_cp (*type_array_of)(fm_type_decl_cp td);
  char *(*type_to_str)(fm_type_decl_cp);
  int (*type_frame_field_idx)(fm_type_decl_cp td, const char *);

  fm_type_decl_cp (*frame_type)(const fm_frame_t *);
  bool (*type_is_frame)(fm_type_decl_cp td);
  void (*frame_reserve)(fm_frame_t *, ...);
  fm_type_decl_cp (*frame_type_get1)(fm_type_sys_t *ts, unsigned num,
                                     const char *names[],
                                     fm_type_decl_cp types[], unsigned nd,
                                     int dims[]);

  fm_type_decl_cp (*type_frame_field_type)(fm_type_decl_cp td, int i);
  fm_type_decl_cp (*cstring_type_get)(fm_type_sys_t *ts);
  fm_type_decl_cp (*tuple_type_get)(fm_type_sys_t *ts, unsigned num, ...);

  fm_comp_t *(*comp_decl)(fm_comp_sys_t *csys, fm_comp_graph_t *graph,
                          const char *comp, unsigned nargs,
                          fm_type_decl_cp type, ...);

  fm_type_decl_cp (*frame_type_get)(fm_type_sys_t *ts, unsigned nf, unsigned nd,
                                    ...);
  fm_type_decl_cp (*type_type_get)(fm_type_sys_t *ts);
  const char *(*type_sys_errmsg)(fm_type_sys_t *ts);
  struct fm_comp_sys_ext_path_list *(*comp_sys_ext_path_list_get)(fm_comp_sys_t *s);
  void (*comp_sys_paths_set_default)(struct fm_comp_sys *sys, fmc_error_t **error);
  void (*comp_sys_ext_path_list_set)(struct fm_comp_sys_ext_path_list **head,
                                     const char **paths, fmc_error_t **error);
  void (*comp_sys_ext_path_list_add)(struct fm_comp_sys_ext_path_list **phead,
                                     const char *path, fmc_error_t **error);

  void (*comp_sys_ext_path_list_del)(struct fm_comp_sys_ext_path_list **phead);
};

FMMODFUNC struct extractor_api_v1 *extractor_api_v1_get();

#ifdef __cplusplus
}
#endif
