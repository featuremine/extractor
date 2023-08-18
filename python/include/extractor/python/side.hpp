#pragma once

#include "Python.h"
#include "fmc++/side.hpp"

struct TradeSideStruct {
  PyObject_VAR_HEAD fmc::trade_side side_;
};
