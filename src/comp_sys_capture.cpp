
extern "C" {
#include "extractor/comp_sys_capture.h"
#include "comp.h"
#include "comp_graph.h"
#include "comp_sys_serialize.h"
#include "extractor/arg_stack.h"
#include "extractor/comp_def.h"
#include "extractor/comp_sys.h"
#include "extractor/frame.h"
#include "extractor/time64.h"
#include "extractor/type_sys.h"
#include "frame_serial.h"
#include "stream_ctx.h"
#include <cmp/cmp.h>
}

#include "comp_sys.hpp"
#include "mp_util.hpp"
#include "serial_util.hpp"

#include <functional>
#include <string.h>
#include <string>
#include <unordered_map>
#include <vector>

/// this should be in serialize
bool fm_comp_graph_op_sort(fm_comp_graph_t *g) {
  auto g_size = fm_comp_graph_nodes_size(g);
  vector<fm_comp_node_t *> term_n(g_size);
  auto term_s = fm_comp_graph_term(g, term_n.data());

  sort(term_n.data(), term_n.data() + term_s, [](auto *a, auto *b) -> unsigned {
    auto *a_str = fm_comp_name(fm_comp_node_const_obj(a));
    auto *b_str = fm_comp_name(fm_comp_node_const_obj(b));
    return strcmp(a_str, b_str);
  });

  return fm_comp_subgraph_stable_top_sort(g, term_s, term_n.data()) == g_size;
}
bool fm_comp_graph_in_stream(fm_comp_graph_t *g, fm_reader reader,
                             void *closure) {
  auto string_writer = [](const void *data, size_t count, void *closure) {
    auto *str = (string *)closure;
    str->append((const char *)data, count);
    return count;
  };

  string input_g_s;
  if (!fm_comp_graph_write(g, string_writer, &input_g_s)) {
    return false;
  }

  size_t lines = std::count(input_g_s.begin(), input_g_s.end(), '\n');
  ostringstream stream_buff;
  for (size_t i = 0; i < lines; ++i) {
    string buf;
    auto arg_buf = fm_read_line(buf, reader, closure);
    stream_buff << arg_buf << endl;
  }
  if (input_g_s != stream_buff.str())
    return false;

  return true;
}
/// this should be in serialize

fm_stream_ctx_t *fm_stream_ctx_recorded(fm_comp_sys_t *s, fm_comp_graph_t *g,
                                        fm_writer writer, void *closure) {
  if (!fm_comp_graph_op_sort(g)) {
    fm_comp_sys_error_set(s, "[ERROR]\t(comp_sys) graph has circular "
                             "dependencies");
    return nullptr;
  }

  struct cmp_closure {
    fm_writer writer;
    void *closure;
  };

  auto cmp_writer = [](struct cmp_ctx_s *ctx, const void *data, size_t count) {
    auto *cl = reinterpret_cast<cmp_closure *>(ctx->buf);
    return cl->writer(data, count, cl->closure);
  };

  auto *cmp_ctx = new cmp_ctx_s();
  cmp_init(cmp_ctx, new cmp_closure{writer, closure}, NULL, NULL, cmp_writer);
  s->destructors.emplace_back([cmp_ctx]() {
    delete (cmp_closure *)cmp_ctx->buf;
    delete cmp_ctx;
  });

  auto *written = new bool();
  s->destructors.emplace_back([written]() { delete written; });

  fm_comp_graph_write(g, writer, closure);

  // Create frame and field callbacks
  auto *it = fm_comp_graph_nodes_begin(g);
  auto *end = fm_comp_graph_nodes_end(g);

  for (int index = 0; it != end; ++it) {
    auto *comp = fm_comp_node_obj(*it);
    if (!fm_comp_clbck_has(comp) && !fm_comp_data_required(comp)) {
      continue;
    }

    auto type = fm_comp_result_type(comp);
    auto *frame_writer = fm_frame_writer_new(type, writer, closure);
    s->destructors.emplace_back(
        [frame_writer]() { fm_frame_writer_del(frame_writer); });
    auto *comp_writer = new function(
        [cmp_ctx, frame_writer, index, written](const fm_frame *frame) {
          cmp_write_integer(cmp_ctx, index);
          // @note we should check if fm_frame_writer_write returns false
          fm_frame_writer_write(frame_writer, frame);
          *written = true;
        });
    s->destructors.emplace_back([comp_writer]() { delete comp_writer; });

    fm_comp_clbck_set(
        comp,
        [](const fm_frame *frame, void *cl, fm_call_ctx_t *) {
          (*(reinterpret_cast<decltype(comp_writer)>(cl)))(frame);
        },
        (void *)comp_writer);

    ++index;
  }

  // Create context
  auto *ctx = fm_stream_ctx_new(g);
  if (fm_exec_ctx_is_error((fm_exec_ctx_t *)ctx)) {
    fm_comp_sys_error_set(s,
                          "[ERROR]\t(comp_sys) failed to create "
                          "stream_ctx;\n\t%s",
                          fm_exec_ctx_error_msg((fm_exec_ctx_t *)ctx));
    fm_stream_ctx_del(ctx);
    return nullptr;
  }

  auto *posproc_clbck = new function([cmp_ctx, written](fm_stream_ctx_t *ctx) {
    // @note end_of_record index is -1
    if (!*written)
      return;
    cmp_write_integer(cmp_ctx, -1);
    auto now = fm_time64_to_nanos(fm_stream_ctx_now(ctx));
    cmp_write_integer(cmp_ctx, now);
    *written = false;
  });
  s->destructors.emplace_back([posproc_clbck]() { delete posproc_clbck; });

  fm_stream_ctx_postproc_clbck_set(
      ctx,
      [](fm_stream_ctx_t *ctx, void *cl) {
        (*(reinterpret_cast<decltype(posproc_clbck)>(cl)))(ctx);
      },
      (void *)posproc_clbck);

  s->destructors.emplace_back([ctx]() { fm_stream_ctx_del(ctx); });
  return ctx;
}

