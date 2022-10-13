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
 * @file exec_ctx.hpp
 * @author Maxim Trokhimtchouk
 * @date 18 Sept 2017
 * @brief File contains C declaration of the execution context
 *
 * This file contains declarations of the execution context
 * @see http://www.featuremine.com
 */

#pragma once

extern "C" {
#include "extractor/exec_ctx.h"
}

#include <string>

struct fm_exec_ctx {
  fm_exec_ctx() { frames = fm_frame_alloc_new(); }
  ~fm_exec_ctx() { fm_frame_alloc_del(frames); }
  std::string errmsg;
  fm_frame_alloc_t *frames = nullptr;
};
