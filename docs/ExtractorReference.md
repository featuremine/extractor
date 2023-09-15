
# Computational system

  
The extractor module uses an internal computational system to set up
your graphs and allows you to set up the desired computations.

You can use the **comp_graph** method without any arguments in this
object to obtain a new graph as follows:

``` python
      graph = extr.system.comp_graph()
```

# Graph

  
The graph object will allow you to use the following methods:

## Stream context generation

  
You can generate a new stream context for your graph using the
**stream_ctx** method in the following way:

``` python
      ctx = graph.stream_ctx()
```

## Callbacks

  
To set up callbacks for your features, you will need to use your graph's
**callback** method.

The callbacks used for the features must take a single argument that
will be used for the feature frame.

For example:

``` python
def clbck(frame):
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

## Frame references

  
You can request a frame reference of any feature you have created using
the graph's **get_ref** method.

``` python
      ref = graph.get_ref(mycomp)
```

  
{\| class="wikitable" style="margin-left: 0px; margin-right: auto;"

\| **Note:**

  
The obtained frame reference cannot be used before the context is
created and the frame is initialized.

\|}

## Feature name

  
Custom names can be specified in your features, when a name is not
provided, it is generated automatically by the computational system.

You can use the graph's **name** method to obtain the name of any
feature you have created.

``` python
      name = graph.name(mycomp)
```

# Stream context

  
The stream context object is used to run your graph, depending on how
you will run your graph you may use one of the provided run methods.

Additional information, such as the current time or the next time to be
executed can also be requested from the stream context object.

## Process One

  
Runs the context a single time and advances the clock to the next time.

``` python
      ctx.run_one()
```

## Run

  
Runs the context in historical mode until all the data has been
processed.

``` python
      ctx.run()
```

## Run to

  
Runs the context in historical mode up to end_time.

``` python
      ctx.run_to(end_time)
```

## Run Live

  
Runs the context in live mode.

``` python
      ctx.run_live()
```

## Current time

  
Obtain the current time of the context.

``` python
      ctx.now()
```

## Next time

  
Obtain the next scheduled time of the context.

``` python
      ctx.now()
```

# Features

  
The Extractor features are used to build your computational graph.

You can use them to set up the computations that will generate the data
required by your alpha research or strategies.

## Built-in Operators

  
The provided built-in features will allow you to perform different
operations with your data.

## Field Names Broadcast Rules

For arithmetic, logic operators and in general for binary associative
operators specific rules for field names will apply. If a and b are two
features, a + b will be defined correctly in two cases: they either have
the same fields with the same names and types, or they both have a
single field of the same type. If they both have a single field, then
resulting frame will have a field with the name of the field of the
first operator.

## IO Features

### CSV play

  
The CSV play feature provides the user a way to read comma separated
value files and output a frame with the desired structure.

``` python
      graph.features.csv_play(file_path, column_description)
```

  
The first argument must be the path where the input file is located, the
second argument must be the description of the subset of the columns in
the csv and the first column must be of type **Time64**. The result
frame is a one dimensional frame type with simple field types. The order
of the columns in the csv is irrelevant

<!-- -->

  
For example it can be used in the following way:

``` python
      data_in = graph.features.csv_play("input_file.csv",
            (("receive", extr.Time64, ""),
             ("market", extr.Array(extr.Char, 32), ""),
             ("ticker", extr.Array(extr.Char, 16), ""),
             ("type", extr.Char, ""),
             ("bidprice", extr.Decimal64, ""),
             ("askprice", extr.Decimal64, ""),
             ("bidqty", extr.Int32, ""),
             ("askqty", extr.Int32, "")))
```

  
The CSV play feature can also be used with a piped input, for example,
if we wish to use a command that will generate a CSV encoded output we
could pipe this output into our CSV play feature like this:

``` python
      data_in = graph.features.csv_play("command args |",
            (("receive", extr.Time64, ""),
             ("market", extr.Array(extr.Char, 32), ""),
             ("ticker", extr.Array(extr.Char, 16), ""),
             ("type", extr.Char, ""),
             ("bidprice", extr.Decimal64, ""),
             ("askprice", extr.Decimal64, ""),
             ("bidqty", extr.Int32, ""),
             ("askqty", extr.Int32, "")))
```

### CSV record

  
The CSV record feature provides the user a way to record the values
generated by a given feature into a comma separated value file.

``` python
      graph.features.csv_record(op, file_path, column_names)
```

  
One input feature must be provided as first argument and the result of
the input operator must be a one dimensional frame and have simple field
types. The second argument must be the path where we will record into
comma separated values the events generated by the input feature. The
third optional argument must be a tuple with the column names that will
be outputted in the recording. The order of columns in the file will be
the same as specified, if not provided, the the columns will be in an
unspecified order

<!-- -->

  
For example it can be used like this:

``` python
      record_op = graph.features.csv_record(input_op,
                  "test_file.csv",
                  ("receive", "ticker", "market", "type",
                   "bidprice", "askprice", "bidqty", "askqty"))
```

### MP play

  
The MP play feature provides the user a way to read messagepack encoded
files and output a frame with the desired structure.

``` python
      graph.features.mp_play(file_path, column_description)
```

  
The first argument must be the path where the input file is located, the
second argument must be the description of the columns used and the
first column must be of type **Time64**. The result frame is a one
dimensional frame type with simple field types.

<!-- -->

  
For example it can be used in the following way:

``` python
      trades_in = graph.features.mp_play(
            "trade_file.mp",
            (("receive", extr.Time64, ""),
             ("ticker", extr.Array(extr.Char, 16), ""),
             ("market", extr.Array(extr.Char, 32), ""),
             ("price", extr.Decimal64, ""),
             ("qty", extr.Int32, ""),
             ("side", extr.Int32, "")))
```

  
The MP play feature can also be used with a piped input, for example, if
we wish to use a command that will generate a messagepack encoded output
we could pipe this output into our MP play feature like this:

``` python
      trades_in = graph.features.mp_play(
            "command args |",
            (("receive", extr.Time64, ""),
             ("ticker", extr.Array(extr.Char, 16), ""),
             ("market", extr.Array(extr.Char, 32), ""),
             ("price", extr.Decimal64, ""),
             ("qty", extr.Int32, ""),
             ("side", extr.Int32, "")))
```

### MP record

  
The MP record feature provides the user a way to record the values
generated by a given feature into a messagepack encoded file.

``` python
      graph.features.mp_record(op, file, frame_names)
```

  
One input feature must be provided as first argument and the result of
the input operator must be a one dimensional frame and have simple field
types. The second argument must be the path where we will record with
messagepack encoding the events generated by the input feature. The
third optional argument must be a tuple with the column names that will
be outputted in the recording. The order of columns in the file will be
the same as specified, if not provided, the the columns will be in an
unspecified order

<!-- -->

  
For example it can be used like this:

``` python
      graph.features.mp_record(quotes_in,
            "test_file.mp",
            ("receive", "ticker", "market", "bidprice",
             "askprice", "bidqty", "askqty"))
