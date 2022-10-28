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
 * @file debug.h
 * @author Maxim Trokhimtchouk
 * @date 4 Aug 2017
 * @brief File contains utilities for debugging
 * @see http://www.featuremine.com
 */

#ifndef __FM_DEBUG_H__
#define __FM_DEBUG_H__

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnullability-completeness"

#include <signal.h>

#pragma clang diagnostic pop

#define BREAKPT (raise(SIGTRAP));

#endif // __FM_DEBUG_H__
