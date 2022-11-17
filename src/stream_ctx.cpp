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
 * @file stream_ctx.cpp
 * @author Maxim Trokhimtchouk
 * @date 14 Aug 2017
 * @brief File contains C++ implementation of the stream execution context
 *
 * @see http://www.featuremine.com
 */

#include "stream_ctx.h"
#include "call_obj.h"
#include "call_stack.h"
#include "comp_base.h"
#include "comp_graph.h"
#include "extractor/frame_base.h"
#include "fmc/time.h"

#include "exec_ctx.hpp"
#include <chrono>
#include <queue>
#include <signal.h>
#include <time.h>

typedef struct fm_call_timer_t {
  using item = std::pair<fmc_time64_t, fm_call_handle_t>;
  using container = std::vector<item>;
  struct compare {
    bool operator()(const item &a, const item &b) {
      return fmc_time64_greater(a.first, b.first);
    }
  };
  std::priority_queue<item, container, compare> queue;
} fm_call_timer_t;

void fm_call_timer_schedule(fm_call_timer_t *timer, fm_call_handle_t handle,
                            fmc_time64_t t) {
  timer->queue.emplace(t, handle);
}

bool fm_call_timer_scheduled(fm_call_timer_t *timer) {
  return !timer->queue.empty();
}

fmc_time64_t fm_call_timer_time(fm_call_timer_t *timer) {
  return timer->queue.empty() ? fmc_time64_end() : timer->queue.top().first;
}

bool fm_call_timer_ready(fm_call_timer_t *timer, fmc_time64_t t) {
  return !(timer->queue.empty() ||
           fmc_time64_greater(timer->queue.top().first, t));
}

fm_call_handle_t fm_call_timer_pop(fm_call_timer_t *timer) {
  auto res = timer->queue.top().second;
  timer->queue.pop();
  return res;
}

struct fm_stream_ctx : fm_exec_ctx_t {
  ~fm_stream_ctx();
  fmc_time64_t now;
  fm_call_stack_t *stack = nullptr;
  fm_call_queue_t *queue = nullptr;
  fm_call_timer_t timer;
  fm_ctx_clbck_p preproc_clbck;
  fm_ctx_cl preproc_cl;
  fm_ctx_clbck_p postproc_clbck;
  fm_ctx_cl postproc_cl;
};

struct call_details {
  fm_comp_node_t *node;
  fm_call_handle_t handle;
};

fm_stream_ctx::~fm_stream_ctx() {
  if (stack)
    fm_call_stack_del(stack);
  if (queue)
    fm_call_queue_del(queue);
}

/**
 * @brief sets up the non-optimized in-place call
 *
 * This sets up in-place call that cannot be optimized
 * by copying the first input frame to the result frame.
 */
void fm_copy_inplace_setup(fm_call_obj_t *call) {
  auto *frame = fm_call_obj_arg(call, 0);
  auto *res = fm_call_obj_result(call);
  // @note destination first, source second
  fm_frame_clone_copy(res, frame);
}

bool optimized_node(const fm_comp_graph_t *g, const fm_comp_node_t *n) {
  const auto *it = fm_comp_node_inps_cbegin(n);
  const auto *end = fm_comp_node_inps_cend(n);
  if (it == end)
    return false;
  if (fm_comp_volatile(fm_comp_node_const_obj(*it)))
    return false;
  auto *outp = fm_comp_node_out_cbegin(*it);
  if (!fm_comp_node_out_cend(outp)) {
    if (!fm_comp_node_out_cend(fm_comp_node_out_cnext(g, outp)))
      return false;
  }
  auto *inp = fm_comp_node_out_cnode(g, outp);
  auto *comp = fm_comp_node_const_obj(inp);
  return !fm_comp_data_required(comp);
}

