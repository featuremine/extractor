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
 * @file py_decimal128.hpp
 * @date 2 Nov 2022
 * @brief Python wrapper for extractor decimal128 type
 * */

#pragma once

#include <Python.h>
#include "fmc/decimal128.h"

struct ExtractorBaseTypeDecimal128 {
  PyObject_HEAD;
  fmc_decimal128_t val;
  static void py_dealloc(ExtractorBaseTypeDecimal128 *self) {
    Py_TYPE(self)->tp_free((PyObject *)self);
  }
  static PyObject *py_richcmp(PyObject *obj1, PyObject *obj2, int op);
  static PyObject *tp_new(PyTypeObject *subtype, PyObject *args,
                          PyObject *kwds);
  static PyObject *py_new(fmc_decimal128_t t);
  static PyObject *tp_str(PyObject *self);
  static bool init(PyObject *m);

  static PyObject *is_nan(PyObject *self, PyObject *args);

  static PyMethodDef tp_methods [];
};

PyMethodDef ExtractorBaseTypeDecimal128::tp_methods [] = {
  // /* Unary arithmetic functions, optional context arg */
  // { "exp", _PyCFunction_CAST(dec_mpd_qexp), METH_VARARGS|METH_KEYWORDS, doc_exp },
  // { "ln", _PyCFunction_CAST(dec_mpd_qln), METH_VARARGS|METH_KEYWORDS, doc_ln },
  // { "log10", _PyCFunction_CAST(dec_mpd_qlog10), METH_VARARGS|METH_KEYWORDS, doc_log10 },
  // { "next_minus", _PyCFunction_CAST(dec_mpd_qnext_minus), METH_VARARGS|METH_KEYWORDS, doc_next_minus },
  // { "next_plus", _PyCFunction_CAST(dec_mpd_qnext_plus), METH_VARARGS|METH_KEYWORDS, doc_next_plus },
  // { "normalize", _PyCFunction_CAST(dec_mpd_qreduce), METH_VARARGS|METH_KEYWORDS, doc_normalize },
  // { "to_integral", _PyCFunction_CAST(PyDec_ToIntegralValue), METH_VARARGS|METH_KEYWORDS, doc_to_integral },
  // { "to_integral_exact", _PyCFunction_CAST(PyDec_ToIntegralExact), METH_VARARGS|METH_KEYWORDS, doc_to_integral_exact },
  // { "to_integral_value", _PyCFunction_CAST(PyDec_ToIntegralValue), METH_VARARGS|METH_KEYWORDS, doc_to_integral_value },
  // { "sqrt", _PyCFunction_CAST(dec_mpd_qsqrt), METH_VARARGS|METH_KEYWORDS, doc_sqrt },

  // /* Binary arithmetic functions, optional context arg */
  // { "compare", _PyCFunction_CAST(dec_mpd_qcompare), METH_VARARGS|METH_KEYWORDS, doc_compare },
  // { "compare_signal", _PyCFunction_CAST(dec_mpd_qcompare_signal), METH_VARARGS|METH_KEYWORDS, doc_compare_signal },
  // { "max", _PyCFunction_CAST(dec_mpd_qmax), METH_VARARGS|METH_KEYWORDS, doc_max },
  // { "max_mag", _PyCFunction_CAST(dec_mpd_qmax_mag), METH_VARARGS|METH_KEYWORDS, doc_max_mag },
  // { "min", _PyCFunction_CAST(dec_mpd_qmin), METH_VARARGS|METH_KEYWORDS, doc_min },
  // { "min_mag", _PyCFunction_CAST(dec_mpd_qmin_mag), METH_VARARGS|METH_KEYWORDS, doc_min_mag },
  // { "next_toward", _PyCFunction_CAST(dec_mpd_qnext_toward), METH_VARARGS|METH_KEYWORDS, doc_next_toward },
  // { "quantize", _PyCFunction_CAST(dec_mpd_qquantize), METH_VARARGS|METH_KEYWORDS, doc_quantize },
  // { "remainder_near", _PyCFunction_CAST(dec_mpd_qrem_near), METH_VARARGS|METH_KEYWORDS, doc_remainder_near },

  // /* Ternary arithmetic functions, optional context arg */
  // { "fma", _PyCFunction_CAST(dec_mpd_qfma), METH_VARARGS|METH_KEYWORDS, doc_fma },

  // /* Boolean functions, no context arg */
  // { "is_canonical", dec_mpd_iscanonical, METH_NOARGS, doc_is_canonical },
  // { "is_finite", dec_mpd_isfinite, METH_NOARGS, doc_is_finite },
  // { "is_infinite", dec_mpd_isinfinite, METH_NOARGS, doc_is_infinite },
  { "is_nan", &ExtractorBaseTypeDecimal128::is_nan, METH_NOARGS, NULL },
  // { "is_qnan", dec_mpd_isqnan, METH_NOARGS, doc_is_qnan },
  // { "is_snan", dec_mpd_issnan, METH_NOARGS, doc_is_snan },
  // { "is_signed", dec_mpd_issigned, METH_NOARGS, doc_is_signed },
  // { "is_zero", dec_mpd_iszero, METH_NOARGS, doc_is_zero },

  // /* Boolean functions, optional context arg */
  // { "is_normal", _PyCFunction_CAST(dec_mpd_isnormal), METH_VARARGS|METH_KEYWORDS, doc_is_normal },
  // { "is_subnormal", _PyCFunction_CAST(dec_mpd_issubnormal), METH_VARARGS|METH_KEYWORDS, doc_is_subnormal },

  // /* Unary functions, no context arg */
  // { "adjusted", dec_mpd_adjexp, METH_NOARGS, doc_adjusted },
  // { "canonical", dec_canonical, METH_NOARGS, doc_canonical },
  // { "conjugate", dec_conjugate, METH_NOARGS, doc_conjugate },
  // { "radix", dec_mpd_radix, METH_NOARGS, doc_radix },

  // /* Unary functions, optional context arg for conversion errors */
  // { "copy_abs", dec_mpd_qcopy_abs, METH_NOARGS, doc_copy_abs },
  // { "copy_negate", dec_mpd_qcopy_negate, METH_NOARGS, doc_copy_negate },

  // /* Unary functions, optional context arg */
  // { "logb", _PyCFunction_CAST(dec_mpd_qlogb), METH_VARARGS|METH_KEYWORDS, doc_logb },
  // { "logical_invert", _PyCFunction_CAST(dec_mpd_qinvert), METH_VARARGS|METH_KEYWORDS, doc_logical_invert },
  // { "number_class", _PyCFunction_CAST(dec_mpd_class), METH_VARARGS|METH_KEYWORDS, doc_number_class },
  // { "to_eng_string", _PyCFunction_CAST(dec_mpd_to_eng), METH_VARARGS|METH_KEYWORDS, doc_to_eng_string },

  // /* Binary functions, optional context arg for conversion errors */
  // { "compare_total", _PyCFunction_CAST(dec_mpd_compare_total), METH_VARARGS|METH_KEYWORDS, doc_compare_total },
  // { "compare_total_mag", _PyCFunction_CAST(dec_mpd_compare_total_mag), METH_VARARGS|METH_KEYWORDS, doc_compare_total_mag },
  // { "copy_sign", _PyCFunction_CAST(dec_mpd_qcopy_sign), METH_VARARGS|METH_KEYWORDS, doc_copy_sign },
  // { "same_quantum", _PyCFunction_CAST(dec_mpd_same_quantum), METH_VARARGS|METH_KEYWORDS, doc_same_quantum },

  // /* Binary functions, optional context arg */
  // { "logical_and", _PyCFunction_CAST(dec_mpd_qand), METH_VARARGS|METH_KEYWORDS, doc_logical_and },
  // { "logical_or", _PyCFunction_CAST(dec_mpd_qor), METH_VARARGS|METH_KEYWORDS, doc_logical_or },
  // { "logical_xor", _PyCFunction_CAST(dec_mpd_qxor), METH_VARARGS|METH_KEYWORDS, doc_logical_xor },
  // { "rotate", _PyCFunction_CAST(dec_mpd_qrotate), METH_VARARGS|METH_KEYWORDS, doc_rotate },
  // { "scaleb", _PyCFunction_CAST(dec_mpd_qscaleb), METH_VARARGS|METH_KEYWORDS, doc_scaleb },
  // { "shift", _PyCFunction_CAST(dec_mpd_qshift), METH_VARARGS|METH_KEYWORDS, doc_shift },

  // /* Miscellaneous */
  // { "from_float", dec_from_float, METH_O|METH_CLASS, doc_from_float },
  // { "as_tuple", PyDec_AsTuple, METH_NOARGS, doc_as_tuple },
  // { "as_integer_ratio", dec_as_integer_ratio, METH_NOARGS, doc_as_integer_ratio },

  // /* Special methods */
  // { "__copy__", dec_copy, METH_NOARGS, NULL },
  // { "__deepcopy__", dec_copy, METH_O, NULL },
  // { "__format__", dec_format, METH_VARARGS, NULL },
  // { "__reduce__", dec_reduce, METH_NOARGS, NULL },
  // { "__round__", PyDec_Round, METH_VARARGS, NULL },
  // { "__ceil__", dec_ceil, METH_NOARGS, NULL },
  // { "__floor__", dec_floor, METH_NOARGS, NULL },
  // { "__trunc__", dec_trunc, METH_NOARGS, NULL },
  // { "__complex__", dec_complex, METH_NOARGS, NULL },
  // { "__sizeof__", dec_sizeof, METH_NOARGS, NULL },

  { NULL, NULL, 1 }
};

