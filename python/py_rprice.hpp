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
 * @file py_rprice.hpp
 * @date 2 Nov 2022
 * @brief Python wrapper for extractor rprice type
 * */

#pragma once

#include "fmc++/rprice.hpp"
#include <Python.h>
#include <extractor/python/extractor.h>
#include <py_type_utils.hpp>

struct ExtractorBaseTypeRprice {
  PyObject_HEAD;
  fmc::rprice val;
  static void py_dealloc(ExtractorBaseTypeRprice *self) {
    Py_TYPE(self)->tp_free((PyObject *)self);
  }
  static PyObject *py_richcmp(PyObject *obj1, PyObject *obj2, int op);
  static PyObject *tp_new(PyTypeObject *subtype, PyObject *args,
                          PyObject *kwds);
  static PyObject *py_new(fmc_rprice_t t);
  static PyObject *tp_str(PyObject *self);
  static bool init(PyObject *m);

  static PyObject *is_finite(PyObject *self, PyObject *args);
  static PyObject *is_infinite(PyObject *self, PyObject *args);
  static PyObject *is_nan(PyObject *self, PyObject *args);
  static PyObject *is_qnan(PyObject *self, PyObject *args);
  static PyObject *is_snan(PyObject *self, PyObject *args);
  static PyObject *is_signed(PyObject *self, PyObject *args);
  static PyObject *is_zero(PyObject *self, PyObject *args);
  static PyObject *compare(PyObject *self, PyObject *args);
  static PyObject *max(PyObject *self, PyObject *args);
  static PyObject *min(PyObject *self, PyObject *args);
  static PyObject *from_float(PyObject *self, PyObject *args);

  static PyObject *nb_add(PyObject *lhs, PyObject *rhs);
  static PyObject *nb_substract(PyObject *lhs, PyObject *rhs);
  static PyObject *nb_multiply(PyObject *lhs, PyObject *rhs);

  static PyObject *nb_absolute(PyObject *self);

  static PyObject *nb_int(PyObject *self);
  static PyObject *nb_float(PyObject *self);

  static PyObject *nb_inplace_add(PyObject *self, PyObject *rhs);
  static PyObject *nb_inplace_substract(PyObject *self, PyObject *rhs);
  static PyObject *nb_inplace_multiply(PyObject *self, PyObject *rhs);

  static PyObject *nb_true_divide(PyObject *lhs, PyObject *rhs);
  static PyObject *nb_inplace_true_divide(PyObject *self, PyObject *rhs);

  static PyNumberMethods tp_as_number;
  static PyMethodDef tp_methods[];
};

PyObject *ExtractorBaseTypeRprice::nb_add(PyObject *lhs, PyObject *rhs) {
  return ExtractorBaseTypeRprice::py_new(((ExtractorBaseTypeRprice *)lhs)->val +
                                         ((ExtractorBaseTypeRprice *)rhs)->val);
}

PyObject *ExtractorBaseTypeRprice::nb_substract(PyObject *lhs, PyObject *rhs) {
  return ExtractorBaseTypeRprice::py_new(((ExtractorBaseTypeRprice *)lhs)->val -
                                         ((ExtractorBaseTypeRprice *)rhs)->val);
}

PyObject *ExtractorBaseTypeRprice::nb_multiply(PyObject *lhs, PyObject *rhs) {
  return ExtractorBaseTypeRprice::py_new(((ExtractorBaseTypeRprice *)lhs)->val *
                                         ((ExtractorBaseTypeRprice *)rhs)->val);
}

PyObject *ExtractorBaseTypeRprice::nb_true_divide(PyObject *self,
                                                  PyObject *rhs) {
  return ExtractorBaseTypeRprice::py_new(
      ((ExtractorBaseTypeRprice *)self)->val /
      ((ExtractorBaseTypeRprice *)rhs)->val);
}

PyObject *ExtractorBaseTypeRprice::nb_inplace_add(PyObject *self,
                                                  PyObject *rhs) {
  ((ExtractorBaseTypeRprice *)self)->val +=
      ((ExtractorBaseTypeRprice *)rhs)->val;
  Py_INCREF(self);
  return self;
}

PyObject *ExtractorBaseTypeRprice::nb_inplace_substract(PyObject *self,
                                                        PyObject *rhs) {
  ((ExtractorBaseTypeRprice *)self)->val -=
      ((ExtractorBaseTypeRprice *)rhs)->val;
  Py_INCREF(self);
  return self;
}

