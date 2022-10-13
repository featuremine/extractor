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


def graph_dump(graph, file_name=None):

    def write_method_gen(f):
        if f:
            of = open(f, 'w')

            def write_file(name, frame):
                of.write(name + '\n')
                of.write(str(frame.as_pandas()) + '\n')
                of.flush()
            return write_file
        else:
            def print_frame(name, frame):
                print(name + '\n', str(frame.as_pandas()))
            return print_frame
    wm = write_method_gen(file_name)

    def write_method_gen_wrapper(name):
        def write_file(frame):
            return wm(name, frame)
        return write_file
    for n, o in graph:
        f = write_method_gen_wrapper(n)
        graph.callback(o, f)
