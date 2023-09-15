# Table of Contents

<!--TOC-->

The Extractor typing system supports the following types:

# Python

  
The types supported by Extractor can be accessed directly from the
module using the following objects:

## Base types

  
The base types do not require a new instance to be used, the types can
be used directly.

<!-- -->

  
Int8:

  
extractor.Int8

Int16:

  
extractor.Int16

Int32:

  
extractor.Int32

Int64:

  
extractor.Int64

Uint8:

  
extractor.Uint8

Uint16:

  
extractor.Uint16

Uint32:

  
extractor.Uint32

Uint64:

  
extractor.Uint64

Float32:

  
extractor.Float32

Float64:

  
extractor.Float64

Time64:

  
extractor.Time64

Rational64:

  
extractor.Rational64

Rprice:

  
extractor.Rprice

Char64:

  
extractor.Char64

Wchar64:

  
extractor.Wchar64

Bool:

  
extractor.Bool

## Array Types

  
The array types require a new instance to be used.

<!-- -->

  
Array:

  
extractor.Array

When the array type is used, a new instance of the type must be created
using the desired type contained by the array.

For example, A type that represents a character array with a length of
30 characters can be created as follows:

` extr.Array(extr.Char, 30)`

# C Interface

  
The C interface for extractor uses the integrated typing system.

To be able to use it, you can request it from the computational system
(fm_comp_sys_t) in the following way:

` fm_type_sys_t *tsys = fm_type_sys_get(comp_sys_ptr);`

## Base types

  
The base types can be created using the **fm_base_type_get** method in
the following way:

` auto my_type = fm_base_type_get(tsys, MY_TYPE_ENUM);`

  
The following enums can be used for base type creation:

  
FM_TYPE_INT8

FM_TYPE_INT16

FM_TYPE_INT32

FM_TYPE_INT64

FM_TYPE_UINT8

FM_TYPE_UINT16

FM_TYPE_UINT32

FM_TYPE_UINT64

FM_TYPE_FLOAT32

FM_TYPE_FLOAT64

FM_TYPE_RATIONAL64

FM_TYPE_RPRICE

FM_TYPE_TIME64

FM_TYPE_CHAR

FM_TYPE_WCHAR

FM_TYPE_BOOL

## Array types

  
The array types can be created using the **fm_array_type_get** method in
the following way:

` auto *my_array = fm_array_type_get(tsys, my_base_type, array_length);`

  
For example, if we would like to create a character array of 30
characters, we could do it in the following way:

` auto *char_type = fm_base_type_get(tsys, FM_TYPE_CHAR);`  
` auto *chararray16 = fm_array_type_get(tsys, char_type, 30);`

## Method declarations

  
The header where the definitions of these methods and additional details
and utilities to utilize types can be found in the
**include/extractor/type_sys.h** header.