PyObject *ExtractorBaseTypeRprice::nb_inplace_multiply(PyObject *self,
                                                       PyObject *rhs) {
  ((ExtractorBaseTypeRprice *)self)->val =
      ((ExtractorBaseTypeRprice *)self)->val *
      ((ExtractorBaseTypeRprice *)rhs)->val;
  Py_INCREF(self);
  return self;
}

PyObject *ExtractorBaseTypeRprice::nb_inplace_true_divide(PyObject *self,
                                                          PyObject *rhs) {
  ((ExtractorBaseTypeRprice *)self)->val =
      ((ExtractorBaseTypeRprice *)self)->val /
      ((ExtractorBaseTypeRprice *)rhs)->val;
  Py_INCREF(self);
  return self;
}

PyObject *ExtractorBaseTypeRprice::nb_absolute(PyObject *self) {
  fmc_rprice_t absval;
  fmc_rprice_abs(&absval, &((ExtractorBaseTypeRprice *)self)->val);
  return ExtractorBaseTypeRprice::py_new(absval);
}

PyObject *ExtractorBaseTypeRprice::nb_float(PyObject *self) {
  double dest;
  fmc_rprice_to_double(&dest, &((ExtractorBaseTypeRprice *)self)->val);
  return PyFloat_FromDouble(dest);
}

PyObject *ExtractorBaseTypeRprice::nb_int(PyObject *self) {
  int64_t dest;
  fmc_rprice_to_int(&dest, &((ExtractorBaseTypeRprice *)self)->val);
  return PyLong_FromLongLong(dest);
}

PyNumberMethods ExtractorBaseTypeRprice::tp_as_number = {
    ExtractorBaseTypeRprice::nb_add,       //  binaryfunc nb_add;
    ExtractorBaseTypeRprice::nb_substract, //  binaryfunc nb_subtract;
    ExtractorBaseTypeRprice::nb_multiply,  //  binaryfunc nb_multiply;
    0,                                     //  binaryfunc nb_remainder;
    0,                                     //  binaryfunc nb_divmod;
    0,                                     //  ternaryfunc nb_power;
    0,                                     //  unaryfunc nb_negative;
    0,                                     //  unaryfunc nb_positive;
    ExtractorBaseTypeRprice::nb_absolute,  //  unaryfunc nb_absolute;
    0,                                     //  inquiry nb_bool;
    0,                                     //  unaryfunc nb_invert;
    0,                                     //  binaryfunc nb_lshift;
    0,                                     //  binaryfunc nb_rshift;
    0,                                     //  binaryfunc nb_and;
    0,                                     //  binaryfunc nb_xor;
    0,                                     //  binaryfunc nb_or;
    ExtractorBaseTypeRprice::nb_int,       //  unaryfunc nb_int;
    0,                                     //  void *nb_reserved;
    ExtractorBaseTypeRprice::nb_float,     //  unaryfunc nb_float;

    ExtractorBaseTypeRprice::nb_inplace_add, //  binaryfunc nb_inplace_add;
    ExtractorBaseTypeRprice::nb_inplace_substract, //  binaryfunc
                                                   //  nb_inplace_subtract;
    ExtractorBaseTypeRprice::nb_inplace_multiply,  //  binaryfunc
                                                   //  nb_inplace_multiply;
    0, //  binaryfunc nb_inplace_remainder;
    0, //  ternaryfunc nb_inplace_power;
    0, //  binaryfunc nb_inplace_lshift;
    0, //  binaryfunc nb_inplace_rshift;
    0, //  binaryfunc nb_inplace_and;
    0, //  binaryfunc nb_inplace_xor;
    0, //  binaryfunc nb_inplace_or;

    0,                                       //  binaryfunc nb_floor_divide;
    ExtractorBaseTypeRprice::nb_true_divide, //  binaryfunc nb_true_divide;
    0, //  binaryfunc nb_inplace_floor_divide;
    ExtractorBaseTypeRprice::nb_inplace_true_divide, //  binaryfunc
                                                     //  nb_inplace_true_divide;

    0, //  unaryfunc nb_index;

    0, //  binaryfunc nb_matrix_multiply;
    0, //  binaryfunc nb_inplace_matrix_multiply;
};

