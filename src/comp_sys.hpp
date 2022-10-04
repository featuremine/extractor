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
 * @file comp_sys.cpp
 * @author Maxim Trokhimtchouk
 * @date 1 Aug 2017
 * @brief File contains C++ implementation of the computational system
 *
 * This file contains implementation of the computational system
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/arg_stack.h"
#include "extractor/comp_sys.h"
#include "extractor/frame.h"
#include "extractor/module.h"
#include "comp.h"
#include "comp_graph.h"
#include "frame_serial.h"
#include "extractor/stream_ctx.h"
#include "extractor/time64.h"
#include "extractor/type_sys.h"
#include <cmp/cmp.h>
}

#include "mp_util.hpp"
#include "serial_util.hpp"

#include <dlfcn.h>
#include <fmc++/counters.hpp>
#include <functional>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

#include <fmc/platform.h>
#ifdef FMC_LICENSE
#include <license.h>
#endif // FMC_LICENSE
using namespace std;

/**
 * @note need to add errors to comp sys as well
 */

struct fm_comp_sys {
  fm_type_sys_t *types = nullptr;
  vector<fm_comp_graph_t *> graphs;
  unordered_map<string, fm_comp_def_t> defs;
  vector<function<void()>> destructors;
  std::string errmsg;
  unordered_map<string, fm_module_t *> modules_;
  unsigned modules_suff_;
#ifdef FMC_LICENSE
  RLM_HANDLE rh;
  RLM_LICENSE lic = (RLM_LICENSE)NULL;
#endif // FMC_LICENSE
  fmc::counter::samples samples_;
};
