
extern "C" {
#include "frame_serial.h"
#include "frame.h"
#include "serial.h"
#include "time64.h"
#include <fmc/cmp/cmp.h>
}

#include "src/mp_util.hpp"
#include <functional>
#include <vector>

typedef struct fm_frame_writer {
  std::vector<fm_frame_writer_p> writers;
  cmp_ctx_s *cmp_ctx;
} fm_frame_writer_t;

typedef struct fm_frame_reader {
  std::vector<fm_frame_reader_p> readers;
  cmp_ctx_s *cmp_ctx;
} fm_frame_reader_t;

struct cmp_writer_closure {
  fm_writer writer;
  void *closure;
};

struct cmp_reader_closure {
  fm_reader reader;
  void *closure;
};

fm_frame_writer_t *fm_frame_writer_new(fm_type_decl_cp type, fm_writer writer,
                                       void *closure) {
  auto ret = new fm_frame_writer_t();

  auto cmp_writer = [](struct cmp_ctx_s *ctx, const void *data, size_t count) {
    auto *cl = reinterpret_cast<cmp_writer_closure *>(ctx->buf);
    return cl->writer(data, count, cl->closure);
  };

  ret->cmp_ctx = new cmp_ctx_s();
  cmp_init(ret->cmp_ctx, new cmp_writer_closure{writer, closure}, NULL, NULL,
           cmp_writer);

  auto nf = fm_type_frame_nfields(type);

  for (size_t i = 0; i < nf; ++i) {
    auto ftype = fm_type_frame_field_type(type, i);
    ret->writers.push_back(fm_type_to_mp_writer(ftype, i));
  }
  return ret;
}

bool fm_frame_writer_write(fm_frame_writer_t *w, const fm_frame *frame) {
  size_t ndims = fm_frame_dim(frame, 0);
  for (size_t i = 0; i < w->writers.size(); ++i) {
    for (size_t j = 0; j < ndims; ++j) {
      if (!w->writers[i](*w->cmp_ctx, frame, j))
        return false;
    }
  }

  return true;
}

void fm_frame_writer_del(fm_frame_writer_t *w) {
  delete (cmp_writer_closure *)w->cmp_ctx->buf;
  delete w->cmp_ctx;
  delete w;
}

fm_frame_reader_t *fm_frame_reader_new(fm_type_decl_cp type, fm_reader reader,
                                       void *closure) {
  auto ret = new fm_frame_reader_t();

  auto cmp_reader = [](struct cmp_ctx_s *ctx, void *data, size_t count) {
    auto *cl = reinterpret_cast<cmp_reader_closure *>(ctx->buf);
    return cl->reader(data, count, cl->closure);
  };

  ret->cmp_ctx = new cmp_ctx_s();
  cmp_init(ret->cmp_ctx, new cmp_reader_closure{reader, closure}, cmp_reader,
           NULL, NULL);

  auto nf = fm_type_frame_nfields(type);

  for (size_t i = 0; i < nf; ++i) {
    auto ftype = fm_type_frame_field_type(type, i);
    ret->readers.push_back(fm_type_to_mp_reader(ftype, i));
  }
  return ret;
}

bool fm_frame_reader_read(fm_frame_reader_t *w, fm_frame *frame) {
  size_t ndims = fm_frame_dim(frame, 0);
  for (size_t i = 0; i < w->readers.size(); ++i) {
    for (size_t j = 0; j < ndims; ++j) {
      if (!w->readers[i](*w->cmp_ctx, frame, j))
        return false;
    }
  }

  return true;
}

void fm_frame_reader_del(fm_frame_reader_t *w) {
  delete (cmp_reader_closure *)w->cmp_ctx->buf;
  delete w->cmp_ctx;
  delete w;
}