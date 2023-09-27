"""
        COPYRIGHT (c) 2017 by Featuremine Corporation.
        This software has been provided pursuant to a License Agreement
        containing restrictions on its use.  This software contains
        valuable trade secrets and proprietary information of
        Featuremine Corporation and is protected by law.  It may not be
        copied or distributed in any form or medium, disclosed to third
        parties, reverse engineered or used in any manner not provided
        for in said License Agreement except with the prior written
        authorization from Featuremine Corporation.

        """
"""
 @file /graph_viz.py
 @author Andres Rangel
 @date 23 Oct 2020
"""
from graphviz import Digraph
from typing import Any, Optional


def graph_viz(graph: Any, file_name: Optional[str] = None) -> Any:
    ''' Graph visualization utility '''

    if file_name is None:
        g = Digraph('Extractor Graph')
    else:
        g = Digraph('Extractor Graph', filename=file_name)

    for name, node in graph:
        for inp in graph.inputs(node):
            g.edge(graph.name(inp), name)

    return g
