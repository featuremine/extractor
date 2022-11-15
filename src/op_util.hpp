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
 * @file op_util.hpp
 * @author Andres Rangel
 * @date 22 Aug 2018
 * @brief File contains definition for op_field exec structure
 *
 * Structure used in multiple operators to create operations that
 * will be performed in fields of a frame.
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/stream_ctx.h"
#include "fmc/time.h"
}

#include "extractor/frame.hpp"
#include "fmc++/mpl.hpp"
#include "fmc++/rprice.hpp"
#include "fmc++/time.hpp"

#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#pragma once

struct op_field_exec {
  virtual ~op_field_exec() {}
  virtual void exec(fm_frame_t *result, size_t,
                    const fm_frame_t *const argv[]) = 0;
};
