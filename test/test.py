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

from fm import *

fm = FeatureMine()
graph = fm.ComputeGraph()
delay = graph.constant(1.0)
shift1 = graph.constant(0.0)
shift2 = graph.constant(0.5)
timer1 = graph.timer(delay, shift1)
timer2 = graph.timer(delay, shift2)
op = timer1 + timer2
op.setCallback(fun)
data = op.result  # return wrapper for numpy array

context = graph.context.stream()
context.process_one()
context.next_time()

context = graph.context.query()
context.query(time1, time2)