```

### Pandas Play

  
The Pandas Play feature provides the user a way to read Pandas
Dataframes and output a frame with the desired structure.

``` python
      graph.features.pandas_play(dataframe, output_frame_description)
```

  
The first argument must be the Pandas DataFrame that will be used as an
input.

The second argument must be the description of the subset of the columns
in the Dataframe.

The result frame is a one dimensional frame type with simple field
types.

### Sim Poll

  
The Sim Poll feature allows the user to use a Python Iterator returning
named tuples as input in simulations.

``` python
      graph.features.sim_poll(input_iterator, time_field_name)
```

  
The desired iterator must be provided as well as the name of the
timestamp field in the named tuple returned by the iterator.

This feature returns a named tuple. To be able to process the output of
this feature as a frame, the tuple_msg feature must be used.

### Live Poll

  
The Live Poll feature allows the user to use a Python Iterator returning
named tuples as input in live mode.

``` python
      graph.features.live_poll(input_iterator, polling_frequency)
```

  
The desired iterator must be provided as well as the polling frequency
to obtain data from the iterator.

This feature returns a named tuple. To be able to process the output of
this feature as a frame, the tuple_msg feature must be used.

### Tuple Message

  
The Tuple Message feature provides a way to process the output of
features that return a named tuple.

``` python
      graph.features.tuple_msg(input, namedtuple_class_name, fields_description_tuple)
```

  
The input feature must return a named tuple.

The second argument provided must be the name of the named tuple to
process.

As a third argument, a tuple with the description of the column names
and types of the fields in the result frame of this feature must be
provided.

### Scheduled Play

  
The Scheduled Play feature allows the user to use a Python Iterator as
input in simulations.

``` python
      graph.features.scheduled_play(input_iterator, data_frame_description, polling_frequency)
```

  
The input iterator must yield a Pandas DataFrame for each entry that
will be used by the platform.

The desired iterator must be provided as well as the frame description
of the output frame of this operator and the polling frequency as a
timedelta object.

### Immediate Play

  
The Immediate Play feature allows the user to use a Python Iterator as
input when running in live mode.

``` python
      graph.features.immediate_play(input_iterator, data_frame_description, polling_frequency)
```

  
The input iterator must yield a Pandas DataFrame for each entry that
will be used by the platform.

The desired iterator must be provided as well as the frame description
of the output frame of this operator and the polling frequency as a
timedelta object.

### YTP Sequence

  
The YTP Sequence feature allows the user to poll a given YTP sequence at
a set frequency.

``` python
      graph.features.ytp_sequence(sequence, timedelta(milliseconds=1))
```

  
The sequence will process data if an update is available, or will
schedule a new poll to take place once "timedelta" has passed.

The timedelta argument specifies the polling frequency .

### Frame YTP encode

  
The Frame YTP encode feature allows the user to convert a given frame
into a MsgPack message and write it to a provided YTP channel.

``` python
      graph.features.frame_ytp_encode(frame, channel)
```

  
The frame argument is the frame to be encoded as MsgPack.

The channel argument is the YTP Channel the resulting message should be
written to.

### Frame YTP decode

  
The Frame YTP decode feature allows the user to obtain MsgPack-decoded
messages from a given YTP channel.

``` python
      data_decoded = graph.features.frame_ytp_decode(channel)
```

  
The channel argument is the YTP Channel the encoded messages are
published to.

### ORE YTP decode

  
The ORE YTP decode feature takes an ORE message that has been published
to YTP and parses it to a timestamped object containing the message data
and the time the message was published.

``` python
       bookupd = graph.features.ore_ytp_decode(channel)
```

  
The channel argument is the YTP Channel the encoded messages are
published to.

### Decode Data

  
The Decode Data feature extracts the data field of an encoded message.

``` python
       decoded = graph.features.decode_data(message)
```

### Decode Receive

  
The Decode Receive feature extracts the time field of an encoded
message.

``` python
       decoded_time = graph.features.decode_receive(message)
```

</br>

## Frame Features

### Split

  
The split feature provides the user a way to separate the events based
on the value of a field.

``` python
      graph.features.split(op, field_name, field_values)
```

  
One feature must be provided as a first argument, a label and a n-tuple
with the desired values to split by must be provided.

<!-- -->

  
The result of this operator is a list of operators. For each of the
arguments passed in the n-tuple there will be a corresponding output in
the same order.

<!-- -->

  
For example, if we want to split the output of a feature by “market”
field values "NYSEArca" and "NASDAQOMX", the split feature should be an
input to one feature for each value. In this particular case we will
need a feature for NYSEArca and one for NASDAQOMX.

``` python
      split = graph.features.split(data_in, "market", ("NYSEArca", "NASDAQOMX"))
      NYSE_feature = split[0]
      NASDAQ_feature = split[1]
```

### Constant

  
This feature is a constant frame operation, the output given will be a
frame initialized with the description passed everytime we access it.

``` python
      graph.features.constant(frame_description)
```

  
The arguments passed must describe the columns in the frame in tuples of
three elements, they must be in the following order:

  
Name_of_the_field, Type_of_the_field, Value_of_the_field.

<!-- -->

  
The result frame will contain the described fields and values.

<!-- -->

  
For example if we would like to create a constant frame with one 64bit
float field that contains the value of Pi, we could do it like this:

``` python
      op_pi = op.constant((("pi", extr.Float64, math.pi)))
```

### Zero

  
This feature is a constant frame operation, the output given will be a
frame shaped as the input frame with zero in every field.

``` python
      graph.features.zero(op_in)
```

  
The result frame will have the same shape as op_in but all fields will
be filled with a **Float64** zero.

<!-- -->

  
For example if we would like to create a constant frame filled with
zeros with the same fields as the quote feature, we could do it using:

``` python
      op_zero = op.zero(quote)
```

### NaN

  
This feature is a constant frame operation, the output given will be a
frame shaped as the input frame with NaN in every field.

``` python
      graph.features.nan(op_in)
```

  
The result frame will have the same shape as op_in but all fields will
be filled with a **Float64** NaN.

<!-- -->

  
For example if we would like to create a constant frame filled with NaN
with the same fields as the quote feature, we could do it using:

``` python
      op_zero = op.nan(quote)
```

### Identity

  
This feature is a time series identity operation, the output given will
be the same as the input used.

``` python
      graph.features.identity(op)
```

  
One feature must be provided as a first argument

### Field

  
The field feature returns a frame with a single specified field from the
result of the input feature.

``` python
      field_op = graph.features.field(op_in, field_name)
```

  
One input feature must be provided as a first argument, the second
argument must be the field name that we would like to extract.

<!-- -->

  
The following syntax can also be used to obtain **field_op**:

``` python
      field_op = op_in.field_name
```

  
For example to obtain the "notional" field from the output of a given
operator (that has a "notional" field in its output frame) we can do the
following:

``` python
      notional = graph.features.field(op, "notional")
```

### Fields

  
The field feature returns a frame with the specified fields from the
result of the input feature.

``` python
      graph.features.fields(op, field_names_tuple)
