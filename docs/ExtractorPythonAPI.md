## Callbacks

  
To set up callbacks for your features, you will need to use your graph's
**callback** method.

The callbacks used for the features must take a single argument that
will be used for the feature frame.

This frame has a single **as_pandas** method that will allow you to
generate a Pandas DataFrame with the content of the frame.

<!-- -->

  
You could use it, for example, in the following way:

``` python
def clbck(frame):
    frame_data = frame.as_pandas()
    #do something

graph.callback(trade, clbck)
```

  
{\| class="wikitable" style="margin-left: 0px; margin-right: auto;"

\| **Note:**

  
If you would like to use additional data in your call as a closure, for
example the instrument or market, you will need to generate the call so
the closure is persistent.

For example:

``` python
def clbck_gen(imnt, mkt):
    return lambda frame: do_something(imnt, mkt, frame._as_pandas())

graph.callback(trade, fillmodel_trade(imnt, mkt))
```

\|}