PyMethodDef ExtractorBaseTypeRprice::tp_methods[] = {
    // /* Unary arithmetic functions, optional context arg */
    // { "exp", _PyCFunction_CAST(dec_mpd_qexp), METH_VARARGS|METH_KEYWORDS,
    // doc_exp },
    // { "ln", _PyCFunction_CAST(dec_mpd_qln), METH_VARARGS|METH_KEYWORDS,
    // doc_ln },
    // { "log10", _PyCFunction_CAST(dec_mpd_qlog10), METH_VARARGS|METH_KEYWORDS,
    // doc_log10 },
    // { "next_minus", _PyCFunction_CAST(dec_mpd_qnext_minus),
    // METH_VARARGS|METH_KEYWORDS, doc_next_minus },
    // { "next_plus", _PyCFunction_CAST(dec_mpd_qnext_plus),
    // METH_VARARGS|METH_KEYWORDS, doc_next_plus },
    // { "normalize", _PyCFunction_CAST(dec_mpd_qreduce),
    // METH_VARARGS|METH_KEYWORDS, doc_normalize },
    // { "to_integral", _PyCFunction_CAST(PyDec_ToIntegralValue),
    // METH_VARARGS|METH_KEYWORDS, doc_to_integral },
    // { "to_integral_exact", _PyCFunction_CAST(PyDec_ToIntegralExact),
    // METH_VARARGS|METH_KEYWORDS, doc_to_integral_exact },
    // { "to_integral_value", _PyCFunction_CAST(PyDec_ToIntegralValue),
    // METH_VARARGS|METH_KEYWORDS, doc_to_integral_value },
    // { "sqrt", _PyCFunction_CAST(dec_mpd_qsqrt), METH_VARARGS|METH_KEYWORDS,
    // doc_sqrt },

    // /* Binary arithmetic functions, optional context arg */
    {"compare", (PyCFunction)ExtractorBaseTypeRprice::compare,
     METH_VARARGS | METH_KEYWORDS, NULL},
    // { "compare_signal", _PyCFunction_CAST(dec_mpd_qcompare_signal),
    // METH_VARARGS|METH_KEYWORDS, doc_compare_signal },
    {"max", ExtractorBaseTypeRprice::max, METH_VARARGS | METH_KEYWORDS, NULL},
    // { "max_mag", _PyCFunction_CAST(dec_mpd_qmax_mag),
    // METH_VARARGS|METH_KEYWORDS, doc_max_mag },
    {"min", ExtractorBaseTypeRprice::min, METH_VARARGS | METH_KEYWORDS, NULL},
    // { "min_mag", _PyCFunction_CAST(dec_mpd_qmin_mag),
    // METH_VARARGS|METH_KEYWORDS, doc_min_mag },
    // { "next_toward", _PyCFunction_CAST(dec_mpd_qnext_toward),
    // METH_VARARGS|METH_KEYWORDS, doc_next_toward },
    // { "quantize", _PyCFunction_CAST(dec_mpd_qquantize),
    // METH_VARARGS|METH_KEYWORDS, doc_quantize },
    // { "remainder_near", _PyCFunction_CAST(dec_mpd_qrem_near),
    // METH_VARARGS|METH_KEYWORDS, doc_remainder_near },

    // /* Ternary arithmetic functions, optional context arg */
    // { "fma", _PyCFunction_CAST(dec_mpd_qfma), METH_VARARGS|METH_KEYWORDS,
    // doc_fma },

    // /* Boolean functions, no context arg */
    // { "is_canonical", dec_mpd_iscanonical, METH_NOARGS, doc_is_canonical },
    {"is_finite", ExtractorBaseTypeRprice::is_finite, METH_NOARGS, NULL},
    {"is_infinite", ExtractorBaseTypeRprice::is_infinite, METH_NOARGS, NULL},
    {"is_nan", ExtractorBaseTypeRprice::is_nan, METH_NOARGS, NULL},
    {"is_qnan", ExtractorBaseTypeRprice::is_qnan, METH_NOARGS, NULL},
    {"is_snan", ExtractorBaseTypeRprice::is_snan, METH_NOARGS, NULL},
    {"is_signed", ExtractorBaseTypeRprice::is_signed, METH_NOARGS, NULL},
    {"is_zero", ExtractorBaseTypeRprice::is_zero, METH_NOARGS, NULL},

    // /* Boolean functions, optional context arg */
    // { "is_normal", _PyCFunction_CAST(dec_mpd_isnormal),
    // METH_VARARGS|METH_KEYWORDS, doc_is_normal },
    // { "is_subnormal", _PyCFunction_CAST(dec_mpd_issubnormal),
    // METH_VARARGS|METH_KEYWORDS, doc_is_subnormal },

    // /* Unary functions, no context arg */
    // { "adjusted", dec_mpd_adjexp, METH_NOARGS, doc_adjusted },
    // { "canonical", dec_canonical, METH_NOARGS, doc_canonical },
    // { "conjugate", dec_conjugate, METH_NOARGS, doc_conjugate },
    // { "radix", dec_mpd_radix, METH_NOARGS, doc_radix },

    // /* Unary functions, optional context arg for conversion errors */
    // { "copy_abs", dec_mpd_qcopy_abs, METH_NOARGS, doc_copy_abs },
    // { "copy_negate", dec_mpd_qcopy_negate, METH_NOARGS, doc_copy_negate },

    // /* Unary functions, optional context arg */
    // { "logb", _PyCFunction_CAST(dec_mpd_qlogb), METH_VARARGS|METH_KEYWORDS,
    // doc_logb },
    // { "logical_invert", _PyCFunction_CAST(dec_mpd_qinvert),
    // METH_VARARGS|METH_KEYWORDS, doc_logical_invert },
    // { "number_class", _PyCFunction_CAST(dec_mpd_class),
    // METH_VARARGS|METH_KEYWORDS, doc_number_class },
    // { "to_eng_string", _PyCFunction_CAST(dec_mpd_to_eng),
    // METH_VARARGS|METH_KEYWORDS, doc_to_eng_string },

    // /* Binary functions, optional context arg for conversion errors */
    // { "compare_total", _PyCFunction_CAST(dec_mpd_compare_total),
    // METH_VARARGS|METH_KEYWORDS, doc_compare_total },
    // { "compare_total_mag", _PyCFunction_CAST(dec_mpd_compare_total_mag),
    // METH_VARARGS|METH_KEYWORDS, doc_compare_total_mag },
    // { "copy_sign", _PyCFunction_CAST(dec_mpd_qcopy_sign),
    // METH_VARARGS|METH_KEYWORDS, doc_copy_sign },
    // { "same_quantum", _PyCFunction_CAST(dec_mpd_same_quantum),
    // METH_VARARGS|METH_KEYWORDS, doc_same_quantum },

    // /* Binary functions, optional context arg */
    // { "logical_and", _PyCFunction_CAST(dec_mpd_qand),
    // METH_VARARGS|METH_KEYWORDS, doc_logical_and },
    // { "logical_or", _PyCFunction_CAST(dec_mpd_qor),
    // METH_VARARGS|METH_KEYWORDS, doc_logical_or },
    // { "logical_xor", _PyCFunction_CAST(dec_mpd_qxor),
    // METH_VARARGS|METH_KEYWORDS, doc_logical_xor },
    // { "rotate", _PyCFunction_CAST(dec_mpd_qrotate),
    // METH_VARARGS|METH_KEYWORDS, doc_rotate },
    // { "scaleb", _PyCFunction_CAST(dec_mpd_qscaleb),
    // METH_VARARGS|METH_KEYWORDS, doc_scaleb },
    // { "shift", _PyCFunction_CAST(dec_mpd_qshift), METH_VARARGS|METH_KEYWORDS,
    // doc_shift },

    // /* Miscellaneous */
    {"from_float", ExtractorBaseTypeRprice::from_float, METH_O | METH_CLASS,
     NULL},
    // { "as_tuple", PyDec_AsTuple, METH_NOARGS, doc_as_tuple },
    // { "as_integer_ratio", dec_as_integer_ratio, METH_NOARGS,
    // doc_as_integer_ratio },

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

    {NULL, NULL, 1}};

