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
 @file /graph_dump.py
 @author Vitaut Tryputsin
 @date 16 Dec 2019
"""

from typing import Optional, Any


def graph_dump(graph: Any, file_name: Optional[str] = None) -> None:
    ''' Graph update dumps utility '''

    def write_method_gen(f: Any) -> Any:
        ''' callback generator '''
        if f:
            of = open(f, 'w')

            def write_file(name: str, frame: Any) -> None:
                of.write(name + '\n')
                of.write(str(frame) + '\n')
                of.flush()
            return write_file
        else:
            def print_frame(name: str, frame: Any) -> None:
                print(name + '\n', str(frame))
            return print_frame
    wm = write_method_gen(file_name)

    def write_method_gen_wrapper(name: str) -> Any:
        ''' writer method generator '''
        def write_file(frame: Any) -> Any:
            return wm(name, frame)
        return write_file
    for n, o in graph:
        f = write_method_gen_wrapper(n)
        graph.callback(o, f)
