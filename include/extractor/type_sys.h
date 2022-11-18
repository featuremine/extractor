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
 * @file type_sys.h
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C declaration of the type system library
 *
 * This file contains declarations of the type systems library
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

#pragma once

#include "extractor/arg_stack.h"
#include "extractor/type_decl.h"
#include "fmc/platform.h"
#include <stddef.h>
#include <stdint.h>

#include "extractor/arg_stack.h"
#include "extractor/type_decl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief type system object
 *
 * object used to store data relevant to type systems.
 */
typedef struct fm_type_sys fm_type_sys_t;

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

/**
 * @brief creates type system object
 *
 * Function creates type system object and returns @c fm_type_sys_t.
 * After use it needs to be destroyed with c@ fm_type_sys_del.
 * @return fm_type_sys_t object.
 * @see fm_type_sys_t
 * @see fm_type_sys_del
 */
FMMODFUNC fm_type_sys_t *fm_type_sys_new();

/**
 * @brief deletes type system object
 *
 * Function deletes type system object and frees memory.
 * @param @c fm_type_sys_t object
 * @see fm_type_sys_t
 * @see fm_type_sys_new
 */
FMMODFUNC void fm_type_sys_del(fm_type_sys_t *ts);

/**
 * @brief set error number of type system
 */
FMMODFUNC void fm_type_sys_err_set(fm_type_sys_t *ts, FM_TYPE_ERROR errnum);

/**
 * @brief set custom error message
 */
FMMODFUNC void fm_type_sys_err_custom(fm_type_sys_t *, FM_TYPE_ERROR,
                                      const char *);

/**
 * @brief return error number of the last type system error
 */
FMMODFUNC FM_TYPE_ERROR fm_type_sys_errno(fm_type_sys_t *ts);

/**
 * @brief return error message of the last type system error
 */
FMMODFUNC const char *fm_type_sys_errmsg(fm_type_sys_t *ts);

/**
 * @brief test whether the two types are equal
 *
 * Function returns true is the two types are equal
 * @return true if equal
 */
FMMODFUNC bool fm_type_equal(fm_type_decl_cp a, fm_type_decl_cp b);

/**
 * @brief gets base type object
 *
 * Function returns type declaration object @c fm_type_decl_cp
 * corresponding to a based type @p t
 * @param ts type system to use
 * @param t base type to create
 * @return type declaration object
 * @see @c fm_type_sys_t
 * @see @c fm_type_decl_cp
 * @see @c FM_BASE_TYPE
 */
FMMODFUNC fm_type_decl_cp fm_base_type_get(fm_type_sys_t *ts, FM_BASE_TYPE t);

/**
 * @brief gets a record type object
 *
 * Function returns type declaration object @c fm_type_decl_cp for
 * a record of name @p name of size @p s
 * @param ts type system to use
 * @param name record name
 * @param s size of the record
 * @return type declaration of the record
 * @see @c fm_type_sys_t
 * @see @c fm_type_decl_cp
 */
FMMODFUNC fm_type_decl_cp fm_record_type_get(fm_type_sys_t *ts,
                                             const char *name, size_t s);

/**
 * @brief gets array type object
 *
 * Function returns type declaration object @c fm_type_decl_cp for
 * an array of elements of type @p td of size @p s
 * @param ts type system to use
 * @param td type declaration of element type
 * @param s size of the array
 * @return type declaration of the array
 * @see @c fm_type_sys_t
 * @see @c fm_type_decl_cp
 */
FMMODFUNC fm_type_decl_cp fm_array_type_get(fm_type_sys_t *ts,
                                            fm_type_decl_cp td, unsigned s);

/**
 * @brief gets frame type object
 *
 * Function returns type declaration object @c fm_type_decl_cp for
 * a frame of @p nf fields and @p nd dimensions where name, types
 * and dimensions are specified as follows:
 * @c nf, nd, name_1, type_1, ..., name_nf, type_nf, d_1, ..., d_nd
 * @param name_i is of type const char *
 * @param type_i is of type fm_type_decl_cp
 * @param d_i is of type int
 * @return type declaration of the frame
 * @see @c fm_type_sys_t
 * @see @c fm_type_decl_cp
 */
FMMODFUNC fm_type_decl_cp fm_frame_type_get(fm_type_sys_t *ts, unsigned nf,
                                            unsigned nd, ...);