struct fm_replay_cl {
  fm_frame_reader *reader;
  fm_frame_t *next;
  fm_call_handle_t handle;
};

bool fm_comp_replay_stream_init(fm_frame_t *result, size_t args,
                                const fm_frame_t *const argv[],
                                fm_call_ctx_t *ctx, fm_call_exec_cl *cl) {
  auto *replay_cl = (fm_replay_cl *)ctx->comp;
  auto *frames = fm_exec_ctx_frames((fm_exec_ctx *)ctx->exec);
  auto type = fm_frame_type(result);
  replay_cl->next = fm_frame_from_type(frames, type);
  fm_frame_reserve(replay_cl->next, 1);
  replay_cl->handle = ctx->handle;
  return true;
}

bool fm_comp_replay_stream_exec(fm_frame_t *result, size_t,
                                const fm_frame_t *const argv[],
                                fm_call_ctx_t *ctx, fm_call_exec_cl cl) {
  auto *replay_cl = (fm_replay_cl *)ctx->comp;
  fm_frame_swap(result, replay_cl->next);
  return true;
}

fm_call_def *fm_comp_replay_stream_call(fm_comp_def_cl comp_cl,
                                        const fm_ctx_def_cl ctx_cl) {
  auto *def = fm_call_def_new();
  fm_call_def_init_set(def, fm_comp_replay_stream_init);
  fm_call_def_exec_set(def, fm_comp_replay_stream_exec);
  return def;
}

void fm_comp_replay_destroy(fm_comp_def_cl cl, fm_ctx_def_t *def) {
  auto *replay_cl = (fm_replay_cl *)fm_ctx_def_closure(def);
  if (replay_cl != nullptr) {
    fm_frame_reader_del(replay_cl->reader);
    delete replay_cl;
  }
}

const fm_comp_def_t fm_comp_replay_def = {"replay", NULL,
                                          &fm_comp_replay_destroy, NULL};

pair<fm_comp_t *, fm_replay_cl *> fm_comp_replay(fm_comp_sys_t *s,
                                                 fm_comp_graph_t *g,
                                                 fm_comp_t *comp, fm_reader r,
                                                 void *cl) {
  auto *name = fm_comp_name(comp);
  auto *type = fm_ctx_def_type(fm_comp_ctx_def(comp));

  auto *reader = fm_frame_reader_new(type, r, cl);
  if (!reader) {
    return {nullptr, nullptr};
  }

  auto *ctx_cl = new fm_replay_cl();
  ctx_cl->reader = reader;
  auto *ctx = fm_ctx_def_new();
  fm_ctx_def_inplace_set(ctx, false);
  fm_ctx_def_type_set(ctx, type);
  fm_ctx_def_closure_set(ctx, ctx_cl);
  fm_ctx_def_stream_call_set(ctx, &fm_comp_replay_stream_call);

  auto *inst = fm_comp_new(&fm_comp_replay_def, ctx, name);
  auto *node = fm_comp_graph_add(g, inst, 0, NULL);
  fm_comp_node_ptr_set(inst, node);
  fm_comp_node_name_add(g, name, node);

  for (auto i = fm_comp_clbck_begin(comp); i != fm_comp_clbck_end(comp); ++i)
    fm_comp_clbck_set(inst, (*i).clbck, (*i).cl);

  return {inst, ctx_cl};
}