fm_stream_ctx_t *fm_stream_ctx_new(fm_comp_graph_t *g) {
  auto *ctx = new fm_stream_ctx();
  ctx->stack = fm_call_stack_new();
  ctx->queue = fm_call_queue_new();

  auto size = fm_comp_graph_nodes_size(g);
  std::vector<call_details> details(size);
  unsigned idx = size;

  // generates the stack with the correct dependencies
  auto *it = fm_comp_graph_nodes_begin(g);
  auto *end = fm_comp_graph_nodes_end(g);
  for (; it != end; ++it) {
    auto *comp = fm_comp_node_obj(*it);
    auto argc = fm_comp_node_inps_size(*it);
    auto *call = fm_stream_call_obj_new(comp, ctx, argc);
    int depc = fm_comp_node_outs_size(g, *it);
    std::vector<fm_call_handle_t> deps(depc);
    auto *outp = fm_comp_node_out_cbegin(*it);
    for (int i = 0; i < depc; ++i) {
      auto nidx = fm_comp_node_idx(fm_comp_node_out_cnode(g, outp));
      deps[depc - i - 1] = details[size - nidx - 1].handle;
      outp = fm_comp_node_out_cnext(g, outp);
    }
    auto handle = fm_call_stack_push(ctx->stack, call, depc, deps.data());
    // @note need to delete the call object, because stack push
    // copies the object into its own memory
    fm_call_obj_del(call);
    details[--idx] = {*it, handle};
  }

  // creates the result data frames and sets the input
  // data frames for the call objects
  long i = 0;
  for (; i < (long)size; ++i) {
    auto handle = details[i].handle;
    auto *call = fm_call_stack_obj(ctx->stack, handle);
    fm_call_obj_handle_set(call, handle);
    fm_call_obj_depc_set(call, fm_call_stack_item_depc(ctx->stack, handle));
    fm_call_obj_deps_set(call, fm_call_stack_item_deps(ctx->stack, handle));

    auto *node = details[i].node;
    auto *comp = fm_comp_node_obj(node);
    auto *it = fm_comp_node_inps_cbegin(node);
    auto *end = fm_comp_node_inps_cend(node);
    int argc = 0;
    for (; it != end; ++it, ++argc) {
      auto nidx = fm_comp_node_idx(*it);
      auto handle = details[size - nidx - 1].handle;
      auto *inp = fm_call_stack_obj(ctx->stack, handle);
      fm_call_obj_dep_queuer_add(inp, call, argc);
      auto *frame = fm_call_obj_result(inp);
      fm_call_obj_arg_set(call, argc, frame);
    }
    auto inplace = fm_comp_inplace(comp);
    if (argc && inplace) {
      // if computation needs to be done in-place on the result frame
      if (optimized_node(g, node)) {
        // if the node is optimized, the result
        // is same as the first input
        auto *frame = fm_call_obj_arg(call, 0);
        fm_call_obj_result_set(call, frame);
        fm_call_obj_setup_set(call, nullptr);
      } else {
        // if not, we create a clone result frame and set a
        // call setup procedure to copy the clone
        auto *in = fm_call_obj_arg(call, 0);
        auto *frame = fm_frame_alloc_clone(ctx->frames, in);
        fm_call_obj_result_set(call, frame);
        fm_call_obj_setup_set(call, &fm_copy_inplace_setup);
      }
    } else {
      auto *frame = fm_comp_frame_mk(comp, ctx->frames);
      fm_call_obj_result_set(call, frame);
      fm_call_obj_setup_set(call, nullptr);
    }

    if (!fm_comp_call_init(comp, call)) {
      if (fm_exec_ctx_is_error(ctx)) {
        fm_exec_ctx_error_set(ctx,
                              "(stream_ctx) computation failed to "
                              "initialize;\n\t(%s) %s",
                              fm_comp_type(comp), fm_exec_ctx_error_msg(ctx));
      } else {
        fm_exec_ctx_error_set(ctx,
                              "(stream_ctx) computation failed to "
                              "initialize.\n\t(%s)",
                              fm_comp_type(comp));
      }
      goto clean_up;
    }
  }

  return ctx;

clean_up:
  // if did not initialize we need to clean up in the reverse order
  for (; i > -1; --i) {
    auto *comp = fm_comp_node_obj(details[i].node);
    fm_comp_call_destroy(comp);
  }
  return ctx;
}

void fm_stream_ctx_del(fm_stream_ctx_t *ctx) { delete ctx; }

void fm_stream_ctx_queue(fm_stream_ctx_t *ctx, fm_call_handle_t handle) {
  fm_call_queue_push(ctx->queue, handle);
}

