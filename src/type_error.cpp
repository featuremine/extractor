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
 * @file type_error.cpp
 * @author Maxim Trokhimtchouk
 * @date 25 Jul 2017
 * @brief File contains C++ implementation of the type space object
 *
 * This file contains implementation of the type space object
 * used by FeatureMine Extractor
 * @see http://www.featuremine.com
 */

#include "type_error.hpp"

const char *FM_ERROR_MESSAGES[FM_TYPE_ERROR_LAST] = {
    "ok",
    "using field as a child "
    "type",
    "a duplicated field name",
    "incorrect operator arguments",
    "incorrect operator parameters",
    "negative dimension",
    "unknown error"};
