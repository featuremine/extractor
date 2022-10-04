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
 * @file frame.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C declaration of the frame object
 *
 * @see http://www.featuremine.com
 */

#ifndef __FM_FRAME_H__
#define __FM_FRAME_H__

#include <stddef.h>

#include "frame_base.h"
#include "type_decl.h"
#include <fmc/platform.h>

typedef int fm_field_t;

#ifdef __cplusplus
extern "C" {
#endif

FMMODFUNC fm_frame_t *fm_frame_from_type(fm_frame_alloc_t *,
                                         fm_type_decl_cp type);

FMMODFUNC fm_type_decl_cp fm_frame_type(const fm_frame_t *);

FMMODFUNC fm_type_decl_cp fm_frame_field_type(const fm_frame_t *, const char *);

FMMODFUNC const void *fm_frame_get_cptr1(const fm_frame_t *, fm_field_t, int);

FMMODFUNC void *fm_frame_get_ptr1(fm_frame_t *, fm_field_t, int);

FMMODFUNC void *fm_frame_get_ptr2(fm_frame_t *, fm_field_t, int, int);

FMMODFUNC fm_field_t fm_frame_field(const fm_frame_t *, const char *);

FMMODFUNC void fm_frame_swap(fm_frame_t *, fm_frame_t *);

FMMODFUNC bool fm_field_valid(fm_field_t);

FMMODFUNC bool fm_frame_singleton(const fm_frame_t *obj);

FMMODFUNC void fm_frame_assign(fm_frame_t *, const fm_frame_t *);

FMMODFUNC void fm_frame_field_copy(fm_frame_t *dest, fm_field_t,
                                   const fm_frame_t *src, fm_field_t);

/**
 * @brief copies the frame fields of a given offset
 *
 * dim0off specifies the offset in the first dimension
 * no checks are performed
 */
FMMODFUNC void fm_frame_field_copy_from0(fm_frame_t *dest, fm_field_t d_f,
                                         const fm_frame_t *src, fm_field_t s_f,
                                         unsigned dim0off);

/**
 * @brief projects field of the first frame onto the second
 *
 * the frames assumed to have correct types to make a proj copy
 */
FMMODFUNC void fm_frame_proj_assign(fm_frame_t *, const fm_frame_t *,
                                    fm_field_t);

/**
 * @brief reserves the frame
 *
 * must have size_t dim sizes
 * the number of dimentions specified must actually equal to number of
 * dimensions
 */
FMMODFUNC void fm_frame_reserve(fm_frame_t *, ...);

/**
 * @brief resizes in the first dimension
 *
 * If dim is larger than the old dimension, the old data remains,
 * new data in initialized to zero. If dim is smaller, as much old
 * data is kept as fits.
 */
FMMODFUNC void fm_frame_reserve0(fm_frame_t *, unsigned dim);

/**
 * @brief returns number of dimensions of the frame
 */
FMMODFUNC int fm_frame_ndims(const fm_frame_t *);

/**
 * @brief returns number ith dimension of the frame
 */
FMMODFUNC int fm_frame_dim(const fm_frame_t *, int i);

#ifdef __cplusplus
}
#endif

#endif // __FM_FRAME_H__
