  
Extractor is a high-performance real-time capable alpha research
platform with an easy interface for financial analytics.

It allows you to arrange your time series feeds and operations in a
computational graph that optimizes how your computations are executed.

# The Extractor Module

  
The real-time capable alpha research platform can be used as a
standalone utility for data treatment and analysis.

You only need to import the Extractor module to your Python script and
take advantage of its features.

# Building your graph

  
The operations that you will execute on your data can be specified
through the usage of nodes in a computational graph, this allows us to
execute only the necessary operations where required.

You can request from the Extractor computational system the graphs where
you will add the desired computations using the **comp_graph** method.

``` python
    graph = extractor.system.comp_graph()
```

# Running Extractor by itself

  
Depending on how you would like to run the context of your graph you may
use different methods.

For a back-testing simulation you can use the **run** or **run_to**
methods.

``` python
    graph.stream_ctx().run()
    #or
    graph.stream_ctx().run(end_time)
```

  
If your graph needs to be executed live, you can use the **run_live**
method, this method will properly synchronize the context time to your
system time while running.

``` python
    graph.stream_ctx().run_live()
```

# Inspecting the frames

  
Extractor offers multiple ways to access the data:

To obtain access to the data in computational nodes of interest when an
update is processed, you can take advantage of the [ callbacks
](ExtractorReference.md#Callbacks) mechanism.

``` python
def clbck(frame):
    #do something

graph.callback(trade, clbck)
```

  
If you need to access the data on-demand and avoid the burden of
unnecessary updates processed by the operator you can use the [ frame
references ](ExtractorReference.md#frame-references).

You can request the reference of any computation using the graph's
**get_ref** method.

``` python
      ref = graph.get_ref(mycomp)
```

  
And use the [ frame access API
](ExtractorReference.md#access) to access the data in the
frame.

For example, to access the price in the dimension zero of a
**frame_obj** object, we can do the following:

``` python
    price = frame_obj[0].price
```

# Performance measurements

  
Extractor built-in features include [ performance measurement
](ExtractorReference.md#performance-measurement-features)
features which will allow you to measure how long it takes to execute a
given sequence of nodes in your graph.

For example, to calculate how long it takes to accumulate a given input
stream, you can do the following:

``` python
    input_stream = op.perf_timer_start(input_stream, "accumulate")
    val_aggr = op.accumulate(input_stream)
    val_aggr = op.perf_timer_stop(val_aggr, "accumulate")
```

# Introspect your graph

  
The Featuremine package includes a graph dumping utility,
**graph_dump**, which will allow you to introspect the nodes in your
graphs.

This utility can be found in graph_dump.py, in the samples directory of
your Featuremine package.

The **graph_dump** method receives a graph and an optional file_name
where the graph can be dumped to.

``` python
from graph_dump import *
...
if __name__ == "__main__":
    ...
    graph_dump(graph)
    #or
    graph_dump(graph, file_path)
```

# Extractor Bars Tutorial

  
A detailed tutorial on the process of building bars using Extractor can
be found on the [Extractor Bars Tutorial](ExtractorBarsTutorial.md) page.
