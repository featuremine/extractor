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
 * @file exec_ctx.cpp
 * @author Maxim Trokhimtchouk
 * @date 15 Sept 2017
 * @brief File contains C++ definition of the execution context
 *
 * This file contains declarations of the execution context
 * @see http://www.featuremine.com
 */

#include "exec_ctx.hpp"

#include <stdarg.h>
#include <string>
#include <vector>

bool fm_exec_ctx_is_error(fm_exec_ctx_t *ctx) { return !ctx->errmsg.empty(); }

const char *fm_exec_ctx_error_msg(fm_exec_ctx_t *ctx) {
  return ctx->errmsg.c_str();
}

void fm_exec_ctx_error_set(fm_exec_ctx_t *ctx, const char *fmt, ...) {
  va_list args1;
  va_start(args1, fmt);
  va_list args2;
  va_copy(args2, args1);
  auto &buf = ctx->errmsg;
  auto size = vsnprintf(NULL, 0, fmt, args1) + 1;
  std::vector<char> errmsg(size);
  va_end(args1);
  vsnprintf(errmsg.data(), size, fmt, args2);
  va_end(args2);
  buf.clear();
  buf.append(errmsg.data(), size);
}

void fm_exec_ctx_error_clear(fm_exec_ctx_t *ctx) { ctx->errmsg.clear(); }

fm_frame_alloc_t *fm_exec_ctx_frames(fm_exec_ctx_t *ctx) { return ctx->frames; }
