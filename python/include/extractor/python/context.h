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
#pragma once

#include "comp_graph.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/stream_ctx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ExtractorStreamContext ExtractorStreamContext;
FMMODFUNC fm_stream_ctx_t *get_stream_ctx(ExtractorStreamContext *self);

#ifdef __cplusplus
}
#endif
