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
 * @file call_stack.h
 * @author Maxim Trokhimtchouk
 * @date 2 Aug 2017
 * @brief File contains C declaration of the call stack object
 *
 * This file contains declarations of the call stack interface
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/handle.h"

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief a call object
 *
 * Represents a generic call object that can have varied size
 */
typedef struct fm_call_obj fm_call_obj_t;

/**
 * @brief return a size of call object in bytes
 */
size_t fm_call_obj_size(fm_call_obj_t *);

/**
 * @brief execute a call object
 */
bool fm_call_obj_exec(fm_call_obj_t *);

/**
 * @brief calls dependencies' queue callbacks
 */
void fm_call_obj_deps_queue(fm_call_obj_t *);

/**
 * @brief representation of the call stack object
 *
 * The call stack object is a stack of callable objects
 * which can be executed an have dependencies down the stack
 */
typedef struct fm_call_stack fm_call_stack_t;

/**
 * @brief creates new call stack object
 */
fm_call_stack_t *fm_call_stack_new();

/**
 * @brief deletes call stack object
 */
void fm_call_stack_del(fm_call_stack_t *s);

/**
 * @brief pushes a copy of a call object onto a stack
 */
fm_call_handle_t fm_call_stack_push(fm_call_stack_t *s, fm_call_obj_t *obj,
                                    int depc, fm_call_handle_t deps[]);

/**
 * @brief obtains a call object on a stack with a given handle
 */
fm_call_obj_t *fm_call_stack_obj(fm_call_stack_t *s, fm_call_handle_t handle);

/**
 * @brief obtains a count of the stack item sependencies
 */
size_t fm_call_stack_item_depc(fm_call_stack_t *s, fm_call_handle_t off);

/**
 * @brief obtains a pointer to stack dependencies
 */
const fm_call_handle_t *fm_call_stack_item_deps(fm_call_stack_t *s,
                                                fm_call_handle_t off);

/**
 * @brief call queue object
 *
 * The call queue object is used to keep track of execution queue
 * It is needed to make sure that only the dependencies of updated
 * call objects are executed.
 */
typedef struct fm_call_queue fm_call_queue_t;

/**
 * @brief creates new call queue object
 */
fm_call_queue_t *fm_call_queue_new();

/**
 * @brief deletes call queue object
 */
void fm_call_queue_del(fm_call_queue_t *q);

/**
 * @brief determines whether queue is empty
 */
bool fm_call_queue_empty(fm_call_queue_t *q);

/**
 * @brief push an call item with a given handle onto a call queue
 */
void fm_call_queue_push(fm_call_queue_t *q, fm_call_handle_t o);

/**
 * @brief executes one item on the stack from the given queue
 *
 * The first item on the queue is executed and its dependencies are
 * then added to the queue if the queue is not empty.
 * @return true if item was executed and generated an update
 */
bool fm_call_stack_exec_one(fm_call_stack_t *s, fm_call_queue_t *q);

/**
 * @brief executes the stack from a given queue
 *
 * The stack is executed in the dependency order. The first item
 * to be called is the first one in the queue. Then dependencies of
 * that item are added to the queue and the process continues.
 * @return true if at least one item was executed and generated an update
 */
bool fm_call_stack_exec(fm_call_stack_t *s, fm_call_queue_t *q);

void fm_call_obj_copy(void *ptr, fm_call_obj_t *obj);

void fm_call_obj_cleanup(fm_call_obj_t *obj);
