extractor_incdir = include_directories('.')

extractor_base_include_files = [
'arg_stack.h',
'call_ctx_base.h',
'call_ctx.h',
'common.hpp',
'comp_def.h',
'comp_def.hpp',
'comp_def_simp.h',
'comp_sys_capture.h',
'comp_sys.h',
'decimal64.h',
'decimal64.hpp',
'exec_ctx.h',
'frame_base.h',
'frame.h',
'frame.hpp',
'handle.h',
'module.h',
'perf_timer.h',
'py_extractor.h',
'query_ctx.h',
'rational64.h',
'rational64.hpp',
'rprice.hpp',
'serial.h',
'side.hpp',
'std_comp.h',
'stream_ctx.h',
'time64.h',
'time64.hpp',
'type_decl.h',
'type_sys.h',
]

extractor_book_include_files = [
'book/updates.hpp',
'book/ore.hpp'
]

extractor_include_files = extractor_base_include_files + extractor_book_include_files

install_headers(extractor_base_include_files, subdir : 'extractor')
install_headers(extractor_book_include_files, subdir : 'extractor/book')