```

  
One input feature must be provided as a first argument, the second
argument must be a tuple of the field names that we would like to
extract.

<!-- -->

  
For example to generate a frame with the "askprice" and "bidprice" from
the output of a given operator (that has the "askprice" and "bidprice"
fields in its output frame) we can do the following:

``` python
      prices = graph.features.fields(op, ("askprice","bidprice"))
```

### Combine

  
The combine feature provides the user a way to combine one or more
frames together possibly renaming their fields.

``` python
      graph.features.combine(op_one, field_map_one, op_two, field_map_two ... )
```

  
It receives two arguments must be provided for each frame:

  
op_X: The feature that will be used as Xth input.

field_map_X: The field map for the Xth input.

<!-- -->

  
The field map is a tuple of string pairs, each pair specifying the name
of the field in the input result frame and the corresponding name of the
field in the combined result frame.

<!-- -->

  
For example, We can build bars with the output of two operators that
calculate different fields of the bar frame with the combine feature in
the following way:

``` python
      data_combined = graph.features.combine(
                  data_one,
                  (("receive", "start_receive"),
                   ("bidprice", "start_bidprice"),
                   ("askprice", "start_askprice"),
                   ("bidqty", "start_bidqty"),
                   ("askqty", "start_askqty")),
                  data_two,
                  (("receive", "end_receive"),
                   ("bidprice", "end_bidprice"),
                   ("askprice", "end_askprice"),
                   ("bidqty", "end_bidqty"),
                   ("askqty", "end_askqty")))
```

  
We can also rename the fields in a given operator like this:

``` python
      data_renamed = graph.features.combine(data_in,
                (("receive", "receive"),
                 ("market","market"),
                 ("ticker", "ticker"),
                 ("type", "type"),
                 ("bidprice", "bidprice2"),
                 ("askprice", "askprice2"),
                 ("bidqty", "bidqty"),
                 ("askqty", "askqty")))
```

  
If you would like to keep all of the fields with the same name, simply
specify an empty tuple. For special case of a field with a single field
you can rename the field by specifying a tuple of a tuple with a single
element. For example the following computation will keep all of the
fields of operator a and rename the single field of operator b (error
will be thrown if b has more then a single field).

``` python
    combined = op.combine(
                a, tuple(),
                b, (("some_new_column_name",),) )
```

### Convert

  
The convert feature provides the user a way to change the type of the
fields of a given frame. (For Time64 type, use 'nano' operator to
convert from/to UInt64)

``` python
      graph.features.convert(op, op_new_type)
```

  
Two arguments must be provided, the input feature and the new type we
want this feature to have.

<!-- -->

  
For example, if we want to change the type of the "shares" field to
float in a given feature we could do it like this:

``` python
 
      shares = graph.features.convert(graph.features.field(op_one, "shares"), extr.Float64)
```

### Nano

  
The nano feature provides the user a way to convert integer nanoseconds
to time64 values.

This feature is also capable of converting from time64 to integer
nanoseconds.

``` python
      graph.features.nano(op)
```

  
Only the input feature needs to be provided as an argument.

The name of the fields in the result frame will have the same names as
the input feature

The result frame fields will be of type **Int64** if the corresponding
input fields are of type **Time64**.

The result frame fields will be of type **Time64** if the corresponding
input fields are of type **Int64**.

<!-- -->

  
For example, if we want to keep track of the timestamp of a given
feature in nanoseconds we can do it in the following way::

``` python
 
      shares = graph.features.nano(graph.features.field(op_one, "timestamp"))
```

### Join

  
The join feature provides the user a way to merge together the input
time series into a single time series.

``` python
      graph.features.join(op1, op2, ..., label_field, label_field_type, input_labels)
```

  
Two or more features must be provided, they must have the same type, The
next argument will be the field label that we will add to identify the
field that will differentiate the entries, followed by the type of this
field and the values that will be used there for each input. Only string
labels are supported at the moment.

<!-- -->

  
For example, if we want to join the output of two features that have
trades and quotes and we wish to set the market to "NASDAQ" in both
inputs, we can produce a single time series in the following way:

``` python
      trade = graph.features.join(Q_op, T_op, "market",
            extr.Array(extr.Char, 16),
            ("NASDAQ", "NASDAQ"))
```

### Elapsed

The Elapsed feature is used to obtain the elapsed time between signals
generated by a signaling input operator.

``` python
      graph.features.elapsed(data, op_signal)
```

### Unique

  
The Unique feature will allow the user to obtain a stream of
non-repeating entries from a given input feature.

``` python
      graph.features.unique(data)
```

### Accumulate

  
The Accumulate operator can be used to accumulate all the updates
generated by a given input feature.

``` python
      graph.features.accumulate(data, reset)
```

  
The accumulate operator allows you to specify an optional operator to
allow the operator to be triggered. The output frame is cleared upon
arrival of the next update.

When the optional operator is not specified, the operator will be
triggered on each update received by the data feature, the frame is
never cleared.

The output frame of this feature has the same type, number of fields and
the same field names as the input feature.

The number of dimensions of the output frame changes depending on the
number of updates generated by the input feature.

### Substring

  
The Substring operator can be used to obtain substrings given the
desired indices where the substring will be contained.

``` python
      graph.features.substr(data, startidx, endidx)
```

  
The fields in the output frame of the input feature must be character
arrays.

The output frame of this feature has the same type, number of fields and
the same field names as the input feature.

The second argument will correspond to the index where the substring
that will be copied to the output frame starts.

The third argument will correspond to the index where the substring that
will be copied to the output frame ends.

The third argument is optional, when it is not provided, the end index
will correspond to the last character in the array.

## Lag and sampling features

### Tick lag

  
The tick lag feature provides the user a way to add a tick lag to the
desired input.

``` python
      graph.features.tick_lag(op, lag)
```

  
One input feature must be provided as a first argument, the second
argument must be an integer that represents the amount of ticks we want
to use as the desired lag.

<!-- -->

  
For example if we would like to lag the output of a given feature by one
tick we can do it like this:

``` python
      nbbo_sampled_lagged = graph.features.tick_lag(nbbo_sampled, 1)
```

### Time lag

  
The time lag feature provides the user a way to add a time lag to the
desired input.

``` python
      graph.features.time_lag(op, lag_time, resolution_time)
```

  
One input feature must be provided as a first argument

The second argument must be a timedelta with the desired lag.

The third argument must be a timedelta with the desired resolution time.

Events will be dropped if they are more than **resolution_time** apart.

<!-- -->

  
For performance reasons, lag time must not be more than 1000 times
greater than the resolution time.

For example if we would like to lag the output of a given feature by one
minute with a one second resolution time we can do it like this:

``` python
      time_lag_out = graph.features.time_lag(imb_data_in, timedelta(minutes=1), timedelta(seconds=1))
```

### Timer

  
The timer C++ feature provides the user a way to create timed signals
that generate events every specified period of time

``` python
      graph.features.timer(delta)
```

  
A single timedelta argument with the desired time interval length must
be provided as a first argument.

<!-- -->

  
For example, to create an operator that will generate events every 5
minutes we can do the following:

``` python
      timer = graph.features.timer(timedelta(minutes=5))
