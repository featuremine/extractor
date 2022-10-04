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
 * @file field_util.hpp
 * @author Andres Rangel
 * @date 24 Dec 2018
 * @brief Field utilities
 *
 * This file defines some field execution closure utilities
 */

#pragma once

extern "C" {
#include "extractor/type_decl.h"
}

#include "extractor/frame.hpp"
#include "fmc++/mpl.hpp"

template <class T, template <class> class C, class... Ts, class... Args>
T *get_field_exec_cl(fmc::type_list<Ts...>, fm_type_decl_cp f_type,
                     Args &&... args) {
  T *result = nullptr;
  auto create = [&](auto t) {
    using Tt = decltype(t);
    using Tn = typename Tt::type;
    auto obj = fm::frame_field_type<Tn>();
    if (!result && obj.validate(f_type)) {
      result = new C<Tn>(std::forward<Args>(args)...);
    }
  };
  (create(fmc::typify<Ts>()), ...);
  return result;
}
