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
 * @file module.h
 * @author Andres Rangel
 * @date 12 Sep 2018
 * @brief File contains C definitions of the module object
 *
 * This file contains declarations of the module object
 * @see http://www.featuremine.com
 */

#ifndef __FM_MODULE_H__
#define __FM_MODULE_H__

#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "fmc/platform.h"

typedef struct fm_module fm_module_t;

typedef struct fm_module_comp fm_module_comp_t;

/**
 * @brief Creates new module
 */
FMMODFUNC fm_module_t *fm_module_new(const char *name, unsigned nargs,
                                     fm_module_comp_t **inputs);

/**
 * @brief Add comp to module
 */
FMMODFUNC fm_module_comp_t *fm_module_comp_add(fm_module_t *m, const char *comp,
                                               const char *name, unsigned nargs,
                                               fm_module_comp_t **inputs,
                                               fm_type_decl_cp type, ...);

/**
 * @brief Add comp to module
 */
FMMODFUNC fm_module_comp_t *
fm_module_comp_add1(fm_module_t *m, const char *comp, const char *name,
                    unsigned nargs, fm_module_comp_t **inputs,
                    fm_type_decl_cp type, fm_arg_stack_t params);

/**
 * @brief Instanciates the module
 */
FMMODFUNC bool fm_module_inst(fm_comp_sys_t *sys, fm_comp_graph_t *g,
                              fm_module_t *m, fm_comp_t **inputs,
                              fm_comp_t **outputs);

/**
 * @brief Returns number of inputs
 */
FMMODFUNC unsigned fm_module_inps_size(fm_module_t *m);

/**
 * @brief Returns number of outputs
 */
FMMODFUNC unsigned fm_module_outs_size(fm_module_t *m);

/**
 * @brief Set outputs of module
 */
FMMODFUNC bool fm_module_outs_set(fm_module_t *m, unsigned nargs,
                                  fm_module_comp_t **comps);

/**
 * @brief Delete module
 */
FMMODFUNC void fm_module_del(fm_module_t *m);

/**
 * @brief Serializes the module
 */
FMMODFUNC bool fm_module_write(fm_module_t *, fm_writer, void *);

/**
 * @brief Loads serialized module
 */
FMMODFUNC fm_module_t *fm_module_read(fm_comp_sys_t *, fm_reader reader,
                                      void *);

#endif // __FM_MODULE_H__