```

  
This timer object will never stop scheduling new events.

When it is added to a graph and we run the context with **run**, the
**stream_ctx** will never stop running.

In some use cases it is required to use timers that only generate
signals in a specific time interval, when this is intended clock_timer
can be used instead.

### Activated Timer

  
The activated timer feature provides the user a way to create timed
signals that generate events every specified period of time after they
are activated.

``` python
      graph.features.activated_timer(signal_op0, ..., signal_opN, delta)
```

  
The feature is considered activated after it receives the first update
from one of the input features.

At least one input feature needs to be used to activate this feature.

A single timedelta argument with the desired time interval length must
be provided.

### Clock Timer

  
The clock_timer C++ feature allows us to create timed signals that
generate events every specified period of time in a given interval

``` python
      graph.features.clock_timer(start, stop, delta)
```

  
The first argument provided must be the time elapsed from the unix epoch
to the desired start time as a timedelta object

The second argument provided must be the time elapsed from the unix
epoch to the desired end time as a timedelta object

The third argument provided must be a timedelta object with the desired
time interval

<!-- -->

  
For example, if we would like to generate signals every 5 minutes within
market hours of a specific day we would do something like this:

``` python
      timer = graph.features.clock_timer(market_start_time_day_0, market_stop_time_day_0, timedelta(minutes=5))
```

### Sample

  
The sample feature provides the user a way to sample a given feature
when a secondary feature generates an event

``` python
      graph.features.sample(op_one,op_two)
```

  
Two features must be provided, the feature to be sampled as a first
argument and the feature that generates the events to sample the first
feature.

<!-- -->

  
For example, to create an operator that will sample op_one every 5
minutes we can do the following:

``` python
      op_sampled = graph.features.sample(op_one, graph.features.timer(timedelta(minutes=5)))
```

### Count

  
The Count feature is used to keep track of the times a given feature has
been activated.

The feature receives only one input operator.

The counter will increase each time the input operator is activated

``` python
      graph.features.count(op_a)
```

  
To keep track of the activations of op_a

<!-- -->

  
The result frame will have the following field:

  
“count” as uint64

<!-- -->

  
For example if we would like to count when we make a reading from a file
we can pass the file reading feature to the count feature like this:

``` python
      file_readings_count = graph.features.count(file_reading_op)
```

### Sample asof

  
The asof feature provides the user a way to sample a given feature when
a secondary feature generates an event

``` python
      graph.features.asof(op_one,op_two)
```

  
Two features must be provided, the feature to be sampled as a first
argument and the feature that generates the events to sample the first
feature.

<!-- -->

  
For example, to sample the values in the quote feature at the moment
close is activated we could use the following code:

``` python
    op.asof(quote, close)
```

### Sample asof previous

  
The asof_prev feature provides the user a way to obtain the values of a
given feature at the previous update of the secondary feature

``` python
      graph.features.asof_prev(op_one,op_two)
```

  
Two features must be provided, the feature to be sampled as a first
argument and the feature that generates the events to sample the first
feature.

<!-- -->

  
For example, to sample the values in the quote feature at the moment of
the previous close activation we could use the following code:

``` python
      open_quote = op.asof_prev(quote, close)
```

### Left limit

  
This feature is used to sample the last value prior to the update of the
secondary feature

For example, to calculate the value at the left limit of a given quote
sampled when a maximum value of the ask price of this quote is found in
intervals limited by the close feature

``` python
      high_quote = op.left_lim(op.asof(quote, op.max(quote_ask, close)), close)
```

### First after

  
This feature allows us to sample the value of the input at the first
update at or after the update of the second input

``` python
      graph.features.first_after(op_one,op_two)
```

  
Two features must be provided, the feature to be sampled as a first
argument and the feature that generates the events to sample the first
feature.

<!-- -->

  
For example, to sample the first trade after the close feature is
activated, we could use write the following:

``` python
      first_trade = op.first_after(trade, close)
```

### Last asof

  
This feature allows us to sample the value of the last update of the
first input prior of the update of the second input

If there are no updates, the value provided will be the value of
op_three if it is provided, otherwise it will be zero

``` python
      graph.features.last_asof(op_one,op_two,op_three)
```

  
Two features must be provided, the feature to be sampled as a first
argument and the feature that generates the events to sample the first
feature.

An optional third parameter can be provided, this feature will be used
as the default value for the sample

<!-- -->

  
For example, to sample the last value of the trade feature before the
close feature is activated

``` python
      high_trade = op.last_asof(trade, close)
```

### Time Weighted Average

  
The average_tw feature is used to generate the time weighted average of
a feature.

If the input operator result frame has more than one field the weighted
average will be calculated for each field individually.

The second input operator will act as a trigger to mark where the
interval ends.

The times during which the value is Nan will not be used in the
computation.

At the very first update the value of the weighted average is the
initial value of the first input.

``` python
      graph.features.average_tw(op_a,op_reset)
```

  
To obtain the time weighted average of op_a in the intervals signaled by
op_reset

<!-- -->

  
For example, to obtain the time weighted average of a given quote in 5
minute bars, we would use the following code:

``` python
    bar_period = timedelta(minutes=5)
    close = op.timer(bar_period)
    tw_quote = op.average_tw(quote, close)
```

  
This feature will have the result frame field types and names of the
first input operator

This feature requires the type of the fields in the result frame of the
first input operator to be of float type.

### Trigger

  
The trigger feature is used to obtain the update time of the events in
the input features.

``` python
      graph.features.trigger(op_a, ... , op_n)
```

  
At least one feature must be provided.

The result frame of this feature will have only one field named
**time**.

When any of the inputs are updated, the time of the event will be
present in the **time** field.

<!-- -->

  
For example, if we would like to know the time of the events when the
traded volume is greater than a given threshold we would do something
like this:

``` python
    higher_time = op.trigger(op.filter_if(op.greater(traded_vol,threshold)))
```

## Numerical Features

### Diff

  
The diff feature provides the user a way to subtract the values of one
frame to a different one.

If the input operator result frames have more than one field they must
have the same type. If the frames have only one field the value in the
second operators result frame field will be substracted from the first
one

``` python
      op_diff = graph.features.diff(op_a, op_b)
      op_diff = op_a - op_b
```

  
To subtract the values in op_b frame to op_a frame

<!-- -->

  
For example if we were to subtract two samples we could do it in the
following way:

``` python
      op_diff = graph.features.diff(op_one_sampled, op_two_sampled)
```

  
This feature will have the result frame field types and names of the
first input operator

This feature requires the type of the fields in the result frame of the
input operators to be of numerical type, **Decimal64** and **Time64**
types are permitted.

### Add

  
The add feature adds the values of one frame to a different one.

If the input operator result frames have more than one field they must
have the same type. If the frames have only one field the field in both
frames must be of the same type.

``` python
      op_add = graph.features.add(op_a, op_b)
      op_add = op_a + op_b
```

  
To add the values in op_b frame to op_a frame

<!-- -->

  
For example if we were to add two samples we could do it in the
following way:

``` python
      op_add = graph.features.add(op_one_sampled, op_two_sampled)