fm_stream_ctx_t *fm_stream_ctx_replayed(fm_comp_sys_t *s, fm_comp_graph_t *g,
                                        fm_reader r, void *cl) {
  if (!fm_comp_graph_op_sort(g)) {
    fm_comp_sys_error_set(s, "[ERROR]\t(comp_sys) graph has circular "
                             "dependencies");
    return nullptr;
  }

  if (!fm_comp_graph_in_stream(g, r, cl)) {
    fm_comp_sys_error_set(s, "[ERROR]\t(comp_sys) provided graph not in "
                             "stream");
    return nullptr;
  }

  fm_comp_type_add(s, &fm_comp_replay_def);

  auto *replay_g = fm_comp_graph_new();

  s->graphs.push_back(replay_g);

  vector<fm_replay_cl *> closures;

  auto *it = fm_comp_graph_nodes_begin(g);
  auto *end = fm_comp_graph_nodes_end(g);

  vector<pair<fm_result_ref_t *, fm_result_ref_t *>> data_remap;

  for (; it != end; ++it) {
    auto *comp = fm_comp_node_obj(*it);

    if (!fm_comp_clbck_has(comp) && !fm_comp_data_required(comp)) {
      continue;
    }
    auto &&[replay_comp, replay_cl] = fm_comp_replay(s, replay_g, comp, r, cl);
    if (!replay_cl) {
      fm_comp_sys_error_set(s, "[ERROR]\t(comp_sys) could not create "
                               "replay operator");
      return nullptr;
    }
    if (fm_comp_data_required(comp)) {
      auto *to_ref = fm_result_ref_get(comp);
      auto *from_ref = fm_result_ref_get(replay_comp);
      data_remap.emplace_back(to_ref, from_ref);
    }
    closures.push_back(replay_cl);
  }

  // Create context
  auto *ctx = fm_stream_ctx_new(replay_g);
  if (fm_exec_ctx_is_error((fm_exec_ctx_t *)ctx)) {
    fm_comp_sys_error_set(s,
                          "[ERROR]\t(comp_sys) failed to create "
                          "stream_ctx;\n\t%s",
                          fm_exec_ctx_error_msg((fm_exec_ctx_t *)ctx));
    fm_stream_ctx_del(ctx);
    return nullptr;
  }
  s->destructors.emplace_back([ctx]() { fm_stream_ctx_del(ctx); });

  for (auto &&[to_ref, from_ref] : data_remap) {
    fm_comp_result_set(to_ref, fm_data_get(from_ref));
  }

  struct cmp_closure {
    fm_reader reader;
    void *closure;
  };

  auto cmp_reader = [](struct cmp_ctx_s *ctx, void *data, size_t count) {
    auto *cl = reinterpret_cast<cmp_closure *>(ctx->buf);
    return cl->reader(data, count, cl->closure);
  };

  auto *cmp_ctx = new cmp_ctx_s();
  cmp_init(cmp_ctx, new cmp_closure{r, cl}, cmp_reader, NULL, NULL);
  s->destructors.emplace_back([cmp_ctx]() {
    delete (cmp_closure *)cmp_ctx->buf;
    delete cmp_ctx;
  });

  auto *postproc_clbck =
      new function([cmp_ctx, closures, queue = vector<fm_replay_cl *>(),
                    last = fm_time64_t{0}](fm_stream_ctx_t *ctx) mutable {
        auto now = fm_stream_ctx_now(ctx);
        //@note this can be changed if we redefine > operator for time64
        if (fm_time64_to_nanos(last) > fm_time64_to_nanos(now))
          return;
        do {
          int64_t index = 0;
          if (!cmp_read_integer(cmp_ctx, &index))
            return;

          if (index == -1) {
            int64_t last_nano;
            if (!cmp_read_integer(cmp_ctx, &last_nano)) {
              fm_exec_ctx_error_set((fm_exec_ctx_t *)ctx,
                                    "(stream_ctx) expecting timestamp");
              return;
            }
            last = fm_time64_from_nanos(last_nano);
            for (auto &&closure : queue) {
              fm_stream_ctx_schedule(ctx, closure->handle, last);
            }
            queue.resize(0);
            break;
          }

          if (!fm_frame_reader_read(closures[index]->reader,
                                    closures[index]->next)) {
            fm_exec_ctx_error_set((fm_exec_ctx_t *)ctx, "(stream_ctx) "
                                                        "expecting frame");
            return;
          }
          queue.push_back(closures[index]);
        } while (true);
      });

  s->destructors.emplace_back([postproc_clbck]() { delete postproc_clbck; });

  (*postproc_clbck)(ctx);

  fm_stream_ctx_postproc_clbck_set(
      ctx,
      [](fm_stream_ctx_t *ctx, void *cl) {
        (*(reinterpret_cast<decltype(postproc_clbck)>(cl)))(ctx);
      },
      (void *)postproc_clbck);

  return ctx;
}
