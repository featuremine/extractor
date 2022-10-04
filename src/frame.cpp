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
 * @file frame.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ implementation of the frame object
 *
 * @see http://www.featuremine.com
 */

extern "C" {
#include "extractor/frame.h"
#include "extractor/type_decl.h"
#include "extractor/type_sys.h"
#include "fmc/extension.h"
}

#include <iostream>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

using namespace std;

// @note
// * row or column alignment
// * contiguous memory if no slice

struct fm_frame {
  fm_frame();
  ~fm_frame();
  // @note these are dimensional offsets with the first one being the size
  vector<size_t> offsets;
  struct field {
    char *ptr = nullptr;
    size_t sz = 0;
    const char *end() const { return ptr + sz; }
  };
  vector<field> fields;
  fm_type_decl_cp type = nullptr;
};

fm_frame::fm_frame() : offsets(1, 0) {}

void fm_frame_clear(fm_frame_t *obj) {
  auto size = obj->offsets[0];
  if (size == 1) {
    free(obj->fields.front().ptr);
  } else if (size != 0) {
    for (auto &&field : obj->fields) {
      free(field.ptr);
    }
  }
  obj->offsets.resize(1);
  obj->offsets[0] = 0;
  obj->fields.resize(0);
  obj->type = nullptr;
}

fm_frame::~fm_frame() { fm_frame_clear(this); }

// @note for now frames are not contiguous
struct fm_frame_alloc {
  std::vector<fm_frame *> frames;
};

bool fm_frame_empty(const fm_frame_t *obj) { return obj->offsets[0] == 0; }

bool fm_frame_singleton(const fm_frame_t *obj) {
  return obj->offsets.size() == 2 && obj->offsets[0] == 1;
}

void fm_frame_assign0(fm_frame_t *dest, const fm_frame_t *src) {
  auto *ptr_dest = dest->fields.front().ptr;
  auto *ptr_src = src->fields.front().ptr;
  auto sz = src->fields.back().end() - ptr_src;
  memcpy(ptr_dest, ptr_src, sz);
}

void fm_frame_assign1(fm_frame_t *dest, const fm_frame_t *src) {
  auto count = src->offsets[0];
  for (unsigned i = 0; i < src->fields.size(); ++i) {
    auto &dest_field = dest->fields[i];
    auto &src_field = src->fields[i];
    memcpy(dest_field.ptr, src_field.ptr, count * src_field.sz);
  }
}

void fm_frame_assign(fm_frame_t *dest, const fm_frame_t *src) {
  if (fm_frame_singleton(src)) {
    fm_frame_assign0(dest, src);
  } else {
    fm_frame_assign1(dest, src);
  }
}

void fm_frame_proj_assign(fm_frame_t *dest, const fm_frame_t *src,
                          fm_field_t fld) {
  auto count = src->offsets[0];
  auto &dest_field = dest->fields.front();
  auto &src_field = src->fields[fld];
  memcpy(dest_field.ptr, src_field.ptr, count * src_field.sz);
}

void fm_frame_field_copy(fm_frame_t *dest, fm_field_t d_f,
                         const fm_frame_t *src, fm_field_t s_f) {
  auto count = src->offsets[0];
  auto &dest_field = dest->fields[d_f];
  auto &src_field = src->fields[s_f];
  memcpy(dest_field.ptr, src_field.ptr, count * src_field.sz);
}

void fm_frame_field_copy_from0(fm_frame_t *dest, fm_field_t d_f,
                               const fm_frame_t *src, fm_field_t s_f,
                               unsigned dim0off) {
  auto count = src->offsets[0];
  auto &dest_field = dest->fields[d_f];
  auto &src_field = src->fields[s_f];
  auto offset = src->offsets[1] * dim0off * dest_field.sz;
  memcpy(dest_field.ptr + offset, src_field.ptr, count * src_field.sz);
}