static PyTypeObject ExtractorBaseTypeRpriceType = {
    PyVarObject_HEAD_INIT(NULL, 0) "extractor.Rprice", /* tp_name */
    sizeof(ExtractorBaseTypeRprice),                   /* tp_basicsize */
    0,                                                 /* tp_itemsize */
    (destructor)ExtractorBaseTypeRprice::py_dealloc,   /* tp_dealloc */
    0,                                                 /* tp_print */
    0,                                                 /* tp_getattr */
    0,                                                 /* tp_setattr */
    0,                                                 /* tp_reserved */
    0,                                                 /* tp_repr */
    &ExtractorBaseTypeRprice::tp_as_number,            /* tp_as_number */
    0,                                                 /* tp_as_sequence */
    0,                                                 /* tp_as_mapping */
    0,                                                 /* tp_hash  */
    0,                                                 /* tp_call */
    (reprfunc)ExtractorBaseTypeRprice::tp_str,         /* tp_str */
    0,                                                 /* tp_getattro */
    0,                                                 /* tp_setattro */
    0,                                                 /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,          /* tp_flags */
    "Extractor system base type object",               /* tp_doc */
    0,                                                 /* tp_traverse */
    0,                                                 /* tp_clear */
    (richcmpfunc)ExtractorBaseTypeRprice::py_richcmp,  /* tp_richcompare */
    0,                                                 /* tp_weaklistoffset */
    0,                                                 /* tp_iter */
    0,                                                 /* tp_iternext */
    ExtractorBaseTypeRprice::tp_methods,               /* tp_methods */
    0,                                                 /* tp_members */
    0,                                                 /* tp_getset */
    0,                                                 /* tp_base */
    0,                                                 /* tp_dict */
    0,                                                 /* tp_descr_get */
    0,                                                 /* tp_descr_set */
    0,                                                 /* tp_dictoffset */
    0,                                                 /* tp_init */
    0,                                                 /* tp_alloc */
    ExtractorBaseTypeRprice::tp_new,                   /* tp_new */
};
PyObject *ExtractorBaseTypeRprice::py_new(fmc_rprice_t t) {
  PyTypeObject *type = (PyTypeObject *)&ExtractorBaseTypeRpriceType;
  ExtractorBaseTypeRprice *self;

  self = (ExtractorBaseTypeRprice *)type->tp_alloc(type, 0);
  if (self == nullptr)
    return nullptr;

  self->val = t;
  return (PyObject *)self;
}
PyObject *ExtractorBaseTypeRprice::tp_new(PyTypeObject *subtype, PyObject *args,
                                          PyObject *kwds) {
  PyObject *input = NULL;
  if (PyArg_ParseTuple(args, "O", &input) &&
      ExtractorComputation_type_check(input))
    return create(subtype, args, kwds);
  fmc_rprice_t val;
  if (py_type_convert<fmc_rprice_t>::convert(val, args)) {
    return py_new(val);
  }
  PyErr_SetString(PyExc_RuntimeError, "Could not convert to type Rprice");
  return nullptr;
}
PyObject *ExtractorBaseTypeRprice::tp_str(PyObject *self) {
  std::string str = std::to_string(((ExtractorBaseTypeRprice *)self)->val);
  return PyUnicode_FromString(str.c_str());
}

