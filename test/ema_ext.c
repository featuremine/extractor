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
 * @file ema_ext.c
 * @author Maxim Trokhimtchouk
 * @date 3 Feb 2021
 * @brief Exponential moving average extension sample
 *
 * @see http://www.featuremine.com
 */

#include "extractor/api.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/stream_ctx.h"
#include "extractor/type_decl.h"
#include "extractor/type_sys.h"

typedef struct {
  double alpha;
  double result;
  bool interval;
} ema_ext_comp_closure;

struct extractor_api_v1 *api_ = NULL;

/**
 * The function is called when the operator is being constructed in order to
 * initialize the result frame. It is provided with a call context and an array
 * of data frame objects from operator dependencies. It should return true if
 * the operator is initialized.
 *
 * @param result frame of the operator
 * @param args number of dependency frames
 * @param argv array of dependency frames
 * @param ctx call context
 * @param cl call context closure
 * @return true on success, false otherwise
 */
static bool ema_ext_comp_stream_init(fm_frame_t *result, size_t args,
                                     const fm_frame_t *const argv[],
                                     fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  // Get the closure of the operator in the desired type
  ema_ext_comp_closure *comp_cl = (ema_ext_comp_closure *)ctx->comp;

  // Get the pointer to the result value in the closure
  double *res = &comp_cl->result;

  // Skip the trigger argument
  int args_offs = (args > 1);

  // Compute the sum of the values held in the dependency result frames
  for (unsigned i = args_offs; i < args; ++i) {
    // Use api_->frame_get_cptr1 to obtain the value stored in the desired field of
    // input i
    *res += *(double *)api_->frame_get_cptr1(argv[i], 0, 0);
  }

  // return true on successful initialization
  return true;
}

/**
 * The actual function used to execute the operation. It is provided
 * with a call context and an array of data frame objects from operator
 * dependencies. It should return true if result has changed.
 *
 * @param result frame of the operator
 * @param args number of dependency frames
 * @param argv array of dependency frames
 * @param ctx call context
 * @param cl call context closure
 * @return true if result was updated, false otherwise
 */
static bool ema_ext_comp_stream_exec(fm_frame_t *result, size_t args,
                                     const fm_frame_t *const argv[],
                                     fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  // Get the closure of the operator in the desired type
  ema_ext_comp_closure *comp_cl = (ema_ext_comp_closure *)ctx->comp;

  // Validate if the operator needs to generate an update
  bool interval = comp_cl->interval | (args < 2);

  // Clear the condition for the current update
  comp_cl->interval = 0;

  // Get the alpha for the EMA
  double alpha = comp_cl->alpha;

  // Get the pointer to the result value in the closure
  double *res = &comp_cl->result;

  // EMA calculation
  *res = (1 - alpha) * (*res);

  // Skip the trigger argument
  int args_offs = (args > 1);
  for (unsigned i = args_offs; i < args; ++i) {
    // Use api_->frame_get_cptr1 to obtain the value stored in the desired field of
    // input i
    *res += alpha * *(double *)api_->frame_get_cptr1(argv[i], 0, 0);
  }

  // Update result frame if and only if you return true from exec
  if (interval) {
    // Use api_->frame_get_ptr1 to obtain a pointer to the desired field of the
    // result frame
    *(double *)api_->frame_get_ptr1(result, 0, 0) = *res;
  }

  // return true if the result frame was updated
  return interval;
}

/**
 * The function is called when the context is being created to setup call
 * definition callbacks that will be used uppon initialization, execution and
 * destruction of the operator instance.
 *
 * @param comp_cl computation closure
 * @param ctx_cl operator instance closure
 * @return fm_call_def_t object
 */
static fm_call_def_t *ema_ext_comp_stream_call(fm_comp_def_cl comp_cl,
                                               const fm_ctx_def_cl ctx_cl) {
  // Create a call definition for the operator
  fm_call_def_t *def = api_->call_def_new();

  // Set the initialization callback
  api_->call_def_init_set(def, ema_ext_comp_stream_init);

  // Set the execution callback
  api_->call_def_exec_set(def, ema_ext_comp_stream_exec);

  // return the call definition
  return def;
}

/**
 * The function is called when one of the operator dependencies has been
 * updated.
 *
 * @param idx index of the dependency frame that has changed
 * @param ctx call context closure
 */
static void ema_ext_comp_queuer(size_t idx, fm_call_ctx_t *ctx) {
  // Get the closure of the operator in the desired type
  ema_ext_comp_closure *comp_cl = (ema_ext_comp_closure *)ctx->comp;

  // If idx correspond to the trigger dependency, set the interval to trigger an
  // update
  if (idx == 0) {
    comp_cl->interval = 1;
  }
}

/**
 * The function generates the operator context definition and it is called when
 * the graph node is being created.
 *
 * Reporting any errors cased by argument or parameter validation or incorrect
 * internal behaviour can be done by using the typing system's
 * api_->type_sys_err_custom to allow the platform to process internal errors
 * during the operator generation.
 *
 * The operator context definition can provide information to the platform to
 * setup the call context callbacks, the result frame type and additional
 * information for the operator execution.
 *
 * @param csys computing system that can be used for error handling and type
 * system access
 * @param ccl computation closure as set in the computation definition
 * @param args number of dependency types
 * @param argv array of dependency types
 * @param ptype argument type description
 * @param plist argument stack
 * @return fm_ctx_def_t object on success, nullptr otherwise
 */