// @note assumes that number of dimensions of dest is the same as nd, but not
// equal
void fm_frame_offset_realloc(fm_frame_t *dest, const size_t offs[]) {
  auto was_singleton = fm_frame_singleton(dest);
  auto oldcount = dest->offsets[0];
  for (unsigned i = 0; i < dest->offsets.size(); ++i) {
    dest->offsets[i] = offs[i];
  }
  auto now_singleton = fm_frame_singleton(dest);
  auto count = dest->offsets[0];
  auto &d_fs = dest->fields;
  if (was_singleton && now_singleton)
    return;
  if (!was_singleton && !now_singleton) {
    for (unsigned i = 0; i < d_fs.size(); ++i) {
      auto sz = count * d_fs[i].sz;
      d_fs[i].ptr = (char *)realloc(d_fs[i].ptr, sz);
    }
    return;
  }
  if (was_singleton) {
    auto mincount = 1 < count ? 1 : count;
    auto *oldptr = d_fs[0].ptr;
    for (unsigned i = 0; i < d_fs.size(); ++i) {
      auto sz = count * d_fs[i].sz;
      auto *prv_ptr = d_fs[i].ptr;
      d_fs[i].ptr = (char *)calloc(1, sz);
      memcpy(d_fs[i].ptr, prv_ptr, mincount * d_fs[i].sz);
    }
    free(oldptr);
  } else {
    auto mincount = 1 < oldcount ? 1 : oldcount;
    size_t sz = 0;
    for (auto fld : d_fs)
      sz += fld.sz;
    char *newptr = (char *)calloc(1, sz);
    for (unsigned i = 0; i < d_fs.size(); ++i) {
      auto *prv_ptr = d_fs[i].ptr;
      d_fs[i].ptr = newptr;
      newptr += d_fs[i].sz;
      memcpy(d_fs[i].ptr, prv_ptr, mincount * d_fs[i].sz);
      free(prv_ptr);
    }
  }
}

// @note assumes the frame is empty
void fm_frame_init(fm_frame_t *dest, fm_type_decl_cp type, unsigned nd,
                   const size_t *offs, unsigned nf, const size_t *szs) {
  dest->type = type;
  auto &d_ofs = dest->offsets;
  d_ofs.resize(nd);
  for (unsigned i = 0; i < nd; ++i) {
    d_ofs[i] = offs[i];
  }
  auto &d_fs = dest->fields;
  d_fs.resize(nf);
  for (unsigned i = 0; i < nf; ++i) {
    d_fs[i].sz = szs[i];
  }
  if (fm_frame_singleton(dest)) {
    size_t sz = 0;
    for (auto fld : d_fs) {
      sz += fld.sz;
    }
    d_fs[0].ptr = (char *)calloc(1, sz);
    char *offset = d_fs[0].ptr + d_fs[0].sz;
    for (unsigned i = 1; i < d_fs.size(); ++i) {
      d_fs[i].ptr = offset;
      offset += d_fs[i].sz;
    }
  } else {
    auto count = d_ofs[0];
    for (unsigned i = 0; i < nf; ++i) {
      auto sz = count * d_fs[i].sz;
      d_fs[i].ptr = (char *)calloc(1, sz);
    }
  }
}

// @note assumes the frame is empty
void fm_frame_clone_init(fm_frame_t *dest, const fm_frame_t *src) {
  auto &s_fs = src->fields;
  auto &s_ofs = src->offsets;

  size_t n_offs = s_ofs.size();
  static thread_local vector<size_t> offs;
  offs.resize(n_offs);
  for (unsigned i = 0; i < n_offs; ++i) {
    offs[i] = s_ofs[i];
  }

  size_t n_flds = s_fs.size();
  static thread_local vector<size_t> sz;
  sz.resize(n_flds);
  for (unsigned i = 0; i < n_flds; ++i) {
    sz[i] = s_fs[i].sz;
  }

  fm_frame_init(dest, src->type, n_offs, offs.data(), n_flds, sz.data());
  fm_frame_assign(dest, src);
}

fm_frame_t *fm_frame_from_type(fm_frame_alloc_t *alloc, fm_type_decl_cp type) {
  if (!fm_type_is_frame(type))
    return nullptr;

  auto *obj = new fm_frame();
  if (!obj)
    return nullptr;

  auto n_offs = fm_type_frame_ndims(type) + 1;
  auto n_flds = fm_type_frame_nfields(type);
  static thread_local vector<size_t> offs;
  offs.resize(n_offs);
  static thread_local vector<size_t> sz;
  sz.resize(n_flds);

  offs[n_offs - 1] = 1;
  for (unsigned i = n_offs - 1; i > 0; --i) {
    offs[i - 1] = offs[i] * fm_type_frame_dim(type, i - 1);
  }

  for (unsigned i = 0; i < n_flds; ++i) {
    sz[i] = fm_type_sizeof(fm_type_frame_field_type(type, i));
  }

  fm_frame_init(obj, type, n_offs, offs.data(), n_flds, sz.data());
  alloc->frames.push_back(obj);
  return obj;
}

