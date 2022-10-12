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
 * @file comp_graph.h
 * @author Maxim Trokhimtchouk
 * @date 9 Aug 2017
 * @brief File contains C declaration of the comp graph object
 *
 * This file contains declarations of the comp graph interface
 * @see http://www.featuremine.com
 */

#ifndef __FM_COMP_GRAPH_H__
#define __FM_COMP_GRAPH_H__

#include "fmc/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief representation of the computational graph object
 *
 * The comp graph is used to describe the computation
 * dependency and store an computation generation object
 */
typedef struct fm_comp_graph fm_comp_graph_t;

/**
 * @brief representation of the compute node
 *
 * The compute node object is representation of the node on the
 * computational graph.
 */
typedef struct fm_comp_node fm_comp_node_t;

typedef struct fm_comp fm_comp_t;

FMMODFUNC void fm_comp_del(fm_comp_t *obj);

/**
 * @brief creates computational graph object
 *
 * Function creates a computational graph object and returns
 * @c fm_comp_graph_t. After use it needs to be destroyed
 * with c@ fm_comp_graph_del.
 * @return fm_comp_graph_t object.
 * @see fm_comp_graph_t
 * @see fm_comp_graph_del
 */
FMMODFUNC fm_comp_graph_t *fm_comp_graph_new();

/**
 * @brief deletes computational graph object
 *
 * Function deletes computational graph object and frees memory.
 * @param @c fm_comp_graph_t object
 * @see fm_comp_graph_t
 * @see fm_comp_graph_new
 */
FMMODFUNC void fm_comp_graph_del(fm_comp_graph_t *);

/**
 * @brief adds a node to the comp graph
 *
 * This function adds a node to the computation graph. It takes a
 * computation generation object for generating a call. It also
 * takes an array of nodes that form an input to the node
 * @param graph a computation graph to use for creating the node
 * @param obj computation generation object
 * @param ninp number of inputs into computation
 * @param inps an array of input nodes
 * @return the call node object
 * @see @c fm_type_sys_t
 * @see @c fm_type_decl_t
 * @see @c FM_BASE_TYPE
 */
FMMODFUNC fm_comp_node_t *fm_comp_graph_add(fm_comp_graph_t *graph,
                                            fm_comp_t *obj, unsigned ninp,
                                            fm_comp_node_t **inps);

/**
 * @brief associate a name with the node
 */
FMMODFUNC bool fm_comp_node_name_add(fm_comp_graph_t *g, const char *name,
                                     fm_comp_node *node);

/**
 * @brief find a node associated with a given name
 */
FMMODFUNC fm_comp_node *fm_comp_node_name_find(fm_comp_graph_t *g,
                                               const char *name);

/**
 * @brief generate a unique name for a computation
 */
FMMODFUNC char *fm_comp_node_uniq_name_gen(fm_comp_graph_t *g,
                                           const char *comp);

FMMODFUNC fm_comp_t *fm_comp_node_obj(fm_comp_node_t *node);

FMMODFUNC const fm_comp_t *fm_comp_node_const_obj(const fm_comp_node_t *node);

/**
 * @brief
 */
FMMODFUNC unsigned fm_comp_graph_nodes_size(const fm_comp_graph_t *graph);

/**
 * @brief
 */
FMMODFUNC unsigned fm_comp_node_inps_size(const fm_comp_node_t *node);

/**
 * @brief
 */
FMMODFUNC unsigned fm_comp_node_outs_size(const fm_comp_graph_t *,
                                          const fm_comp_node_t *);

typedef fm_comp_node_t **fm_comp_node_it;

FMMODFUNC unsigned fm_comp_node_idx(const fm_comp_node_t *node);

/**
 * @brief
 */
FMMODFUNC fm_comp_node_it fm_comp_graph_nodes_begin(fm_comp_graph_t *graph);

/**
 * @brief
 */
FMMODFUNC fm_comp_node_it fm_comp_graph_nodes_end(fm_comp_graph_t *graph);

/**
 * @brief
 */
FMMODFUNC fm_comp_node_it fm_comp_node_inps_begin(fm_comp_node_t *node);

/**
 * @brief
 */
FMMODFUNC fm_comp_node_it fm_comp_node_inps_end(fm_comp_node_t *node);

typedef const fm_comp_node_t *const *fm_comp_node_const_it;
/**
 * @brief
 */
FMMODFUNC fm_comp_node_const_it
fm_comp_graph_nodes_cbegin(const fm_comp_graph_t *graph);

/**
 * @brief
 */
FMMODFUNC fm_comp_node_const_it
fm_comp_graph_nodes_cend(const fm_comp_graph_t *graph);

/**
 * @brief
 */
FMMODFUNC fm_comp_node_const_it
fm_comp_node_inps_cbegin(const fm_comp_node_t *node);

/**
 * @brief
 */
FMMODFUNC fm_comp_node_const_it
fm_comp_node_inps_cend(const fm_comp_node_t *node);

typedef int fm_comp_edge_i;
typedef const fm_comp_edge_i *fm_comp_edge_const_it;

FMMODFUNC fm_comp_edge_const_it
fm_comp_node_out_cbegin(const fm_comp_node_t *n);

FMMODFUNC bool fm_comp_node_out_cend(fm_comp_edge_const_it it);

FMMODFUNC fm_comp_edge_const_it
fm_comp_node_out_cnext(const fm_comp_graph_t *g, fm_comp_edge_const_it it);

FMMODFUNC const fm_comp_node_t *
fm_comp_node_out_cnode(const fm_comp_graph_t *g, fm_comp_edge_const_it it);

/**
 * @brief stable topological sort (terminal nodes first)
 */
FMMODFUNC bool fm_comp_graph_stable_top_sort(fm_comp_graph_t *g);

/**
 * @brief returns nodes with no inputs
 */
FMMODFUNC unsigned fm_comp_graph_indep(const fm_comp_graph_t *g,
                                       const fm_comp_node_t **nodes);

/**
 * @brief stable topological sort in dependency order
 */
FMMODFUNC unsigned fm_comp_graph_dep_sort(const fm_comp_graph_t *, unsigned,
                                          const fm_comp_node_t **);

FMMODFUNC unsigned fm_comp_subgraph_stable_top_sort(fm_comp_graph_t *g,
                                                    unsigned count,
                                                    fm_comp_node_t **nodes);

FMMODFUNC unsigned fm_comp_graph_term(fm_comp_graph_t *g,
                                      fm_comp_node_t **nodes);

#ifdef __cplusplus
}
#endif

#endif // __FM_COMP_GRAPH_H__