static fm_ctx_def_t *ema_ext_comp_gen(fm_comp_sys_t *csys, fm_comp_def_cl ccl,
                                      unsigned argc, fm_type_decl_cp *argv,
                                      fm_type_decl_cp ptype,
                                      fm_arg_stack_t plist) {
  // Obtain the type system from the computing system
  fm_type_sys_t *sys = api_->type_sys_get(csys);

  // Validate that operator has at least one dependency
  if (argc == 0) {
    const char *errstr =
        "at least one computation must be provided as an argument";
    api_->type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return NULL;
  }

  // Validate that there is at least one parameter
  if (!ptype) {
    const char *errstr = "expecting a floating point alpha as a parameter, "
                         "unable to find parameters";
    api_->type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return NULL;
  }

  // Validate that the parameters are provided as a tuple
  if (!api_->type_is_tuple(ptype)) {
    const char *errstr = "expecting a floating point alpha as a parameter, "
                         "parameters are not a tuple";
    api_->type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return NULL;
  }

  // Validate that there is a single element in the parameters tuple
  if (api_->type_tuple_size(ptype) != 1) {
    const char *errstr = "expecting a floating point alpha as a parameter, "
                         "incorrect parameter tuple size";
    api_->type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return NULL;
  }

  // Obtain the type of the first element from the parameters tuple
  fm_type_decl_cp alpha_param = api_->type_tuple_arg(ptype, 0);

  // Validate that the type of the first element is a valid float value
  if (!api_->type_is_float(alpha_param)) {
    const char *errstr =
        "expecting a floating point alpha as a parameter, type is not float";
    api_->type_sys_err_custom(sys, FM_TYPE_ERROR_PARAMS, errstr);
    return NULL;
  }

  // Obtain the double value from the parameter stack and advance the pointer to
  // the next parameter
  double alpha = STACK_POP(plist, double);

  // Skip the trigger argument
  int argc_offs = (argc > 1);

  // Obtain the double type declaration from the type system
  fm_type_decl_cp double_field_t = api_->base_type_get(sys, FM_TYPE_FLOAT64);

  for (unsigned i = argc_offs; i < argc; ++i) {
    // Validate that all dependencies have a single field in the result frames.
    if (api_->type_frame_nfields(argv[i]) != 1) {
      const char *errstr = "the frames must have a single field";
      api_->type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
      return NULL;
    }

    // Obtain type declaration of the result frame field from the type system
    fm_type_decl_cp field_t = api_->type_frame_field_type(argv[i], 0);

    // Validate that the type declaration of the result frame field is for a
    // double type
    if (!api_->type_equal(field_t, double_field_t)) {
      const char *errstr = "the type of the fields in the input operators must "
                           "be double (FLOAT64)";
      api_->type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
      return NULL;
    }
  }

  // Build the result frame type
  // Create an array with the names of the fields of the result frame
  const char *names[1] = {"result"};
  // Create an array with the types of the fields of the result frame
  fm_type_decl_cp types[1] = {api_->base_type_get(sys, FM_TYPE_FLOAT64)};
  // Create an array with the dimensions of the fields of the result frame
  int dims[1] = {1};

  // Generate the type declaration of the result frame type
  fm_type_decl_cp ret_type = api_->frame_type_get1(sys, 1, names, types, 1, dims);
  if (!ret_type) {
    const char *errstr = "unable to create result frame type";
    api_->type_sys_err_custom(sys, FM_TYPE_ERROR_ARGS, errstr);
    return NULL;
  }

  // Allocate memory for operator closure
  ema_ext_comp_closure *cl =
      (ema_ext_comp_closure *)calloc(1, sizeof(ema_ext_comp_closure));

  // Initialize EMA alpha value in the operator closure
  cl->alpha = alpha;

  // Create the operator context definition. Providing information to the
  // platform to setup the call context callbacks, the result frame type and
  // additional information for the operator execution.
  fm_ctx_def_t *def = api_->ctx_def_new();

  // Set inplace to false to let the platform know that this operator generates
  // a result
  api_->ctx_def_inplace_set(def, false);

  // Set the result frame type
  api_->ctx_def_type_set(def, ret_type);

  // Set the operator closure
  api_->ctx_def_closure_set(def, cl);

  // Set the queuer function
  api_->ctx_def_queuer_set(def, &ema_ext_comp_queuer);

  // Set the call definition generator function for stream context
  api_->ctx_def_stream_call_set(def, &ema_ext_comp_stream_call);

  // Set the call definition generator function for query context
  api_->ctx_def_query_call_set(def, NULL);
  return def;
}

/**
 * The function is called when the operator is being destroyed
 *
 * @param cl computation closure
 * @param def computation context definition
 */
static void ema_ext_comp_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  // Get the closure of the operator in the desired type
  ema_ext_comp_closure *comp_cl =
      (ema_ext_comp_closure *)api_->ctx_def_closure(def);

  // Release the computation closure memory if it is allocated
  if (comp_cl) {
    free(comp_cl);
  }
}

/**
 * Computation definition
 * It must provide the following information to describe a computation:
 * name: used by the computing system to identify the computation, this name
 * MUST be unique. generate: reference to the operator's generator function
 * destroy: reference to the operator's destruction function
 * closure: pointer to the closure that is shared between all operator instances
 */
fm_comp_def_t ema_comp_def = {
    "ema",                 // name
    &ema_ext_comp_gen,     // generate
    &ema_ext_comp_destroy, // destroy
    NULL                   // closure
};

/**
 * Registers the operator definition in the computational system.
 * The name of this function MUST start with ExtractorInit_ to allow the system
 * extension loader to find the function in external modules.
 *
 * @param sys computing system
 */
FMMODFUNC void ExtractorInit_ema(struct extractor_api_v1 *api,
                                 fm_comp_sys_t *sys, fmc_error_t **error) {
  fmc_error_clear(error);
  api_ = api;
  api_->comp_type_add(sys, &ema_comp_def);
}