/**
 * @brief gets frame type object
 *
 * Function returns type declaration object @c fm_type_decl_cp for
 * a frame of @p nf fields and @p nd dimensions where names are
 * specified in @p names, types in @p types[] and dimensions in
 * @p dims.
 * @param name_i is of type const char *
 * @param type_i is of type fm_type_decl_cp
 * @param d_i is of type int
 * @return type declaration of the frame
 * @see @c fm_type_sys_t
 * @see @c fm_type_decl_cp
 */
FMMODFUNC fm_type_decl_cp fm_frame_type_get1(fm_type_sys_t *ts, unsigned num,
                                             const char *names[],
                                             fm_type_decl_cp types[],
                                             unsigned nd, int dims[]);

/**
 * @brief gets tuple type object
 */
FMMODFUNC fm_type_decl_cp fm_tuple_type_get(fm_type_sys_t *ts, unsigned num,
                                            ...);

/**
 * @brief gets tuple type object
 */
FMMODFUNC fm_type_decl_cp fm_tuple_type_get1(fm_type_sys_t *ts, unsigned num,
                                             fm_type_decl_cp types[]);

/**
 * @brief gets cstring type object
 */
FMMODFUNC fm_type_decl_cp fm_cstring_type_get(fm_type_sys_t *ts);

/**
 * @brief gets module type object
 */
FMMODFUNC fm_type_decl_cp fm_module_type_get(fm_type_sys_t *ts, unsigned ninps,
                                             unsigned nouts);

/**
 * @brief gets type of type object
 */
FMMODFUNC fm_type_decl_cp fm_type_type_get(fm_type_sys_t *ts);

/**
 * @brief checks whether type declaration is of base type
 */
