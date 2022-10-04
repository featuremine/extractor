#include <fmc/platform.h>

#include "extractor/frame.h"
#include "extractor/serial.h"

typedef struct fm_frame_writer fm_frame_writer_t;

typedef struct fm_frame_reader fm_frame_reader_t;

#ifdef __cplusplus
extern "C" {
#endif

FMMODFUNC fm_frame_writer_t *
fm_frame_writer_new(fm_type_decl_cp type, fm_writer writer, void *closure);

FMMODFUNC bool fm_frame_writer_write(fm_frame_writer_t *w,
                                     const fm_frame *frame);

FMMODFUNC void fm_frame_writer_del(fm_frame_writer_t *w);

FMMODFUNC fm_frame_reader_t *
fm_frame_reader_new(fm_type_decl_cp type, fm_reader writer, void *closure);

FMMODFUNC bool fm_frame_reader_read(fm_frame_reader_t *w, fm_frame *frame);

FMMODFUNC void fm_frame_reader_del(fm_frame_reader_t *w);

#ifdef __cplusplus
}
#endif
