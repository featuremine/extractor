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
 * @file serial.h
 * @author Maxim Trokhimtchouk
 * @date 10 Jul 2017
 * @brief File contains C declaration of the serialization callbacks
 * @see http://www.featuremine.com
 */

#pragma once

#include "fmc/platform.h"
#include "extractor/api.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*fm_reader)(void *data, size_t limit, void *closure);

#ifdef __cplusplus
}
#endif
