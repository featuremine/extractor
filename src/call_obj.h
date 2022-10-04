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
 * @file call_obj.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the call object
 *
 * This file contains declarations of the call context
 * @see http://www.featuremine.com
 */

#ifndef __FM_CALL_OBJ_H__
#define __FM_CALL_OBJ_H__

#include "extractor/call_ctx.h"
#include "extractor/frame_base.h"
#include "extractor/handle.h"
#include "call_stack.h"

#include <stddef.h>

/**
 * @brief a call object
 *
 * Represents a generic call object that can have varied size
 */
typedef struct fm_call_obj fm_call_obj_t;

typedef void (*fm_setup_func_p)(fm_call_obj_t *);

/**
 * @brief
 */
fm_call_obj_t *fm_call_obj_new(unsigned argc);

/**
 * @brief
 */
void fm_call_obj_cleanup(fm_call_obj_t *);

/**
 * @brief
 */
void fm_call_obj_del(fm_call_obj_t *obj);

/**
 * @brief return a size of call object in bytes
 */
size_t fm_call_obj_size(fm_call_obj_t *);

/**
 * @brief execute a call object
 */
bool fm_call_obj_exec(fm_call_obj_t *);

/**
 * @brief
 */
fm_call_handle_t fm_call_obj_handle(fm_call_obj_t *);

/**
 * @brief
 */
void fm_call_obj_handle_set(fm_call_obj_t *, fm_call_handle_t handle);

/**
 * @brief
 */
void fm_call_obj_depc_set(fm_call_obj_t *obj, size_t depc);

/**
 * @brief
 */
void fm_call_obj_deps_set(fm_call_obj_t *obj, const fm_call_handle_t *deps);

/**
 * @brief
 */
fm_frame_t *fm_call_obj_result(fm_call_obj_t *);

/**
 * @brief
 */
void fm_call_obj_arg_set(fm_call_obj_t *obj, size_t argc, fm_frame_t *f);

/**
 * @brief
 */
size_t fm_call_obj_argc(fm_call_obj_t *call);

/**
 * @brief
 */
fm_frame_t *const *fm_call_obj_argv(fm_call_obj_t *call);

/**
 * @brief
 */
fm_frame_t *fm_call_obj_arg(fm_call_obj_t *call, size_t argi);

/**
 * @brief
 */
void fm_call_obj_result_set(fm_call_obj_t *call, fm_frame_t *frame);

/**
 * @brief
 */
void fm_call_obj_setup_set(fm_call_obj_t *call, fm_setup_func_p setfunc);

fm_call_ctx *fm_call_obj_ctx(fm_call_obj_t *call);

/**
 * @brief
 */
void fm_call_obj_exec_ctx_set(fm_call_obj_t *call, fm_exec_ctx_p ctx);

/**
 * @brief
 */
void fm_call_obj_comp_ctx_set(fm_call_obj_t *call, fm_comp_ctx_p ctx);

/**
 * @brief
 */
fm_call_exec_cl fm_call_obj_exec_cl(fm_call_obj_t *obj);

/**
 * @brief set a queue callback
 */
void fm_call_obj_queuer_set(fm_call_obj_t *, fm_call_queuer_p q);

/**
 * @brief
 */
void fm_call_obj_exec_set(fm_call_obj_t *, fm_call_exec_p, fm_call_exec_cl);

/**
 * @brief
 */
void fm_call_obj_clbck_set(fm_call_obj_t *obj, fm_frame_clbck_p func,
                           fm_frame_clbck_cl cl);

/**
 * @brief
 */
fm_exec_ctx_p fm_call_obj_exec_ctx(fm_call_obj_t *call);

/**
 * @brief calls dependencies' queue callbacks
 */
void fm_call_obj_deps_queue(fm_call_obj_t *);

/**
 * @brief adds a queuing callback for a dependency
 */
void fm_call_obj_dep_queuer_add(fm_call_obj_t *, fm_call_obj_t *, size_t);

#endif // __FM_CALL_OBJ_H__