```

  
This feature will have the result frame field types and names of the
first input operator

This feature requires the type of the fields in the result frame of the
input operators to be of numerical type, **Decimal64** and **Time64**
types are permitted.

### Divide

  
The divide feature provides the user a way to divide the values of one
frame by the values in a different one.

If the input operator result frames have more than one field they must
have the same type. If the frames have only one field the field in both
frames must be of the same type.

``` python
      op_div = graph.features.divide(op_a, op_b)
      op_div = op_a / op_b
```

  
To divide the values in op_b frame to op_a frame

<!-- -->

  
For example if we were to calculate the average price of an instrument
using the notional and shares of this instrument, we could do it in the
following way:

``` python
      avg_px = graph.features.divide(notional, shares)
```

  
This feature will have the result frame field types and names of the
first input operator

This feature requires the type of the fields in the result frame of the
input operators to be of numerical type, **Decimal64** and **Time64**
types are not permitted.

### Mult

  
The Mult feature is used to multiply the values of one frame with the
values in a different one.

If the input operator result frames have more than one field they must
have the same type. If the frames have only one field the field in both
frames must be of the same type.

``` python
      op_mult = graph.features.mult(op_a, op_b)
      op_mult = op_a * op_b
```

  
To multiply the values in op_b frame to op_a frame

<!-- -->

  
For example if we were to calculate the notional of an instrument using
its average price and shares, we could do it in the following way:

``` python
      notional = graph.features.mult(avg_px, shares)
```

  
This feature will have the result frame field types and names of the
first input operator

This feature requires the type of the fields in the result frame of the
input operators to be of numerical type, **Decimal64** and **Time64**
types are not permitted.

### Log

  
The Log feature allows the user to calculate the logarithm of base 10 of
the output of a given feature

If the input operator result frame has more than one field the logarithm
computation will be applied to each field individually.

``` python
      graph.features.log(op_a)
```

  
To obtain the base 10 logarithm of the feature op_a

<!-- -->

  
This feature requires the type of the fields in the result frame of the
input operators to be of float type.

### Ln

  
The Ln feature is used to calculate the neperian logarithm of the output
of a given feature.

If the input operator result frame has more than one field the logarithm
computation will be applied to each field individually.

``` python
      graph.features.ln(op_a)
```

  
To obtain the neperian logarithm of the feature op_a

<!-- -->

  
This feature will have the result frame field types and names of the
first input operator

This feature requires the type of the fields in the result frame of the
input operators to be of float type.

### Pow

  
The Pow feature returns the base to the exponent power.

If the input operator result frame has more than one field the
exponential computation will be applied to each field individually.

``` python
      op_pow = graph.features.pow(op_a,op_exp)
      op_pow = op_a ** op_exp
```

  
To obtain the feature op_a to the power of op_exp

op_exp must have only one field

<!-- -->

  
This feature will have the result frame field types and names of the
first input operator

This feature requires the type of the fields in the result frame of the
input operators to be of float type.

### Exp

  
The Exp feature is an equivalent of using Pow with base e.

If the input operator result frame has more than one field the
exponential computation will be applied to each field individually.

``` python
      graph.features.exp(op_a)
```

  
To obtain the feature op_a to the power of e

<!-- -->

  
This feature will have the result frame field types and names of the
first input operator

This feature requires the type of the fields in the result frame of the
input operators to be of float type.

### Max

  
The Max feature is used to keep track of the maximum value of a feature.

The first input operator must have only one field.

The second input operator will reset the maximum value tracked to the
current value of the feature

``` python
      graph.features.max(op_a,op_reset)
```

  
To obtain the maximum value of op_a everytime reset is activated

<!-- -->

  
For example if we would like to keep track of the maximum value of the
op_one feature in 30 second intervals we could do it in this way:

``` python
      five_min_max = graph.features.max(op_one,graph.features.timer(timedelta(minutes=5)))
```

  
This feature will have the result frame field types and names of the
first input operator

This feature requires the type of the fields in the result frame of the
first input operator to be of numerical type, **Decimal64** and
**Time64** types are permitted.

### Min

  
The Min feature is used to keep track of the minimum value of a feature.

The first input operator must have only one field.

The second input operator will reset the minimum value tracked to the
current value of the feature

``` python
      graph.features.max(op_a,op_reset)
```

  
To obtain the minimum value of op_a everytime reset is activated

<!-- -->

  
For example if we would like to keep track of the minimum value of the
op_one feature in 30 second intervals we could do it in this way:

`     five_min_min = graph.features.min(op_one,graph.features.timer(timedelta(minutes=5)))`
```

  
This feature will have the result frame field types and names of the
first input operator

This feature requires the type of the fields in the result frame of the
first input operator to be of numerical type, **Decimal64** and
**Time64** types are permitted.

### Time weighted sum

  
This feature will allow the user to calculate the time-weighted sum of a
given input feature.

``` python
      graph.features.sum_tw(data, op_signal)
```

  
The output calculated will be the time weighted sum of the first input
operator.

The second input operator is used to reset the value of the output.

### Delta

  
The Delta feature is used to find the difference between the values of a
given input feature upon activations.

``` python
      graph.features.delta(data, op_signal)
```

  
The activations are received from the second input operator.

### Cumulative

  
This feature is used to accumulate the updates received from an input
feature.

``` python
      graph.features.cumulative(data)
```

  
Each output field will contain the sum of all the updates received in
the corresponding input field.

### Simple moving average

  
The simple moving average features calculate the simple moving average
of a given input feature for the preferred window type.

<!-- -->

  
If the user would like to have the window size limit set to a constant,
the following feature can be used:

``` python
      graph.features.sma_tick_mw(data,ticks_window)
```

  
The first argument is the input used for the calculations.

The second argument must specify the desired maximum window size.

<!-- -->

  
When a time window is used for this calculation, the following feature
can be used:

``` python
      graph.features.sma_time_mw(data,timedelta_window)
```

  
The first argument is the input used for the calculations.

The second argument must specify the desired time window length as a
timedelta object.

### Exponential moving average

  
This feature allows the user to calculate the exponential moving average
of a given input feature.

``` python
      graph.features.ema_exp(data, op_signal, timedelta_window)
```

  
The first feature is used as an input.

The second feature will generate a signal so this feature can process
the value in the input feature.

The third argument must specify the window size as a timedelta object.

### Standard deviation

  
The standard deviation features calculate the standard deviation of a
given input feature for the preferred window type.

<!-- -->

  
If the user would like to have the window size limit set to a constant,
the following feature can be used:

``` python
      graph.features.stdev_tick_mw(data,ticks_window)
```

  
The first argument is the input used for the calculations.

The second argument must specify the desired maximum window size.

<!-- -->

  
When a time window is used for this calculation, the following feature
can be used:

``` python
      graph.features.stdev_time_mw(data,timedelta_window)
```

  
The first argument is the input used for the calculations.

The second argument must specify the desired time window length as a
timedelta object.

### Exponential Standard deviation

  
This feature allows the user to calculate the exponential standard
deviation of a given input feature.

