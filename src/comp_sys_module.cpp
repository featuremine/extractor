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

extern "C" {
#include "comp_sys_module.h"
}

#include "comp_sys.hpp"
#include <fmc++/mpl.hpp>
#include <string>

using namespace std;

char *fm_module_uniq_name_gen(fm_comp_sys_t *sys) {
  string name = "module_";
  size_t s = name.size();
  auto &count = sys->modules_suff_;

  while (fm_module_name_find(sys, fmc::append_int(name, count).c_str())) {
    name.resize(s);
    ++count;
  }
  auto sz = name.size();
  char *name_ptr = (char *)malloc(sz + 1);
  memcpy(name_ptr, name.data(), sz);
  name_ptr[sz] = 0;
  return name_ptr;
}

bool fm_module_name_add(fm_comp_sys_t *sys, const char *name, fm_module_t *m) {
  return sys->modules_.emplace(name, m).second;
}

fm_module_t *fm_module_name_find(fm_comp_sys_t *sys, const char *name) {
  auto where = sys->modules_.find(name);
  if (where == sys->modules_.end())
    return nullptr;
  return where->second;
}
