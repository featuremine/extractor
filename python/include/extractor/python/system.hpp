#include "Python.h"
#include "extractor/comp_def.h"
#include "extractor/std_comp.h"
#include <vector>

typedef struct {
  PyObject_HEAD fm_comp_sys_t *sys;
  vector<fm_comp_def_t> custom;
  bool to_delete;
} ExtractorSystem;