``` python
      graph.features.stdev_exp(data, op_signal, timedelta_window)
```

  
The first feature is used as an input.

The second feature will generate a signal so this feature can process
the value in the input feature.

The third argument must specify the window size as a timedelta object.

### Median

  
The median features calculate the median of a given input feature for
the preferred window type.

<!-- -->

  
If the user would like to have the window size limit set to a constant,
the following feature can be used:

``` python
      graph.features.median_tick_mw(data,ticks_window)
```

  
The first argument is the input used for the calculations.

The second argument must specify the desired maximum window size.

<!-- -->

  
When a time window is used for this calculation, the following feature
can be used:

``` python
      graph.features.median_time_mw(data,timedelta_window)
```

  
The first argument is the input used for the calculations.

The second argument must specify the desired time window length as a
timedelta object.

### Percentile

  
The percentile features calculate the specified percentiles of a given
input feature for the preferred window type.

<!-- -->

  
If the user would like to have the window size limit set to a constant,
the following feature can be used:

``` python
      graph.features.percentile_tick_mw(data,ticks_window, desired_percentile_0, ..., desired_percentile_N)
```

  
The first argument is the input used for the calculations.

The second argument must specify the desired maximum window size.

At least one percentile must be specified.

<!-- -->

  
When a time window is used for this calculation, the following feature
can be used:

``` python
      graph.features.percentile_time_mw(data,timedelta_window, desired_percentile_0, ..., desired_percentile_N)
```

  
The first argument is the input used for the calculations.

The second argument must specify the desired time window length as a
timedelta object.

At least one percentile must be specified.

### AR

  
The AR operator will allow the user to calculate a weighted average.

``` python
      graph.features.ar(data, weight0, weight1)
```

  
The first argument will be used as the input for the calculations.

The second and third arguments will be used as the weights that will
modify the current update and the last output respectively.

The weight input operators must have a single field.

### Round

  
The round feature allows the user to round an input feature to a
specified precision and convert the output to the precision-appropriate
type.

``` python
      graph.features.round(data, divisor)
```

  
  
**data**: a Decimal64 or floating-point input feature.

**divisor**: optional argument indicating desired precision: not
specifying any divisor will produce an Int64 output. Providing a
floating point input along with a divisor will result in a Decimal64
output.

### Sum

  
The Sum Feature will allow the user to generate the sum of N inputs.

``` python
      graph.features.sum(data0, ..., dataN)
```

## Logical Features

  
The result frame of the following features share some caracteristics
such as:

  
The result frame fields will be of type boolean.

The result frame of this feature will have as many fields as the input
features.

The result frame names will correspond to the names of the first input
feature.

<!-- -->

  
The type of the fields in the result frame of the input feature must be
boolean. When the frame has multiple fields, the operation is performed
field by field.

### And

  
The And feature allows the user to perform a logical and over the output
of multiple features.

``` python
      graph.features.logical_and(op_one, ..., op_n)
```

  
At least two features must be provided, depending on the number of
fields the result frame of the input features have the output result
frame could be:

  
For more than one field: The same type as the type of the result frame
of the input features

For one field: The same type as the type of the result frame of the
first input feature

All the fields in the result frame of the input features must be of
boolean type.

### Or

  
The Or feature is designed to perform a logical or operation over the
output of multiple features.

``` python
      graph.features.logical_or(op_one, ..., op_n)
```

  
At least two features must be provided, depending on the number of
fields the result frame of the input features have the output result
frame could be:

  
For more than one field: The same type as the type of the result frame
of the input features

For one field: The same type as the type of the result frame of the
first input feature

### Not

  
The Not feature was included in the platform to facilitate the users
creating operators that negate the output of other operators.

``` python
      graph.features.logical_not(op_in)
```

  
One feature must be provided.

## Conditional features

### cond

  
The Conditional feature keeps track of the changes in the first feature
to output the selected feature. The output result frame will have the
same content as the second or third input feature depending on the
changes of the first input feature.

``` python
      graph.features.cond(op_select,op_one,op_two)
```

  
Three features must be provided.

op_select must have a single boolean field frame.

op_one and op_two must have result frames of the same type.

The type of the fields in the result frames of the first input feature
must be boolean. When the frames have multiple fields, the operation is
performed field by field.

The result frame of this feature will have the same type as op_one and
op_two.

The result frame will display the contents of op_one or op_two depending
on the value of the first input frame.

### filter_if

  
The If feature checks the value in the input feature frame and is
executed when the value changes to True. When the frame has multiple
values, the field is executed when all the fields in the frame are true.

``` python
      graph.features.filter_if(op_in,op_optional)
```

  
As the first input, one feature must be provided and all the fields must
be boolean.

As the second input, is the feature to sample when the first feature
(the condition) is True.

The result frame will have the same content as the feature in the second
paramter.

### filter_unless

  
The Unless feature checks the value in the input feature frame and is
executed when the value is false. When the frame has multiple values,
the field is executed when all the fields in the frame are false

``` python
      graph.features.filter_unless(op_in, op_optional)
```

  
As the first input, one feature must be provided and all the fields must
be boolean.

As the second input, an optional feature can be provided.

The result frame will have the same content as the result frame of the
first input feature if no second input is provided.

If the optional feature is provided, the result frame will have the same
content as the optional feature.=== Comparison features ===

### Equal

  
Returns true if the values in the result frames of the input features
are equal.

``` python
      graph.features.equal(op_one, op_two)
```

  
Two features must be provided.

op_one and op_two must have result frames of the same type.

The type of the fields in the result frame of this feature will be
boolean.

The result frame of this feature will have as many fields as the input
operators.

### Not Equal

  
Returns true if the values in the result frames of the input features
are not equal.

``` python
      graph.features.not_equal(op_one, op_two)
```

  
Two features must be provided.

op_one and op_two must have result frames of the same type.

The type of the fields in the result frame of this feature will be
boolean.

The result frame of this feature will have as many fields as the input
operators.

### Greater

  
Returns true if the values in the result frames of the first input
feature are greater than the second input feature

``` python
      graph.features.greater(op_one, op_two)
```

  
Two features must be provided.

op_one and op_two must have result frames of the same type.

The type of the fields in the result frame of this feature will be
boolean.

The result frame of this feature will have as many fields as the input
operators.

### Greater or Equal

  
Returns true if the values in the result frames of the first input
feature are greater or equal than the second input feature

``` python
      graph.features.greater_equal(op_one, op_two)
```

  
Two features must be provided.

op_one and op_two must have result frames of the same type.

The type of the fields in the result frame of this feature will be
boolean.

The result frame of this feature will have as many fields as the input
operators.

### Less

  
Returns true if the values in the result frames of the first input
feature are less than the second input feature

``` python
      graph.features.less(op_one, op_two)
```

  
Two features must be provided.

op_one and op_two must have result frames of the same type.

The type of the fields in the result frame of this feature will be
boolean.

The result frame of this feature will have as many fields as the input
operators.

### Less or Equal

  
Returns true if the values in the result frames of the first input
feature are less or equal than the second input feature

