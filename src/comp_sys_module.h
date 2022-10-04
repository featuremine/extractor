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

#include "extractor/comp_sys.h"
#include "extractor/module.h"
#include "comp_graph.h"
#include "frame_serial.h"

char *fm_module_uniq_name_gen(fm_comp_sys_t *sys);
bool fm_module_name_add(fm_comp_sys_t *sys, const char *name, fm_module_t *m);
fm_module_t *fm_module_name_find(fm_comp_sys_t *sys, const char *name);
