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
 * @file arg_serial.h
 * @author Maxim Trokhimtchouk
 * @date 12 Jun 2018
 * @brief File contains C declaration of the argument serialization
 *
 * This file contains declarations of the argument serialization
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/arg_stack.h"
#include "extractor/serial.h"
#include "extractor/type_sys.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fm_arg_buffer fm_arg_buffer_t;

fm_arg_buffer_t *fm_arg_buffer_new(fm_type_decl_cp type, fm_arg_stack_t args);
void fm_arg_buffer_del(fm_arg_buffer_t *buf);
size_t fm_arg_buffer_dump(fm_arg_buffer_t *buf, const char **);

bool fm_arg_write(const fm_arg_buffer_t *buf, fm_writer writer, void *closure);
fm_arg_buffer_t *fm_arg_read(fm_type_sys_t *ts, fm_type_decl_cp *td,
                             fm_arg_stack_t **s, fm_reader reader,
                             void *closure);
fm_arg_buffer_t *fm_arg_stack_from_buffer(fm_type_sys_t *ts,
                                          fm_arg_buffer_t *args,
                                          fm_type_decl_cp *type,
                                          fm_arg_stack_t **s);

#ifdef __cplusplus
}
#endif