``` python
      graph.features.less_equal(op_one, op_two)
```

  
Two features must be provided.

op_one and op_two must have result frames of the same type.

The type of the fields in the result frame of this feature will be
boolean.

The result frame of this feature will have as many fields as the input
operators.

### Is Zero

  
The Is Zero feature allows us to check if the values in the result frame
of a given feature are zero.

``` python
      graph.features.is_zero(op_one)
```

  
One feature must be provided.

The type of the fields in the result frame of this feature will be
boolean.

The result frame of this feature will have as many fields as the input
operator.

``` python
      op.cond(op.is_zero(op.field(quote, name)),
              op.nan(quote),
              op.convert(quote, extr.Float64))
```

### Is NaN

  
The Is NaN feature allows us to check if the values in the result frame
of a given feature are NaN.

``` python
      graph.features.is_nan(op_one)
```

  
One feature must be provided.

The type of the fields in the result frame of this feature will be
boolean.

The result frame of this feature will have as many fields as the input
operator.

<!-- -->

  
This feature requires the type of the fields in the result frame of the
input operator to be of float type.

### Is inf

  
The Is inf feature allows us to check if the values in the result frame
of a given feature are +inf or -inf.

``` python
      graph.features.is_inf(op_one)
```

  
One feature must be provided.

The type of the fields in the result frame of this feature will be
boolean.

The result frame of this feature will have as many fields as the input
operator.

<!-- -->

  
This feature requires the type of the fields in the result frame of the
input operator to be of float type.

### Find

  
This feature allows the user to search for occurrences of the provided
string in the specified string field of the input feature.

``` python
      graph.features.find(data, field_name, substr_value)
```

  
One feature must be provided.

The second argument must specify the name of the field that will be used
to search for the substring.

The third argument is the substring that will be searched for.

The result frame of this feature will have a single field.

<!-- -->

  
This feature requires the type of the specified field in the result
frame of the input operator to be of character array type.

## Trading Data Features

### BBO aggregate

  
The BBO aggregate feature provides an easy way for the users to
aggregate BBOs in the platform.

``` python
      graph.features.bbo_aggr(op_one, op_two, ...)
```

  
At least one feature must be provided as an argument

<!-- -->

  
The following fields must be part of the frames for all the input
features:

  
"bidprice" as **Decimal64**

"askprice" as **Decimal64**

"bidqty" as **Int32**

"askqty" as **Int32**

<!-- -->

  
For example if we would like to compute the nbbo out of two bbo
computations we could do it in the following way:

``` python
    nbbos = [graph.features.bbo_aggr(*x) for x in zip(bbo_one,bbo_two)]
```

### Cumulative trade

  
The cumulative trade C++ feature provides an easy way to keep track of
the shares and notional traded in a given event

``` python
      graph_features.cum_trade(op_one)
```

  
A single feature must be provided as an argument

<!-- -->

  
The following fields must be part of the frames for the input feature:

  
“price” as **Decimal64**

“qty” as **Int32**

<!-- -->

  
The result frame will have the following fields:

  
“notional” as **Decimal64**

“shares” as **Int32**

<!-- -->

  
For example if we have a feature that outputs the trades information we
can create a cum_trade feature like this:

``` python
      cum_trade = graph_features.cum_trade(trades)
```

### Cumulative trade total

  
The cumulative trade total C++ feature provides an easy way to
accumulate trading data to keep track of traded shares and notional

``` python
      graph_features.cum_trade_total(op_one, op_two, ...)
```

  
A single feature must be provided as an argument

<!-- -->

  
The following fields must be a part of the frames of the input features:

  
“notional” as **Decimal64**

“shares” as **Int32**

<!-- -->

  
For example if we have a list of features that output cum_trades
information (ctrds) we can create a cum_trade_total feature like this:

``` python
      ctrdts = [graph.features.cum_trade_total(*x) for x in zip(*ctrds)]
```

## Bookbuilding features

### Book play split

  
The Book play split feature is used to read a file with book updates and
separates the updates based on the symbol.

``` python
      upds = graph.features.book_play_split(input_file, (symbol0, ..., symbolN))
```

  
Arguments:

  
input_file : yamal

  
The Yamal file that contains order updates of different symbols.

(symbol0, ..., symbolN) : tuple

  
The symbols to be separated from the Yamal file.

<!-- -->

  
Output:

  
upds : tuple

  
The output of this feature is a list of the output stream for each
symbol.

<!-- -->

  
The script ore_yamal_play.py can be used to replay the file containing
ORE messages into a Yamal file.

The script yamal_ore_record.py can be used to check that there is data
being written to the Yamal file.

Both scripts are available at featuremine/tools/extractor/

### ORE Live Split

  
The ore_live_split feature is used to read book updates in real time
from a Yamal file and separate its data by ticker.

``` python
      upds = op.ore_live_split(input_file, (ticker0, ..., tickerN))
```

  
The Yamal input file and a tuple containing the desired tickers are
passed as arguments to the function.

<!-- -->

  
Arguments:

  
input_file : yamal

  
The Yamal file that contains order updates of different tickers.

(ticker0, ..., tickerN) : tuple

  
The tickers to be separated from the Yamal file.

<!-- -->

  
Output:

  
upds : tuple

  
A list of computations that correspond to the book for each of the
specified tickers.

The computations for each ticker are stored inside the corresponding
index of the output tuple.

In other words, the data for tickerN will be found in upds\[N\].

<!-- -->

  
The script ore_yamal_play.py can be used to replay the file containing
ORE messages into a Yamal file.

The script yamal_ore_record.py can be used to check that there is data
being written to the Yamal file.

Both scripts are available at featuremine/tools/extractor/

### Book build

  
The Book build operator builds a book from the provided updates with the
specified levels.

``` python
      graph.features.book_build(data, level_count, book_obj)
```

  
The provided input must be a feature with book updates for the desired
symbol.

The level count must be an integer.

Optionally, an Extractor Book object can be provided as an argument. If
provided, you can inspect all the levels in the book using this object.

The output frame of this field contains the prices, shares, and orders
at each level for the specified level count.

The names of the fields in the output frame will be, for each level,
bid_prx_X, bid_shr_X, bid_ord_X, ask_prx_X, ask_shr_X ask_ord_X where X
is the book level.

### Book message

  
The book message feature will allow the user to obtain the desired
update messages from an input feature.

``` python
      graph.features.book_msg(data, message_type)
```

  
As a first argument the book updates for the desired symbol must be
provided.

The second argument is a string with the name of the desired message
type.

Messages can be one of the following types:

- add - identified in ORE files as ORE 1, or ORE 6 for order
  modification involving addition of an order.

  
The output frame has the following fields:

- vendor as **Time64**
- seqn as **Uint64**
- id as **Uint64**
- price as **Decimal64**
- qty as **Uint64**
- is_bid as **Uint16**
- batch as **Uint16**

:\* insert - identified in ORE files as ORE 2.

  
  
The output frame has the following fields:

- vendor as **Time64**
- seqn as **Uint64**
- id as **Uint64**
- prio as **Uint64**
- price as **Decimal64**
- qty as **Uint64**
- is_bid as **Uint16**
- batch as **Uint16**

