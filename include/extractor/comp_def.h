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
 * @file comp_def.h
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C declaration of the computation interface
 *
 * This file contains declarations of the computation interface.
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/arg_stack.h"
#include "extractor/call_ctx.h"
#include "extractor/frame_base.h"
#include "extractor/type_sys.h"
#include "fmc/platform.h"
#include "fmc/time.h"

#include <stdarg.h>

#ifdef __cplusplus
#define FmMODINIT_FUNC extern "C" void
#else
#define FmMODINIT_FUNC void
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*fm_call_init_p)(fm_frame_t *, size_t,
                               const fm_frame_t *const argv[], fm_call_ctx_t *,
                               fm_call_exec_cl *);

typedef void (*fm_call_destroy_p)(fm_call_exec_cl);

typedef fmc_time64_range_t (*fm_call_range_p)(const fm_call_ctx_t *,
                                              fm_call_exec_cl);

typedef struct fm_call_def fm_call_def_t;

typedef const void *fm_ctx_def_cl;

typedef struct fm_ctx_def fm_ctx_def_t;

typedef void *fm_comp_def_cl;

typedef struct fm_comp_sys fm_comp_sys_t;

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

/**
 * @brief
 */
FMMODFUNC fm_ctx_def_t *fm_ctx_def_new();

/**
 * @brief
 */
FMMODFUNC void fm_ctx_def_del(fm_ctx_def_t *);

/**
 * @brief
 */
FMMODFUNC fm_type_decl_cp fm_ctx_def_type(const fm_ctx_def_t *);

/**
 * @brief
 */
FMMODFUNC void fm_ctx_def_type_set(fm_ctx_def_t *, fm_type_decl_cp);

/**
 * @brief
 */
FMMODFUNC fm_type_decl_cp fm_ctx_def_type_get(fm_ctx_def_t *);

/**
 * @brief
 */
FMMODFUNC bool fm_ctx_def_inplace(const fm_ctx_def_t *);

/**
 * @brief
 */
FMMODFUNC bool fm_ctx_def_volatile(const fm_ctx_def_t *);

/**
 * @brief
 */
FMMODFUNC void fm_ctx_def_inplace_set(fm_ctx_def_t *, bool);

/**
 * @brief
 */
FMMODFUNC void fm_ctx_def_volatile_set(fm_ctx_def_t *, unsigned);

/**
 * @brief
 */
FMMODFUNC unsigned fm_ctx_def_volatile_get(const fm_ctx_def_t *);

/**
 * @brief sets the context definition closure
 */
FMMODFUNC void fm_ctx_def_closure_set(fm_ctx_def_t *, fm_ctx_def_cl closure);

/**
 * @brief
 */
FMMODFUNC void fm_ctx_def_queuer_set(fm_ctx_def_t *, fm_call_queuer_p q);

/**
 * @brief
 */
FMMODFUNC void fm_ctx_def_stream_call_set(
    fm_ctx_def_t *,
    fm_call_def_t *(*stream)(fm_comp_def_cl, const fm_ctx_def_cl));

/**
 * @brief
 */
FMMODFUNC void fm_ctx_def_query_call_set(
    fm_ctx_def_t *,
    fm_call_def_t *(*query)(fm_comp_def_cl, const fm_ctx_def_cl));

/**
 * @brief
 */
FMMODFUNC fm_call_def_t *fm_ctx_def_stream_call(fm_comp_def_cl comp_cl,
                                                fm_ctx_def_t *obj);

/**
 * @brief
 */
FMMODFUNC fm_call_def_t *fm_ctx_def_query_call(fm_comp_def_cl comp_cl,
                                               fm_ctx_def_t *obj);

/**
 * @brief
 */
FMMODFUNC fm_ctx_def_cl fm_ctx_def_closure(fm_ctx_def_t *);

/**
 * @brief
 */
FMMODFUNC fm_call_queuer_p fm_ctx_def_queuer(fm_ctx_def_t *obj);

/**
 * @brief
 */
FMMODFUNC fm_call_def_t *fm_call_def_new();

/**
 * @brief
 */
FMMODFUNC void fm_call_def_del(fm_call_def_t *);

/**
 * @brief
 */
FMMODFUNC void fm_call_def_init_set(fm_call_def_t *, fm_call_init_p init);

/**
 * @brief
 */
FMMODFUNC void fm_call_def_destroy_set(fm_call_def_t *, fm_call_destroy_p d);

/**
 * @brief
 */
FMMODFUNC void fm_call_def_range_set(fm_call_def_t *, fm_call_range_p range);

/**
 * @brief
 */
FMMODFUNC void fm_call_def_exec_set(fm_call_def_t *obj, fm_call_exec_p exec);

/**
 * @brief
 */
FMMODFUNC fm_call_init_p fm_call_def_init(fm_call_def_t *obj);

/**
 * @brief
 */
FMMODFUNC fm_call_destroy_p fm_call_def_destroy(fm_call_def_t *obj);

/**
 * @brief
 */
FMMODFUNC fm_call_range_p fm_call_def_range(fm_call_def_t *obj);

/**
 * @brief
 */
FMMODFUNC fm_call_exec_p fm_call_def_exec(fm_call_def_t *obj);

#ifdef __cplusplus
}
#endif