void fm_stream_ctx_schedule(fm_stream_ctx_t *ctx, fm_call_handle_t handle,
                            fmc_time64_t time) {
  fm_call_timer_schedule(&ctx->timer, handle, time);
}

bool fm_stream_ctx_scheduled(fm_stream_ctx_t *ctx) {
  return fm_call_timer_scheduled(&ctx->timer);
}

bool fm_stream_ctx_queued(fm_stream_ctx_t *ctx) {
  return !fm_call_queue_empty(ctx->queue);
}

bool fm_stream_ctx_idle(fm_stream_ctx_t *ctx) {
  return !fm_call_timer_scheduled(&ctx->timer) &&
         fm_call_queue_empty(ctx->queue);
}

bool fm_stream_ctx_proc_one(fm_stream_ctx_t *ctx, fmc_time64_t now) {
  auto *timer = &ctx->timer;
  while (fm_call_timer_ready(timer, now)) {
    fm_call_queue_push(ctx->queue, fm_call_timer_pop(timer));
  }
  ctx->now = now;
  if (ctx->preproc_clbck)
    ctx->preproc_clbck(ctx, ctx->preproc_cl);
  if (fm_call_stack_exec(ctx->stack, ctx->queue) &&
      !fm_exec_ctx_is_error(ctx)) {
    if (ctx->postproc_clbck)
      ctx->postproc_clbck(ctx, ctx->postproc_cl);
    return true;
  }
  return false;
}

fmc_time64_t fm_stream_ctx_now(fm_stream_ctx_t *ctx) { return ctx->now; }

fmc_time64_t fm_stream_ctx_next_time(fm_stream_ctx_t *ctx) {
  return fm_call_timer_time(&ctx->timer);
}

bool fm_stream_ctx_run_to(fm_stream_ctx_t *ctx, fmc_time64_t e) {
  fmc_time64_t now = fm_stream_ctx_next_time(ctx);
  do {
    if (!fm_stream_ctx_proc_one(ctx, now) && fm_exec_ctx_is_error(ctx)) {
      return false;
    }
    now.value = std::max(now.value, fm_stream_ctx_next_time(ctx).value);
  } while (fmc_time64_less(now, e));
  return !fm_exec_ctx_is_error((fm_exec_ctx_t *)ctx);
}

bool fm_stream_ctx_run(fm_stream_ctx_t *ctx) {
  return fm_stream_ctx_run_to(ctx, fmc_time64_end());
}

fmc_time64_t fmc_time64_now() {
  using namespace std::chrono;
  timespec ts;
  clock_gettime(CLOCK_REALTIME, &ts);
  auto nanos =
      duration_cast<nanoseconds>(seconds(ts.tv_sec)) + nanoseconds(ts.tv_nsec);
  return fmc_time64_from_nanos(nanos.count());
};

static bool continue_event_loop = true;

void INThandler(int sig) {
  signal(sig, SIG_IGN);
  continue_event_loop = false;
}

bool fm_stream_ctx_run_live(fm_stream_ctx_t *ctx) {
  auto *prev_sig_handler = signal(SIGINT, INThandler);
  if (prev_sig_handler == SIG_ERR) {
    fm_exec_ctx_error_set((fm_exec_ctx_t *)ctx,
                          "Error while installing a signal handler.");
    return false;
  }
  do {
    auto now = fmc_time64_now();
    fm_stream_ctx_proc_one(ctx, now);
    if (fm_exec_ctx_is_error((fm_exec_ctx_t *)ctx)) {
      signal(SIGINT, prev_sig_handler);
      return false;
    }
  } while (continue_event_loop);
  signal(SIGINT, prev_sig_handler);
  return true;
}

void fm_stream_ctx_preproc_clbck_set(fm_stream_ctx_t *ctx, fm_ctx_clbck_p f,
                                     fm_ctx_cl cl) {
  ctx->preproc_clbck = f;
  ctx->preproc_cl = cl;
}

void fm_stream_ctx_postproc_clbck_set(fm_stream_ctx_t *ctx, fm_ctx_clbck_p f,
                                      fm_ctx_cl cl) {
  ctx->postproc_clbck = f;
  ctx->postproc_cl = cl;
}

fm_call_queue_t *fm_stream_ctx_get_queue(fm_stream_ctx_t *ctx) {
  return ctx->queue;
}
