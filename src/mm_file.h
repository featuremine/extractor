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
 * @file mm_file.h
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C definitions of the mm_file object and API
 *
 * This file contains declarations of the mm_file object and API
 * @see http://www.featuremine.com
 */

#pragma once

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mm_file mm_file_t;

mm_file_t *mm_file_open(const char *file_path);

size_t mm_file_read(mm_file *m, size_t limit, void **buf);

void mm_file_rewind(mm_file_t *m);

void mm_file_close(mm_file_t *m);