:\* position - identified in ORE files as ORE 3.

  
  
The output frame has the following fields:

- vendor as **Time64**
- seqn as **Uint64**
- id as **Uint64**
- price as **Decimal64**
- pos as **Uint32**
- qty as **Uint64**
- is_bid as **Uint16**
- batch as **Uint16**

:\* cancel - identified in ORE files as ORE 4, ORE 5 or ORE 6 for order
modification involving removal of an order.

  
  
The output frame has the following fields:

- vendor as **Time64**
- seqn as **Uint64**
- id as **Uint64**
- price as **Decimal64**
- qty as **Uint64**
- is_bid as **Uint16**
- batch as **Uint16**

:\* execute - identified in ORE files as ORE 7, ORE 8, ORE 9 or ORE 10.

  
  
The output frame has the following fields:

- vendor as **Time64**
- seqn as **Uint64**
- id as **Uint64**
- price as **Decimal64**
- trade_price as **Decimal64**
- qty as **Uint64**
- is_bid as **Uint16**
- batch as **Uint16**

:\* trade - identified in ORE files as ORE 11.

  
  
The output frame has the following fields:

- vendor as **Time64**
- seqn as **Uint64**
- trade_price as **Decimal64**
- qty as **Uint64**
- batch as **Uint16**
- decoration as character array of 8 characters

:\* state - identified in ORE files as ORE 12.

  
  
The output frame has the following fields:

- vendor as **Time64**
- seqn as **Uint64**
- id as **Uint64**
- price as **Decimal64**
- state as **Uint32**
- is_bid as **Uint16**
- batch as **Uint16**

:\* control - identified in ORE files as ORE 13.

  
  
The output frame has the following fields:

- vendor as **Time64**
- seqn as **Uint64**
- batch as **Uint16**
- uncross as **Uint8**
- command as character array

:\* set - identified in ORE files as ORE 14.

  
  
The output frame has the following fields:

- vendor as **Time64**
- seqn as **Uint64**
- price as **Decimal64**
- qty as **Uint64**
- is_bid as **Uint16**
- batch as **Uint16**

### Book trades

  
The book trades feature will allow the user to obtain the trade messages
from the updates generated by the input feature.

``` python
      graph.features.book_trades(data)
```

  
As a first argument the book updates for the desired symbol must be
provided.

The output frame has the following fields:

- vendor as **Time64**
- seqn as **Uint64**
- trade_price as **Decimal64**
- qty as **Uint64**
- batch as **Uint16**
- decoration as character array of 8 characters.

### Book header

  
The book header feature will allow the user to obtain the header from
the updates generated by the input feature.

``` python
      graph.features.book_header(data)
```

  
As a first argument the book updates for the desired symbol must be
provided.

The output frame has the following fields:

- receive as **Time64**
- vendor as **Time64**
- seqn as **Uint64**
- batch as **Uint16**

## Performance measurement features

### Performance timer start

  
The Performance timer start allows you to create a sample with the
specified name that will track how long does it take to execute a given
path of your graph.

Triggering this operator will start measuring the time elapsed. Use the
Performance timer stop to stop the measurement.

``` python
      graph.features.perf_timer_start(input, sample_name)
```

  
A single feature must be provided

The result frame of this feature will be an identity of the input

The output of this node should be used instead of the input to be able
to correctly measure the elapsed time.

### Performance timer stop

  
The Performance timer stop allows you to create a sample with the
specified name that will stop the measurement started by the performance
timer start node with the same sample name.

Triggering this operator will stop measuring the time elapsed. Use the
Performance timer start to start the measurement.

``` python
      graph.features.perf_timer_stop(input, sample_name)
```

  
A single feature must be provided

The result frame of this feature will be an identity of the input

The output of this node should be used instead of the input to be able
to correctly measure the elapsed time.

# Frames

  
The Extractor Frames are used to store the data returned by your
features, you can access the feature data from callbacks and also from
frame references obtained from the features directly.

## Access

  
The preferred way to access frame objects from Python is using the
provided frame API. You will be able to access the desired dimension and
field using this API.

<!-- -->

  
To obtain the desired dimension, you can use the squared brackets to
specify the desired dimension you would like to obtain from the frame in
the following way:

``` python
    frame_dim = frame_obj[dim]
```

  
**dim** must be an integer representing the desired dimension to access.

<!-- -->

  
Once the desired dimension has been obtained, the field data can be
obtained using the desired field name as an attribute of the object
retrieved when we specified the desired dimension.

For example, if we were to obtain the data on dimension zero for a field
name **price** we could do the following:

``` python
    price_data = frame_obj[0].price
```

## Pandas

  
For debugging purposes, we also included as part of the frame API a
method that will allow you to retrieve the frame data as a Pandas
DataFrame.

You can use it in the following way:

``` python
    frame_data = frame_obj.as_pandas()
```

# Trade Side

  
The Trade Side object is used to represent the Trade Side of an order
object.

The Trade Side type is available at:

extractor.**trade_side**

### Methods

#### BID

  
Trade Side BID

  
extractor.trade_side.**BID**

``` python
side = extractor.trade_side.BID()
```

  
  
This method does not receive any arguments. The Trade Side BID singleton
is returned.

#### ASK

  
Trade Side ASK

  
extractor.trade_side.**ASK**

``` python
side = extractor.trade_side.ASK()
```

  
  
This method does not receive any arguments. The Trade Side ASK singleton
is returned.

#### Other side

  
Trade Side other side

``` python
side = initial_side.other_side()
```

  
  
This method does not receive any arguments. The Trade Side singleton
corresponding to the opposite side of the initial side is returned.

# Book

  
The book_build operator supports an optional Extractor Book to be passed
as an argument.

This object will allow you to inspect the book levels and orders as
desired and can be created using:

``` python
    book = extractor.Book()
```

## Iteration

  
The book object can be iterated as follows:

``` python
    for side, levels in book:
        for price, level in levels:
            for order in level:
                pass
```

## Access

### Book

  
You can obtain the levels for the desired side of the book using:

``` python
    lvls = book[side]
```

  
  
The provided side must be a trade side object.

### Levels

  
You can obtain the level from the levels using the desired price or
index of the level using:

``` python
    lvl = lvls[px]
    lvl = lvls[idx]
```

  
  
When providing a price, the price must be a floating point object.

When porividing an index, the index must be an integer.

<!-- -->

  
The **len** method can be used to obtain how many levels can be
accessed.

### Level

  
You can obtain the desired order from a level using the index of the
order as:

``` python
    ord = lvl[idx]
```

  
  
The provided index must be an integer.

<!-- -->

  
The **len** method can be used to obtain how many orders can be
accessed.

<!-- -->

  
The available attributes in the level object are as follows:

  
px: returns the level price

shares: returns the shares count

orders: returns the orders count

### Order

  
The available attributes in the order object are as follows:

  
priority: Order priority

id: Order identifier

qty: Order quantity

received: Order received time

ven: Time difference between order received time and time reported by
broker.

seqnum: Order sequence number
