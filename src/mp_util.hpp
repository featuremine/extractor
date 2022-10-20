
extern "C" {
#include "extractor/frame.h"
#include "extractor/type_sys.h"
#include "fmc/time.h"
}

#include "fmc++/serialization.hpp"
#include <functional>
#include <vector>

#pragma once

using fm_frame_writer_p =
    std::function<bool(cmp_ctx_t &cmp, const fm_frame_t *frame, int)>;
using fm_frame_reader_p =
    std::function<bool(cmp_ctx_t &cmp, fm_frame_t *frame, int)>;

inline bool msgpack_writer(cmp_ctx_t &cmp, int8_t val) {
  return cmp_write_integer(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, int16_t val) {
  return cmp_write_integer(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, int32_t val) {
  return cmp_write_integer(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, int64_t val) {
  return cmp_write_integer(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, uint8_t val) {
  return cmp_write_uinteger(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, uint16_t val) {
  return cmp_write_uinteger(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, uint32_t val) {
  return cmp_write_uinteger(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, uint64_t val) {
  return cmp_write_uinteger(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, float val) {
  return cmp_write_decimal(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, double val) {
  return cmp_write_decimal(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, fm_rational64_t val) {
  if (!cmp_write_array(&cmp, 2))
    return false;
  if (!cmp_write_integer(&cmp, val.num))
    return false;
  return cmp_write_integer(&cmp, val.den);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, fm_decimal64_t val) {
  return cmp_write_integer(&cmp, val.value);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, fmc_time64_t val) {
  return cmp_write_integer(&cmp, val.value);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, char val) {
  return cmp_write_integer(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, WCHAR val) {
  return cmp_write_integer(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, bool val) {
  return cmp_write_bool(&cmp, val);
}
inline bool msgpack_writer(cmp_ctx_t &cmp, fmc_decimal128_t val) {
  //TODO: implement
  return false;
}

template <class T> auto base_writer(fm_field_t offset) {
  return [offset](cmp_ctx_t &cmp, const fm_frame_t *frame, int row) {
    const T &val = *(const T *)fm_frame_get_cptr1(frame, offset, row);
    return msgpack_writer(cmp, val);
  };
}

inline fm_frame_writer_p fm_type_to_mp_writer(fm_type_decl_cp decl,
                                              fm_field_t offset) {
  if (fm_type_is_base(decl)) {
    auto em = fm_type_base_enum(decl);
    switch (em) {
    case FM_TYPE_INT8:
      return base_writer<INT8>(offset);
      break;
    case FM_TYPE_INT16:
      return base_writer<INT16>(offset);
      break;
    case FM_TYPE_INT32:
      return base_writer<INT32>(offset);
      break;
    case FM_TYPE_INT64:
      return base_writer<INT64>(offset);
      break;
    case FM_TYPE_UINT8:
      return base_writer<UINT8>(offset);
      break;
    case FM_TYPE_UINT16:
      return base_writer<UINT16>(offset);
      break;
    case FM_TYPE_UINT32:
      return base_writer<UINT32>(offset);
      break;
    case FM_TYPE_UINT64:
      return base_writer<UINT64>(offset);
      break;
    case FM_TYPE_FLOAT32:
      return base_writer<FLOAT32>(offset);
      break;
    case FM_TYPE_FLOAT64:
      return base_writer<FLOAT64>(offset);
      break;
    case FM_TYPE_RATIONAL64:
      return base_writer<RATIONAL64>(offset);
      break;
    case FM_TYPE_DECIMAL64:
      return base_writer<DECIMAL64>(offset);
      break;
    case FM_TYPE_DECIMAL128:
      return base_writer<DECIMAL128>(offset);
      break;
    case FM_TYPE_TIME64:
      return base_writer<TIME64>(offset);
      break;
    case FM_TYPE_CHAR:
      return base_writer<CHAR>(offset);
      break;
    case FM_TYPE_WCHAR:
      return base_writer<WCHAR>(offset);
      break;
    case FM_TYPE_BOOL:
      return base_writer<bool>(offset);
      break;
    case FM_TYPE_LAST:
      break;
    }
  } else if (fm_type_is_array(decl)) {
    auto type = fm_type_array_of(decl);
    auto size = fm_type_array_size(decl);
    if (fm_type_is_base(type)) {
      switch (fm_type_base_enum(type)) {
      case FM_TYPE_CHAR:
        return
            [offset, size](cmp_ctx_t &cmp, const fm_frame_t *frame, int row) {
              auto *str = (const char *)fm_frame_get_cptr1(frame, offset, row);
              return cmp_write_bin(&cmp, str, size);
            };
        break;
      default:
        break;
      }
    }
  }
  return fm_frame_writer_p();
}

inline bool msgpack_parser(cmp_ctx_t &cmp, int8_t &val) {
  return cmp_read_char(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, int16_t &val) {
  return cmp_read_short(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, int32_t &val) {
  return cmp_read_int(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, int64_t &val) {
  return cmp_read_long(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, uint8_t &val) {
  return cmp_read_uchar(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, uint16_t &val) {
  return cmp_read_ushort(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, uint32_t &val) {
  return cmp_read_uint(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, uint64_t &val) {
  return cmp_read_ulong(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, float &val) {
  return cmp_read_float(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, double &val) {
  return cmp_read_decimal(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, fm_rational64_t &val) {
  uint32_t arr_len;
  if (!cmp_read_array(&cmp, &arr_len))
    return false;
  if (arr_len != 2)
    return false;
  if (!cmp_read_int(&cmp, &val.num))
    return false;
  return cmp_read_int(&cmp, &val.den);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, fm_decimal64_t &val) {
  bool result = cmp_read_long(&cmp, &val.value);
  return result;
}
inline bool msgpack_parser(cmp_ctx_t &cmp, fmc_time64_t &val) {
  return cmp_read_long(&cmp, &val.value);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, char &val) {
  int8_t to;
  auto res = cmp_read_char(&cmp, &to);
  val = to;
  return res;
}
inline bool msgpack_parser(cmp_ctx_t &cmp, WCHAR &val) {
  int32_t to;
  auto res = cmp_read_int(&cmp, &to);
  val = to;
  return res;
}
inline bool msgpack_parser(cmp_ctx_t &cmp, bool &val) {
  return cmp_read_bool(&cmp, &val);
}
inline bool msgpack_parser(cmp_ctx_t &cmp, fmc_decimal128_t &val) {
  //TODO: Implement
  return false;
}

template <class T> auto base_reader(fm_field_t offset) {
  return [offset](cmp_ctx_t &cmp, fm_frame_t *frame, int row) -> bool {
    T &val = *(T *)fm_frame_get_cptr1(frame, offset, row);
    bool result = msgpack_parser(cmp, val);
    return result;
  };
}

inline fm_frame_reader_p fm_type_to_mp_reader(fm_type_decl_cp decl,
                                              fm_field_t offset) {
  if (fm_type_is_base(decl)) {
    auto em = fm_type_base_enum(decl);
    switch (em) {
    case FM_TYPE_INT8:
      return base_reader<INT8>(offset);
      break;
    case FM_TYPE_INT16:
      return base_reader<INT16>(offset);
      break;
    case FM_TYPE_INT32:
      return base_reader<INT32>(offset);
      break;
    case FM_TYPE_INT64:
      return base_reader<INT64>(offset);
      break;
    case FM_TYPE_UINT8:
      return base_reader<UINT8>(offset);
      break;
    case FM_TYPE_UINT16:
      return base_reader<UINT16>(offset);
      break;
    case FM_TYPE_UINT32:
      return base_reader<UINT32>(offset);
      break;
    case FM_TYPE_UINT64:
      return base_reader<UINT64>(offset);
      break;
    case FM_TYPE_FLOAT32:
      return base_reader<FLOAT32>(offset);
      break;
    case FM_TYPE_FLOAT64:
      return base_reader<FLOAT64>(offset);
      break;
    case FM_TYPE_RATIONAL64:
      return base_reader<RATIONAL64>(offset);
      break;
    case FM_TYPE_DECIMAL64:
      return base_reader<DECIMAL64>(offset);
      break;
    case FM_TYPE_DECIMAL128:
      return base_reader<DECIMAL128>(offset);
      break;
    case FM_TYPE_TIME64:
      return base_reader<TIME64>(offset);
      break;
    case FM_TYPE_CHAR:
      return base_reader<CHAR>(offset);
      break;
    case FM_TYPE_WCHAR:
      return base_reader<WCHAR>(offset);
      break;
    case FM_TYPE_BOOL:
      return base_reader<bool>(offset);
      break;
    case FM_TYPE_LAST:
      break;
    }
  } else if (fm_type_is_array(decl)) {
    auto type = fm_type_array_of(decl);
    uint32_t size = fm_type_array_size(decl);
    if (fm_type_is_base(type)) {
      switch (fm_type_base_enum(type)) {
      case FM_TYPE_CHAR:
        return
            [offset, size](cmp_ctx_t &cmp, fm_frame_t *frame, int row) -> bool {
              auto *str = (char *)fm_frame_get_cptr1(frame, offset, row);
              memset(str, '\0', sizeof(char) * size);
              return cmp_read_bin(&cmp, str, (uint32_t *)&size);
            };
        break;
      default:
        break;
      }
    }
  }
  return fm_frame_reader_p();
}

namespace fm {
namespace mp_util {
using namespace std;
constexpr uint16_t version[3] = {2, 0, 0};

inline bool write_version(cmp_ctx_t *ctx) {
  return cmp_write_uint(ctx, version[0]) && cmp_write_uint(ctx, version[1]) &&
         cmp_write_uint(ctx, version[2]);
}

// @note parser can read files with the same major version and higher minor
// version
inline bool validate_version(uint16_t ver[3]) {
  return ver[0] == version[0] && ver[1] <= version[1];
}

inline bool read_header(cmp_ctx_t *ctx, uint16_t ver[3],
                        vector<string> &column_names, string &error) {
  cmp_object_t obj;
  if (!cmp_read_object(ctx, &obj)) {
    error = "could not read the header object";
    return false;
  }
  auto error_ret = [&]() {
    error = "header format is incorrect";
    return false;
  };
  if (cmp_object_is_array(&obj)) {
    ver[0] = 1;
    ver[1] = 0;
    ver[2] = 0;
  } else if (cmp_object_is_ushort(&obj)) {
    bool done = cmp_object_as_ushort(&obj, &ver[0]) &&
                cmp_read_ushort(ctx, &ver[1]) &&
                cmp_read_ushort(ctx, &ver[2]) && cmp_read_object(ctx, &obj) &&
                cmp_object_is_array(&obj);
    if (!done)
      return error_ret();
  } else {
    return error_ret();
  }
  uint32_t size;
  if (!cmp_object_as_array(&obj, &size))
    return error_ret();

  for (unsigned i = 0; i < size; ++i) {
    column_names.push_back(string());
    if (!cmp_read_string(ctx, column_names.back())) {
      return error_ret();
    }
  }

  return true;
}

} // namespace mp_util
} // namespace fm