bool ExtractorBaseTypeRprice::init(PyObject *m) {
  if (PyType_Ready(&ExtractorBaseTypeRpriceType) < 0)
    return false;

  /* Numeric abstract base classes */
  PyObject *numbers = PyImport_ImportModule("numbers");
  if (!numbers)
    return false;
  PyObject *Number = PyObject_GetAttrString(numbers, "Number");
  if (!Number)
    return false;
  /* Register Decimal with the Number abstract base class */
  PyObject *obj = PyObject_CallMethod(Number, "register", "(O)",
                                      (PyObject *)&ExtractorBaseTypeRpriceType);
  if (!obj)
    return false;

  Py_CLEAR(obj);
  Py_CLEAR(numbers);
  Py_CLEAR(Number);

  Py_INCREF(&ExtractorBaseTypeRpriceType);
  PyModule_AddObject(m, "Rprice", (PyObject *)&ExtractorBaseTypeRpriceType);

  return true;
}

PyObject *ExtractorBaseTypeRprice::py_richcmp(PyObject *obj1, PyObject *obj2,
                                              int op) {
  if (!Rprice_Check(obj1) || !Rprice_Check(obj2)) {
    return PyBool_FromLong(op == Py_NE);
  }
  int c = 0;
  fmc_rprice_t t1;
  fmc_rprice_t t2;
  t1 = ((ExtractorBaseTypeRprice *)obj1)->val;
  t2 = ((ExtractorBaseTypeRprice *)obj2)->val;
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

PyObject *ExtractorBaseTypeRprice::is_finite(PyObject *self, PyObject *args) {
  Py_RETURN_TRUE;
}

PyObject *ExtractorBaseTypeRprice::is_infinite(PyObject *self, PyObject *args) {
  Py_RETURN_FALSE;
}

PyObject *ExtractorBaseTypeRprice::is_nan(PyObject *self, PyObject *args) {
  Py_RETURN_FALSE;
}

PyObject *ExtractorBaseTypeRprice::is_qnan(PyObject *self, PyObject *args) {
  Py_RETURN_FALSE;
}

PyObject *ExtractorBaseTypeRprice::is_snan(PyObject *self, PyObject *args) {
  Py_RETURN_FALSE;
}

PyObject *ExtractorBaseTypeRprice::is_signed(PyObject *self, PyObject *args) {
  return PyBool_FromLong(((ExtractorBaseTypeRprice *)self)->val.value < 0);
}

PyObject *ExtractorBaseTypeRprice::is_zero(PyObject *self, PyObject *args) {
  return PyBool_FromLong(((ExtractorBaseTypeRprice *)self)->val.value == 0);
}
PyObject *ExtractorBaseTypeRprice::compare(PyObject *self, PyObject *args) {
  PyObject *lhs, *rhs;
  if (!PyArg_ParseTuple(args, "OO", &lhs, &rhs)) {
    return nullptr;
  }
  fmc_rprice_t dlhs;
  if (py_type_convert<fmc_rprice_t>::convert(dlhs, lhs)) {
    return nullptr;
  }
  fmc_rprice_t drhs;
  if (py_type_convert<fmc_rprice_t>::convert(drhs, rhs)) {
    return nullptr;
  }
  if (dlhs < drhs) {
    return PyLong_FromLong(-1);
  }
  return PyLong_FromLong(dlhs > drhs);
}

PyObject *ExtractorBaseTypeRprice::max(PyObject *self, PyObject *args) {
  PyObject *lhs, *rhs;
  if (!PyArg_ParseTuple(args, "OO", &lhs, &rhs)) {
    return nullptr;
  }
  fmc_rprice_t dlhs;
  if (py_type_convert<fmc_rprice_t>::convert(dlhs, lhs)) {
    return nullptr;
  }
  fmc_rprice_t drhs;
  if (py_type_convert<fmc_rprice_t>::convert(drhs, rhs)) {
    return nullptr;
  }
  if (dlhs > drhs) {
    Py_INCREF(lhs);
    return lhs;
  }
  Py_INCREF(rhs);
  return rhs;
}

PyObject *ExtractorBaseTypeRprice::min(PyObject *self, PyObject *args) {
  PyObject *lhs, *rhs;
  if (!PyArg_ParseTuple(args, "OO", &lhs, &rhs)) {
    return nullptr;
  }
  fmc_rprice_t dlhs;
  if (py_type_convert<fmc_rprice_t>::convert(dlhs, lhs)) {
    return nullptr;
  }
  fmc_rprice_t drhs;
  if (py_type_convert<fmc_rprice_t>::convert(drhs, rhs)) {
    return nullptr;
  }
  if (dlhs < drhs) {
    Py_INCREF(lhs);
    return lhs;
  }
  Py_INCREF(rhs);
  return rhs;
}

PyObject *ExtractorBaseTypeRprice::from_float(PyObject *type, PyObject *fval) {
  double src = PyFloat_AsDouble(fval);
  if (PyErr_Occurred()) {
    return nullptr;
  }
  fmc_rprice_t res;
  fmc_rprice_from_double(&res, src);
  return ExtractorBaseTypeRprice::py_new(res);
}

bool Rprice_Check(PyObject *obj) {
  return PyObject_TypeCheck(obj, &ExtractorBaseTypeRpriceType);
}

fmc_rprice_t Rprice_val(PyObject *obj) {
  if (!Rprice_Check(obj)) {
    PyErr_SetString(PyExc_RuntimeError, "Object not of type Rprice");
    return {};
  }
  return ((ExtractorBaseTypeRprice *)obj)->val;
}