FMMODFUNC bool fm_type_is_base(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is of record type
 */
FMMODFUNC bool fm_type_is_record(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is of array type
 */
FMMODFUNC bool fm_type_is_array(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is of simple type
 *
 * Simple type is either a base or record or an array of
 * simple type.
 */
FMMODFUNC bool fm_type_is_simple(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is for unsigned integer
 */
FMMODFUNC bool fm_type_is_unsigned(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is for signed integer
 */
FMMODFUNC bool fm_type_is_signed(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is for a float
 */
FMMODFUNC bool fm_type_is_float(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is for a boolean
 */
FMMODFUNC bool fm_type_is_bool(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is for a decimal64
 */
FMMODFUNC bool fm_type_is_decimal(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is for a decimal128
 */
FMMODFUNC bool fm_type_is_decimal128(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is of frame type
 */
FMMODFUNC bool fm_type_is_frame(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration td1 a subframe type td2
 */
FMMODFUNC bool fm_type_is_subframe(fm_type_decl_cp td1, fm_type_decl_cp td2);

/**
 * @brief checks whether type declaration is of tuple type
 */
FMMODFUNC bool fm_type_is_tuple(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is of cstring type
 */
FMMODFUNC bool fm_type_is_cstring(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is of module type
 */
FMMODFUNC bool fm_type_is_module(fm_type_decl_cp td);

/**
 * @brief checks whether type declaration is of type type
 */
FMMODFUNC bool fm_type_is_type(fm_type_decl_cp td);

/**
 * @brief return the FM_BASE enum corresponding to base type
 *
 * If the type declaration does not correspond to base type,
 * the result is FM_TYPE_LAST;
 */
FMMODFUNC FM_BASE_TYPE fm_type_base_enum(fm_type_decl_cp td);

/**
 * @brief return the size of the array
 */
FMMODFUNC size_t fm_type_array_size(fm_type_decl_cp td);

/**
 * @brief returns the type of the array element
 */
FMMODFUNC fm_type_decl_cp fm_type_array_of(fm_type_decl_cp td);

/**
 * @brief returns number of frame's dimensions
 */
FMMODFUNC unsigned fm_type_frame_ndims(fm_type_decl_cp td);

/**
 * @brief returns @p i th dimension of the frame @p td
 */
FMMODFUNC int fm_type_frame_dim(fm_type_decl_cp td, int i);

/**
 * @brief returns size of number of frame's fields
 */
FMMODFUNC unsigned fm_type_frame_nfields(fm_type_decl_cp td);

/**
 * @brief returns @p i th field's type for the frame @p td
 */
FMMODFUNC fm_type_decl_cp fm_type_frame_field_type(fm_type_decl_cp td, int i);

/**
 * @brief returns the index of the @p i th field in the frame @p td
 */
FMMODFUNC int fm_type_frame_field_idx(fm_type_decl_cp td, const char *);

/**
 * @brief returns the name of the field with index @p i
 */
FMMODFUNC const char *fm_type_frame_field_name(fm_type_decl_cp td, int i);

/**
 * @brief return the type of the projection frame
 */
FMMODFUNC fm_type_decl_cp fm_frame_proj_type_get(fm_type_sys_t *ts,
                                                 fm_type_decl_cp td,
                                                 const char *);

/**
 * @brief returns size of a given type in bytes
 */
FMMODFUNC size_t fm_type_sizeof(fm_type_decl_cp td);

/**
 * @brief returns size of tuple object
 */
FMMODFUNC unsigned fm_type_tuple_size(fm_type_decl_cp);

/**
 * @brief returns an item from the tuple object
 */
FMMODFUNC fm_type_decl_cp fm_type_tuple_arg(fm_type_decl_cp, unsigned);

/**
 * @brief builds an argument stack from varg list
 *
 * Return 0 on success, -1 if no conversion is possible,
 * 1 if stack is not large enough
 */
FMMODFUNC int fm_arg_stack_build(fm_type_decl_cp, fm_arg_stack_t *, va_list *);

/**
 * @brief will try to get an uint64_t
 *
 * Function will try to get an unsigned int. If the type is unmatched,
 * stack will remain untouched.
 */
FMMODFUNC bool fm_arg_try_uint64(fm_type_decl_cp, fm_arg_stack_t *, uint64_t *);

/**
 * @brief will try to get an double
 *
 * Function will try to get an double. If the type is unmatched,
 * stack will remain untouched.
 */
bool fm_arg_try_float64(fm_type_decl_cp td, fm_arg_stack_t *arg, double *i);

/**
 * @brief will try to get an const char *
 *
 * Function will try to get a const char *. If the type is unmatched,
 * stack will remain untouched and NULL is returned.
 */
FMMODFUNC const char *fm_arg_try_cstring(fm_type_decl_cp, fm_arg_stack_t *);

/**
 * @brief will try to get an integer
 *
 * Function will try to get an signed int. If the type is unmatched,
 * stack will remain untouched.
 */
FMMODFUNC bool fm_arg_try_integer(fm_type_decl_cp, fm_arg_stack_t *, int64_t *);

/**
 * @brief will obtain an unsigned integer
 *
 * Function will try to get an unsigned integer by reading it either
 * as a signed or unsigned integer. If the type is unmatched stack
 * will remain untouched.
 */
FMMODFUNC bool fm_arg_try_uinteger(fm_type_decl_cp, fm_arg_stack_t *,
                                   uint64_t *);

/**
 * @brief will try to get an fmc_time64_t
 *
 * Function will try to get an signed int. If the type is unmatched,
 * stack will remain untouched.
 */
FMMODFUNC bool fm_arg_try_time64(fm_type_decl_cp, fm_arg_stack_t *,
                                 fmc_time64_t *);

/**
 * @brief will try to get a type declaration parameter
 *
 * Function will try to get a fm_type_decl_cp. If the type is unmatched,
 * returns null and stack will remain untouched.
 */
FMMODFUNC fm_type_decl_cp fm_arg_try_type_decl(fm_type_decl_cp,
                                               fm_arg_stack_t *);

/**
 * @brief returns true if type is null or empty tuple
 */
FMMODFUNC bool fm_args_empty(fm_type_decl_cp);

/**
 * @brief returns a string representation of the type
 *
 * the returned pointer is either NULL on error or
 * needs to be freed with free
 */
FMMODFUNC char *fm_type_to_str(fm_type_decl_cp);

/**
 * @brief parses a type from a string
 */
FMMODFUNC fm_type_decl_cp fm_type_from_str(fm_type_sys_t *ts, const char *c,
                                           size_t len);

typedef struct fm_type_io fm_type_io_t;

/**
 * @brief returns an io object for a simple type
 */
FMMODFUNC fm_type_io_t *fm_type_io_get(fm_type_sys_t *ts, fm_type_decl_cp decl);

/**
 * @brief parses the type into the location pointed by void *
 */
FMMODFUNC const char *fm_type_io_parse(fm_type_io_t *, const char *,
                                       const char *, void *);

/**
 * @brief writes into a file from the location pointed by void *
 */
FMMODFUNC bool fm_type_io_fwrite(fm_type_io_t *, FILE *, const void *);

#ifdef __cplusplus
}
#endif
