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

/*******************************************************************

    Some of the features we need to address:
    - Ease of prototyping
    - Availability in many languages (C++, Python, R, JavaScript, Java)
    - Write once, run everywhere
    - Fast computation with in-place (no-copy) optimization
    - Ability to handle both streaming and query interface.
      Hence different execution contexts need to be generated
      from a single "program".
    - Ability to "record" part of the computation for later "replay"
    - Ease of creating extensions, by both developer and user
    - Ease to distribute computation

********************************************************************/

csys = fm_comp_sys_new();

fm_comp_type_add(csys, constant);
fm_comp_type_add(csys, timer);
fm_comp_type_add(csys, add);

fm_type_sys_t *tsys = fm_type_sys_get(csys);
fm_comp_graph *graph = fm_comp_graph_get(csys);
fm_type_decl_cp time_type = fm_base_type_get(tsys, FM_TYPE_TIME64);
fm_comp_t *period_c = fm_comp_decl(graph, "constant", 0, time_type, 1.0);
fm_comp_t *shift1_c = fm_comp_decl(graph, "constant", 0, time_type, 0.0);
fm_comp_t *shift2_c = fm_comp_decl(graph, "constant", 0, time_type, 0.5);

fm_comp_t *timer1_op =
    fm_comp_decl(csys, graph, "timer", 2, NULL, period_c, shift1_c);
fm_comp_t *timer2_op =
    fm_comp_decl(csys, graph, "timer", 2, NULL, period_c, shift2_c);
fm_comp_t *add_op =
    fm_comp_decl(csys, graph, "add", 2, NULL, timer1_op, timer2_op);

fm_comp_clbck_set(add_op, &monitor_op, NULL);
fm_result_ref_t *result = fm_result_ref_get(add_op);
fm_frame_t *data = fm_data_get(result);

size_t size = fm_comp_graph_store(graph, fd);
fm_comp_graph *graph = fm_comp_graph_restore(csys, fd);

/* stream context */
fm_stream_ctx_t *ctx = fm_stream_ctx_get(csys, graph);

fm_ctx_proc_one(ctx);
time_t t = fm_ctx_next_time(ctx);
fm_ctx_set_now(ctx, t);

bellport.timer timer;
do {
  time_t t = fm_ctx_next_time(ctx);
  if (t != end_of_time()) {
    timer.schedule(t);
  }
  bellport.wait();
  fm_ctx_set_now(ctx, bellport.time());
  fm_ctx_proc_one(ctx);
}

/* for query context */
fm_query_ctx_t *ctx = fm_query_ctx_get(csys, graph);
fm_ctx_query(ctx, time1, time2);

fm_comp_sys_del(csystem);