static PyTypeObject ExtractorBaseTypeDecimal128Type = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Decimal128", /* tp_name */
    sizeof(ExtractorBaseTypeDecimal128),                   /* tp_basicsize */
    0,                                                 /* tp_itemsize */
    (destructor)ExtractorBaseTypeDecimal128::py_dealloc,   /* tp_dealloc */
    0,                                                 /* tp_print */
    0,                                                 /* tp_getattr */
    0,                                                 /* tp_setattr */
    0,                                                 /* tp_reserved */
    0,                                                 /* tp_repr */
    0,                                                 /* tp_as_number */
    0,                                                 /* tp_as_sequence */
    0,                                                 /* tp_as_mapping */
    0,                                                 /* tp_hash  */
    0,                                                 /* tp_call */
    (reprfunc)ExtractorBaseTypeDecimal128::tp_str,         /* tp_str */
    0,                                                 /* tp_getattro */
    0,                                                 /* tp_setattro */
    0,                                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,          /* tp_flags */
    "Extractor system base type object",               /* tp_doc */
    0,                                                 /* tp_traverse */
    0,                                                 /* tp_clear */
    (richcmpfunc)ExtractorBaseTypeDecimal128::py_richcmp,  /* tp_richcompare */
    0,                               /* tp_weaklistoffset */
    0,                               /* tp_iter */
    0,                               /* tp_iternext */
    ExtractorBaseTypeDecimal128::tp_methods,                               /* tp_methods */
    0,                               /* tp_members */
    0,                               /* tp_getset */
    0,                               /* tp_base */
    0,                               /* tp_dict */
    0,                               /* tp_descr_get */
    0,                               /* tp_descr_set */
    0,                               /* tp_dictoffset */
    0,                               /* tp_init */
    0,                               /* tp_alloc */
    ExtractorBaseTypeDecimal128::tp_new, /* tp_new */
};
PyObject *ExtractorBaseTypeDecimal128::py_new(fmc_decimal128_t t) {
  PyTypeObject *type = (PyTypeObject *)&ExtractorBaseTypeDecimal128Type;
  ExtractorBaseTypeDecimal128 *self;

  self = (ExtractorBaseTypeDecimal128 *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->val = t;
  return (PyObject *)self;
}
PyObject *ExtractorBaseTypeDecimal128::tp_new(PyTypeObject *subtype,
                                          PyObject *args, PyObject *kwds) {
  // PyObject *input = NULL;
  // if (PyArg_ParseTuple(args, "O", &input) &&
  //     ExtractorComputation_type_check(input))
  //   return create(subtype, args, kwds);
  // fmc_decimal128_t val;
  // if (py_type_convert<fmc_decimal128_t>::convert(val, args)) {
  //   return py_new(val);
  // }
  PyErr_SetString(PyExc_RuntimeError, "Could not convert to type Decimal128");
  return nullptr;
}
PyObject *ExtractorBaseTypeDecimal128::tp_str(PyObject *self) {
  std::string str = std::to_string(((ExtractorBaseTypeDecimal128 *)self)->val);
  return PyUnicode_FromString(str.c_str());
}

bool ExtractorBaseTypeDecimal128::init(PyObject *m) {
  if (PyType_Ready(&ExtractorBaseTypeDecimal128Type) < 0)
    return false;
  Py_INCREF(&ExtractorBaseTypeDecimal128Type);
  PyModule_AddObject(m, "Decimal128", (PyObject *)&ExtractorBaseTypeDecimal128Type);
  return true;
}

PyObject *ExtractorBaseTypeDecimal128::py_richcmp(PyObject *obj1,
                                              PyObject *obj2, int op) {
  auto type = &ExtractorBaseTypeDecimal128Type;
  if (!PyObject_TypeCheck(obj1, type) || !PyObject_TypeCheck(obj2, type)) {
    if (op == Py_NE) {
      Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
  }
  int c = 0;
  fmc_decimal128_t t1;
  fmc_decimal128_t t2;
  t1 = ((ExtractorBaseTypeDecimal128 *)obj1)->val;
  t2 = ((ExtractorBaseTypeDecimal128 *)obj2)->val;
  switch (op) {
  case Py_LT:
    c = t1 < t2;
    break;
  case Py_LE:
    c = t1 <= t2;
    break;
  case Py_EQ:
    c = t1 == t2;
    break;
  case Py_NE:
    c = t1 != t2;
    break;
  case Py_GT:
    c = t1 > t2;
    break;
  case Py_GE:
    c = t1 >= t2;
    break;
  }
  return PyBool_FromLong(c);
}

PyObject *ExtractorBaseTypeDecimal128::is_nan(PyObject *self, PyObject *args) {
  return PyBool_FromLong(fmc_decimal128_is_nan(&((ExtractorBaseTypeDecimal128 *)self)->val));
}