void fm_frame_clone_copy(fm_frame_t *dest, const fm_frame_t *src) {
  if (fm_frame_singleton(dest) && fm_frame_singleton(src)) {
    fm_frame_assign0(dest, src);
  } else if (dest->offsets == src->offsets) {
    fm_frame_assign1(dest, src);
  } else if (dest->offsets.size() == src->offsets.size()) {
    fm_frame_offset_realloc(dest, src->offsets.data());
    fm_frame_clone_copy(dest, src);
  } else {
    // @note should never happen, since type should match
    fm_frame_clear(dest);
    fm_frame_clone_init(dest, src);
  }
}

fm_frame_t *fm_frame_alloc_clone(fm_frame_alloc_t *alloc,
                                 const fm_frame_t *src) {
  auto *obj = new fm_frame();
  fm_frame_clone_init(obj, src);
  alloc->frames.push_back(obj);
  return obj;
}

fm_frame_alloc_t *fm_frame_alloc_new() { return new fm_frame_alloc(); }

void fm_frame_alloc_del(fm_frame_alloc_t *obj) {
  for (auto *frame : obj->frames) {
    delete frame;
  }
  obj->frames.resize(0);
  delete obj;
}

fm_type_decl_cp fm_frame_type(const fm_frame_t *obj) { return obj->type; }

const void *fm_frame_get_cptr1(const fm_frame_t *frame, fm_field_t idx,
                               int i0) {
  return fm_frame_get_ptr1(const_cast<fm_frame_t *>(frame), idx, i0);
}

void *fm_frame_get_ptr1(fm_frame_t *frame, fm_field_t idx, int i0) {
  auto &offs = frame->offsets;
  auto &fld = frame->fields[idx];
  return fld.ptr + fld.sz * offs[1] * i0;
}

void *fm_frame_get_ptr2(fm_frame_t *frame, fm_field_t idx, int i0, int i1) {
  auto &offs = frame->offsets;
  auto &fld = frame->fields[idx];
  return fld.ptr + fld.sz * (offs[1] * i0 + offs[2] * i1);
}

fm_field_t fm_frame_field(const fm_frame_t *frame, const char *name) {
  int i = fm_type_frame_field_idx(frame->type, name);
  return i < 0 ? -1 : i;
}

fm_type_decl_cp fm_frame_field_type(const fm_frame_t *frame, const char *name) {
  int i = fm_type_frame_field_idx(frame->type, name);
  return i < 0 ? nullptr : fm_type_frame_field_type(frame->type, i);
}

bool fm_field_valid(fm_field_t idx) { return idx >= 0; }

void fm_frame_swap(fm_frame_t *a, fm_frame_t *b) {
  auto *tmp = b->type;
  b->type = a->type;
  a->type = tmp;
  swap(a->offsets, b->offsets);
  swap(a->fields, b->fields);
}

void fm_frame_reserve(fm_frame_t *frame, ...) {
  size_t nd = fm_frame_ndims(frame);
  static thread_local vector<size_t> offs;
  offs.resize(nd + 1);
  va_list args;
  va_start(args, frame);
  offs[nd] = 1;
  for (size_t i = nd; i > 0; --i) {
    size_t dim = va_arg(args, size_t);
    offs[i - 1] = offs[i] * dim;
  }
  va_end(args);
  fm_frame_offset_realloc(frame, offs.data());
}

void fm_frame_reserve0(fm_frame_t *frame, unsigned dim) {
  size_t n_offs = frame->offsets.size();
  static thread_local vector<size_t> offs;
  offs.resize(n_offs);
  memcpy(offs.data(), frame->offsets.data(), n_offs * sizeof(size_t));
  offs[0] = offs[1] * dim;
  fm_frame_offset_realloc(frame, offs.data());
}

int fm_frame_ndims(const fm_frame_t *frame) {
  return frame->offsets.size() - 1;
}

int fm_frame_dim(const fm_frame_t *frame, int i) {
  return frame->offsets[i] / frame->offsets[i + 1];
}
