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
 * @file comp_sys.h
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C declaration of the computational system
 *
 * This file contains declarations of the computational system.
 * @see http://www.featuremine.com
 */

#ifndef __FM_COMP_SYS_H__
#define __FM_COMP_SYS_H__

#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/frame_base.h"
#include "extractor/serial.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
#include "extractor/type_decl.h"
#include "fmc/platform.h"

typedef struct fm_comp_graph fm_comp_graph_t;

typedef struct fm_comp fm_comp_t;

typedef struct fm_result_ref fm_result_ref_t;

/**
 * @brief
 */
FMMODFUNC fm_comp_sys_t *fm_comp_sys_new(const char *license_file,
                                         char **errmsg);

FMMODFUNC void fm_comp_sys_cleanup(fm_comp_sys_t *);

/**
 * @brief
 */
FMMODFUNC void fm_comp_sys_del(fm_comp_sys_t *);

/**
 * @brief
 */
FMMODFUNC bool fm_comp_type_add(fm_comp_sys_t *, const fm_comp_def_t *);

/**
 * @brief
 */
FMMODFUNC fm_comp_graph_t *fm_comp_graph_get(fm_comp_sys_t *);

/**
 * @brief
 */
FMMODFUNC void fm_comp_graph_remove(fm_comp_sys_t *, fm_comp_graph_t *);

/**
 * @brief
 */
FMMODFUNC fm_type_sys_t *fm_type_sys_get(fm_comp_sys_t *);

/**
 * @brief
 */
FMMODFUNC fm_stream_ctx_t *fm_stream_ctx_get(fm_comp_sys_t *,
                                             fm_comp_graph_t *);

/**
 * @brief
 */
FMMODFUNC fm_comp_t *fm_comp_decl(fm_comp_sys_t *csys, fm_comp_graph_t *graph,
                                  const char *comp, unsigned nargs,
                                  fm_type_decl_cp type, ...);

/**
 * @brief
 */
FMMODFUNC fm_comp_t *fm_comp_decl2(fm_comp_sys_t *csys, fm_comp_graph_t *graph,
                                   const char *comp, const char *name,
                                   unsigned nargs, fm_type_decl_cp type, ...);

/**
 * @brief
 */
FMMODFUNC fm_comp_t *fm_comp_decl4(fm_comp_sys_t *csys, fm_comp_graph_t *graph,
                                   const char *comp, const char *name,
                                   unsigned nargs, fm_comp_t **inputs,
                                   fm_type_decl_cp type, fm_arg_stack_t args);

/**
 * @brief
 */
FMMODFUNC fm_comp_t *fm_comp_find(fm_comp_graph_t *g, const char *name);

/**
 * @brief
 */
FMMODFUNC const char *fm_comp_name(const fm_comp_t *);

/**
 * @brief
 */
FMMODFUNC void fm_comp_clbck_set(fm_comp_t *, fm_frame_clbck_p func,
                                 fm_frame_clbck_cl closure);

/**
 * @brief
 */
FMMODFUNC fm_result_ref_t *fm_result_ref_get(fm_comp_t *);

/**
 * @brief
 */
FMMODFUNC fm_frame_t *fm_data_get(fm_result_ref_t *);

/**
 * @brief
 */
FMMODFUNC bool fm_comp_sys_is_error(fm_comp_sys_t *s);

/**
 * @brief
 */
FMMODFUNC void fm_comp_sys_error_set(fm_comp_sys_t *s, const char *fmt, ...);

/**
 * @brief
 */
FMMODFUNC const char *fm_comp_sys_error_msg(fm_comp_sys_t *s);

/**
 * @brief Load extension module
 */
FMMODFUNC bool fm_comp_sys_ext_load(fm_comp_sys_t *, const char *name,
                                    const char *path);

/**
 * @brief Serializes the graph
 */
FMMODFUNC bool fm_comp_graph_write(const fm_comp_graph_t *, fm_writer, void *);

/**
 * @brief Loads serialized graph
 */
FMMODFUNC fm_comp_graph_t *fm_comp_graph_read(fm_comp_sys_t *, fm_reader reader,
                                              void *);

FMMODFUNC bool fm_comp_sys_sample_value(fm_comp_sys_t *sys,
                                        const char *sample_name, double *value);

#endif // __FM_COMP_SYS_H__
