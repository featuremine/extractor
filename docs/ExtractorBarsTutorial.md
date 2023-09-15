# Table of Contents

<!--TOC-->

This tutorial will walk us through the process of building bars using
Extractor.

We will start by creating an Extractor graph that will contain all the
computations needed to read the market data and then we will generate
bars with it.

Finally We will export our newly generated bars from Extractor to a
Pandas DataFrame using the builtin exporting tool.

# Get market data for individual instruments

  
Before we use Extractor we need to initialize the platform.

In this section we will learn how to do it and how to add the required
features to read the market data and shape it in a way we can compute
bars.

## Initializing the platform

  
Before we are able to add any features, we need to setup our
environment.

We can do it importing the Extractor Python package and setting
up a graph where we will add the features that
will compute our bars.

``` python
import extractor as extr

if __name__ == "__main__":
        graph = extr.system.comp_graph()
```

  
Now we will need to obtain a graph where we will add the features that
will compute our bars.

``` python
        graph = extr.system.comp_graph()
```

  
For conveniency we will use the following variable to easily create our
features from this point onwards:

``` python
        op = graph.features
```

## Loading the market data

  
Now, we will proceed to add the first features to our graph that will
load the market data into the platform.

The Jubilee-Tutorial package has two files that we can use as our market
data input:

``` python
        bbo_file =  "../test/sip_quotes_20171018.base.mp"
        trade_file = "../test/sip_trades_20171018.base.mp"
```

  
Since the files used are MessagePack encoded files, we will use the
**mp_play** feature to read them.

``` python
        bbos_in = op.mp_play(
              bbo_file,
              (("receive", extr.Time64, ""),
               ("ticker", extr.Array(extr.Char, 16), ""),
               ("market", extr.Array(extr.Char, 32), ""),
               ("bidprice", extr.Decimal64, ""),
               ("askprice", extr.Decimal64, ""),
               ("bidqty", extr.Int32, ""),
               ("askqty", extr.Int32, "")))
        trades_in = op.mp_play(
              trade_file,
              (("receive", extr.Time64, ""),
               ("ticker", extr.Array(extr.Char, 16), ""),
               ("market", extr.Array(extr.Char, 32), ""),
               ("price", extr.Decimal64, ""),
               ("qty", extr.Int32, ""),
               ("side", extr.Int32, "")))
```

## Preparing data for calculations

  
To generate bars, we need to setup the markets and tickers we would like
to generate bars for, in this tutorial we would like to generate bars
for instruments that are listed on three markets described as follows:

``` python
        markets = ["NYSEMKT", "NASDAQOMX", "NYSEArca"]
        tickers = [
          {"NYSEMKT": "A", "NASDAQOMX": "A", "NYSEArca": "A"},
          {"NYSEMKT": "AA", "NASDAQOMX": "AA", "NYSEArca": "AA"},
          {"NYSEMKT": "BA", "NASDAQOMX": "BA", "NYSEArca": "BA"}
        ]
```

  
Now We will use the lists we just added to split the market data into
streams for each the market like this:

``` python
        bbo_split = op.split(bbos_in, "market", tuple(markets))
        trade_split = op.split(trades_in, "market", tuple(markets))
```

  
Now that we have the desired market data streams we can proceed to
generate streams by instrument for each one of them.

``` python
        mkt_idx = 0;
        for mkt in markets:
            mkt_tickers = [x[mkt] for x in tickers]
            mkt_bbo_split = op.split(bbo_split[mkt_idx], "ticker", tuple(mkt_tickers))
            mkt_trade_split = op.split(trade_split[mkt_idx], "ticker", tuple(mkt_tickers))
            ticker_idx = 0
            for _ in tickers:
                bbo = mkt_bbo_split[ticker_idx]
                trade = mkt_trade_split[ticker_idx]
                ticker_idx = ticker_idx + 1
            mkt_idx = mkt_idx + 1
```

  
To generate bars we will use as input the **cumulative trade** for each
instrument, so we will write in the **for** loops we just added the
necessary features to obtain cumulative trades for each market and
instrument:

``` python
        mkt_idx = 0;
        for mkt in markets:
            mkt_tickers = [x[mkt] for x in tickers]
            mkt_bbo_split = op.split(bbo_split[mkt_idx], "ticker", tuple(mkt_tickers))
            mkt_trade_split = op.split(trade_split[mkt_idx], "ticker", tuple(mkt_tickers))
            ticker_idx = 0
            for _ in tickers:
                bbo = mkt_bbo_split[ticker_idx]
                trade = mkt_trade_split[ticker_idx]
                cum_trade = op.cum_trade(trade)
                ticker_idx = ticker_idx + 1
            mkt_idx = mkt_idx + 1
```

  
Now we will place our bbos and cumulative trades in a couple of lists
that will allow us to use them as input of the features that will help
us calculate NBBOs and Cumulative trades:

``` python
        bbos = []
        ctrds = []
        mkt_idx = 0;
        for mkt in markets:
            mkt_tickers = [x[mkt] for x in tickers]
            mkt_bbo_split = op.split(bbo_split[mkt_idx], "ticker", tuple(mkt_tickers))
            mkt_trade_split = op.split(trade_split[mkt_idx], "ticker", tuple(mkt_tickers))
            mkt_bbos = []
            mkt_ctrds = []
            ticker_idx = 0
            for _ in tickers:
                bbo = mkt_bbo_split[ticker_idx]
                trade = mkt_trade_split[ticker_idx]
                cum_trade = op.cum_trade(trade)
                mkt_bbos.append(bbo)
                mkt_ctrds.append(cum_trade)
                ticker_idx = ticker_idx + 1
            bbos.append(mkt_bbos)
            ctrds.append(mkt_ctrds)
            mkt_idx = mkt_idx + 1
```

  
To calculate the NBBOs we will use the BBOs for each market and
instrument as input to the **bbo_aggr** feature.

``` python
        nbbos = [op.bbo_aggr(*x) for x in zip(*bbos)]
```

  
Similarly to what we did with NBBOs we will generate the Cumulative
Trade totals using the **cum_trade_total** feature.

``` python
        ctrdts = [op.cum_trade_total(*x) for x in zip(*ctrds)]
```

  
To add additional information related to the trades to our bars we will
generate streams of trading data for each instrument like this:

``` python
        trade_imnt_split = op.split(trades_in, "ticker", tuple([x["NASDAQOMX"] for x in tickers]))
```

  
At this point we have the NBBO, Cumulative trade data and trading data
streams for each instrument and our code should look close to this:

``` python
import extractor as extr

if __name__ == "__main__":
        graph = extr.system.comp_graph()
        op = graph.features
        bbo_file =  "../test/sip_quotes_20171018.base.mp"
        trade_file = "../test/sip_trades_20171018.base.mp"
        markets = ["NYSEMKT", "NASDAQOMX", "NYSEArca"]
        tickers = [
          {"NYSEMKT": "A", "NASDAQOMX": "A", "NYSEArca": "A"},
          {"NYSEMKT": "AA", "NASDAQOMX": "AA", "NYSEArca": "AA"},
          {"NYSEMKT": "BA", "NASDAQOMX": "BA", "NYSEArca": "BA"}
        ]
        bbos_in = op.mp_play(
              bbo_file,
              (("receive", extr.Time64, ""),
               ("ticker", extr.Array(extr.Char, 16), ""),
               ("market", extr.Array(extr.Char, 32), ""),
               ("bidprice", extr.Decimal64, ""),
               ("askprice", extr.Decimal64, ""),
               ("bidqty", extr.Int32, ""),
               ("askqty", extr.Int32, "")))
        trades_in = op.mp_play(
              trade_file,
              (("receive", extr.Time64, ""),
               ("ticker", extr.Array(extr.Char, 16), ""),
               ("market", extr.Array(extr.Char, 32), ""),
               ("price", extr.Decimal64, ""),
               ("qty", extr.Int32, ""),
               ("side", extr.Int32, "")))
        bbo_split = op.split(bbos_in, "market", tuple(markets))
        trade_split = op.split(trades_in, "market", tuple(markets))
        bbos = []
        ctrds = []
        mkt_idx = 0;
        for mkt in markets:
            mkt_tickers = [x[mkt] for x in tickers]
            mkt_bbo_split = op.split(bbo_split[mkt_idx], "ticker", tuple(mkt_tickers))
            mkt_trade_split = op.split(trade_split[mkt_idx], "ticker", tuple(mkt_tickers))
            mkt_bbos = []
            mkt_ctrds = []
            ticker_idx = 0
            for _ in tickers:
                bbo = mkt_bbo_split[ticker_idx]
                trade = mkt_trade_split[ticker_idx]
                cum_trade = op.cum_trade(trade)
                mkt_bbos.append(bbo)
                mkt_ctrds.append(cum_trade)
                ticker_idx = ticker_idx + 1
            bbos.append(mkt_bbos)
            ctrds.append(mkt_ctrds)
            mkt_idx = mkt_idx + 1
        nbbos = [op.bbo_aggr(*x) for x in zip(*bbos)]
        ctrdts = [op.cum_trade_total(*x) for x in zip(*ctrds)]
        trade_imnt_split = op.split(trades_in, "ticker", tuple([x["NASDAQOMX"] for x in tickers]))
```

# Generating the bars

  
Now that our data is shaped correctly we will define a new method that
will set up the features required to build the bars, it will receive the
NBBO, cumulative trade and trading features for an instrument, and
return a feature that will have in its result frame all the fields we
would like to add with the desired labels.

The fields we would like to have in our bars are:

- NBBO at bar open and close time.
- Highest and Lowest NBBOs in bar interval.
- Time weighted average of the NBBOs during the bar.
- Trade info at highest and lowest traded prices during the bar.
- First and last trades in the bar.
- Shares traded during the bar.

## Setting up a timer

``` python
    def compute_bar(nbbo, trades, ctrdt):
```

  
To generate the event that will signal the end of the bars every five
minutes, we will create a timer with the desired five minute bar period.

Since the length of the bar period is a **timedelta** object, we will
need an aditional dependency in our script:

``` python
from datetime import timedelta
```

  
Now we will create the timer in the following way:

``` python
    bar_period = timedelta(minutes=5)
    close = op.timer(bar_period)
```

## Computing NBBO data

  
Only some fields of interest are used when generating the bars, so we
will filter them from the input before we start generating our bars:

``` python
    quote = op.fields(nbbo, ("bidprice", "askprice", "bidqty", "askqty"))
    quote_bid = op.field(nbbo, "bidprice")
    quote_ask = op.field(nbbo, "askprice")
```

  
Now that the market data contains the correct fields, we would like to
obtain the open and close values of the quotes, we can do it using the
following code:

``` python
    open_quote = op.asof_prev(quote, close)
```

  
This will allow us to sample the value of the quote at the moment of the
previous close activation for the open value of the quote.

``` python
    close_quote = op.left_lim(quote, close)
```

  
And this will allow us to sample the last value before close is
activated for the close value of the quote

<!-- -->

  
To sample the quote when the ask price has reached its maximum value on
a given interval, we will need to sample the last maximum value of the
quote before the close feature is activated, this can be achieved in the
following way:

``` python
    high_quote = op.left_lim(op.asof(quote, op.max(quote_ask, close)), close)
```

  
This allows us to sample the quote when the maximum value of the bid
price activates the **asof** sample feature; the **left_lim** feature is
used to obtain the value in a \[closed,open) interval.

<!-- -->

  
We would also like to sample the quote when the bid price has reached
its minimum value on a given interval, this can be achieved with this
code:

``` python
    low_quote = op.left_lim(op.asof(quote, op.min(quote_bid, close)), close)
```

  
As we can see, in a similar way to the high quote calculation, we will
sample the quote when the minimum value of the bid price activates the
**asof** sample feature.

<!-- -->

  
The following method is defined to filter the entries where the field
specified by name is zero using the **cond** feature.

It returns as output a frame with the same shape as the quote, with
**Float64** NaN in every field when the condition is met.

Since the **average_tw** feature only accepts **Float32** and
**Float64** field types as input we apply the convert operator to the
quote to return a frame with **Float64** fields

``` python
def quote_side_float64(quote, name):
    return op.cond(op.is_zero(op.field(quote, name)),
                   op.nan(quote),
                   op.convert(quote, extr.Float64))
```

  
We will need a method that uses the method we just declared to process
the time weighted average input.

It will fill with NaN where required the entries where no trades have
been executed so this intervals are not involved in our calculations.

``` python
def quote_float64(quote):
    bid_quote = op.fields(quote, ("bidprice", "bidqty"))
    ask_quote = op.fields(quote, ("askprice", "askqty"))
    return op.combine(quote_side_float64(bid_quote, "bidqty"), tuple(),
                      quote_side_float64(ask_quote, "askqty"), tuple())
```

  
Now we would like to calculate the time weighted average of the quote,
to do that we will add the following code:

``` python
    tw_quote = op.average_tw(quote_float64(quote), close)
```

  
Since the NaN entries are not considered in the **average_tw** feature
we will be able to accurately calculate the time weighted average of the
quote.

## Computing trading data

  
We would also like to add some trade related information to our bars, to
generate it, we will filter the desired fields from the trade input
feature in the following way:

``` python
    trade = op.fields(trades, ("price", "qty", "market"))
    trade_px = op.field(trade, "price")
```

  
Now we would like to sample the first trade after the close feature is
activated

``` python
    first_trade = op.first_after(trade, close)
```

  
Similarly to quotes we would like to sample the open and close values:

``` python
    open_trade = op.last_asof(first_trade, close)
    close_trade = op.last_asof(trade, close)    
```

  
Now to obtain the highest price in a given interval we would sample the
last value that was achieved before the close feature is activated of
the trade feature as of the time of the maximum value of the trade
price, using the first trade activation to reset the max feature.

``` python
    high_trade = op.last_asof(op.asof(trade, op.max(trade_px, first_trade)), close)
```

  
We will do something very similar to obtain the trade sample at the
moment of the minimum trade in the interval.

``` python
    low_trade = op.last_asof(op.asof(trade, op.min(trade_px, first_trade)), close)
```

  
Finally we will compute the cumulative trade sample we would like to add
to our bars

This can be achieved with the difference between the cumulative trade
sample on the current interval and the sample on the previous interval,
using the following code:

``` python
    ctrdt_sampled = op.left_lim(ctrdt, close)
    ctrdt_sampled_lagged = op.tick_lag(ctrdt_sampled, 1)
    ctrdt_diff = op.diff(ctrdt_sampled, ctrdt_sampled_lagged)
```

## Ensembling the bar data

  
Now that we have computed the data we would like to have in our bars, we
will to combine the features into a single frame and return it from the
method like this:

``` python
    return op.combine(
                open_trade,
                (("price", "open_px"),
                 ("qty", "open_sz"),
                 ("market", "open_exch")),
                close_trade,
                (("price", "close_px"),
                 ("qty", "close_sz"),
                 ("market", "close_exch")),
                high_trade,
                (("price", "high_px"),
                 ("qty", "high_sz"),
                 ("market", "high_exch")),
                low_trade,
                (("price", "low_px"),
                 ("qty", "low_sz"),
                 ("market", "low_exch")),
                open_quote,
                (("bidprice", "open_bidpx"),
                 ("askprice", "open_askpx"),
                 ("bidqty", "open_bidsz"),
                 ("askqty", "open_asksz")),
                close_quote,
                (("bidprice", "close_bidpx"),
                 ("askprice", "close_askpx"),
                 ("bidqty", "close_bidsz"),
                 ("askqty", "close_asksz")),
                high_quote,
                (("bidprice", "high_bidpx"),
                 ("askprice", "high_askpx"),
                 ("bidqty", "high_bidsz"),
                 ("askqty", "high_asksz")),
                low_quote,
                (("bidprice", "low_bidpx"),
                 ("askprice", "low_askpx"),
                 ("bidqty", "low_bidsz"),
                 ("askqty", "low_asksz")),
                tw_quote,
                (("bidprice", "tw_bidpx"),
                 ("askprice", "tw_askpx"),
                 ("bidqty", "tw_bidsz"),
                 ("askqty", "tw_asksz")),
                ctrdt_diff, tuple(),
                close, (("actual", "close_time"),))
```

## Computing the bars

  
Now that we can add all the features we need to generate our bars for
each instrument easily we will use our new method in our main function
like this:

``` python
        bars = [compute_bar(nbbo, trd, ctrdt) for nbbo, trd, ctrdt in zip(nbbos, trade_imnt_split, ctrdts)]
```

  
This will generate a list of the bars for each instrument, wich can be
later joined to generate a stream with the bars for all instruments,
which can be performed in the following way:

``` python
        out_stream = op.join(*bars, "ticker", extr.Array(extr.Char, 16),
            tuple([x["NASDAQOMX"] for x in tickers]))
```

  
We will use a feature to accumulate the output of this stream so it can
be later exported.

``` python
        val_aggr = op.accumulate(out_stream)
```

# Running the simulation and exporting the result

  
We will also define a few methods that will help us specify the end time
of the simulation easily and clean quote data and convert it to the type
that should be used in the calculations.

<!-- -->

  
To specify the bar length and calculate the ending time of the
simulation we will need to add a couple of methods, this methods will
require the pytz dependency and an additional dependency from datetime:

``` python
import pytz
from datetime import datetime, timedelta
```

  
The following method is defined to generate a time delta corresponding
to a specified date

``` python
def epoch_delta(date):
    return date - pytz.timezone("UTC").localize(datetime(1970, 1, 1))
```

  
This method allows us to calculate the time delta of a specified date
with the New York timezone

``` python
def New_York_time(year, mon, day, h=0, m=0, s=0):
    return epoch_delta(pytz.timezone("America/New_York").
        localize(datetime(year, mon, day, h, m, s)))
```

  
Since the timers involved in the computations will not stop generating
future events, we need to set an end to the simulation, in our scenario
it will be at 16:00 on October 16, 2017

``` python
        graph.stream_ctx().run_to(New_York_time(2017, 10, 18, 16))
```

  
Now that we added the required code to run our simulation we would like
to export the accumulated data to a Pandas DataFrame so we will import
Pandas into our script:

``` python
import pandas as pd
```

  
To export the accumulated data we can use the **result_as_pandas**
method like this:

``` python
        as_pd = extr.result_as_pandas(val_aggr)
```

  
Now that the accumulated data is exported, we can use it from Python in
any way we would use any Pandas DataFrame. For example we can store it
as CSV using the Pandas **to_csv** method:

``` python
        as_pd.to_csv("../test/data/bar_20171018.test.csv", index=False)
```

  
It is important to notice that the Pandas **to_csv** method is slower
than the builtin **csv_record** feature to save the output produced by a
feature in a file as CSV.

<!-- -->

  
Following the steps in this tutorial we were able to learn how to
process market data to generate bars and export them using Extractor.

The Python script we built following the tutorial should look like this
at this point:

``` python
import pandas as pd
from datetime import datetime, timedelta
import extractor as extr
import pytz

def epoch_delta(date):
    return date - pytz.timezone("UTC").localize(datetime(1970, 1, 1))

def New_York_time(year, mon, day, h=0, m=0, s=0):
    return epoch_delta(pytz.timezone("America/New_York").
        localize(datetime(year, mon, day, h, m, s)))

def quote_side_float64(quote, name):
    return op.cond(op.is_zero(op.field(quote, name)),
                   op.nan(quote),
                   op.convert(quote, extr.Float64))

def quote_float64(quote):
    bid_quote = op.fields(quote, ("bidprice", "bidqty"))
    ask_quote = op.fields(quote, ("askprice", "askqty"))
    return op.combine(quote_side_float64(bid_quote, "bidqty"), tuple(),
                      quote_side_float64(ask_quote, "askqty"), tuple())

def compute_bar(nbbo, trades, ctrdt):
    bar_period = timedelta(minutes=5)
    close = op.timer(bar_period)
    quote = op.fields(nbbo, ("bidprice", "askprice", "bidqty", "askqty"))
    quote_bid = op.field(nbbo, "bidprice")
    quote_ask = op.field(nbbo, "askprice")
    open_quote = op.asof_prev(quote, close)
    close_quote = op.left_lim(quote, close)
    high_quote = op.left_lim(op.asof(quote, op.max(quote_ask, close)), close)
    low_quote = op.left_lim(op.asof(quote, op.min(quote_bid, close)), close)
    tw_quote = op.average_tw(quote_float64(quote), close)
    trade = op.fields(trades, ("price", "qty", "market"))
    trade_px = op.field(trade, "price")
    first_trade = op.first_after(trade, close)
    open_trade = op.last_asof(first_trade, close)
    close_trade = op.last_asof(trade, close)
    high_trade = op.last_asof(op.asof(trade, op.max(trade_px, first_trade)), close)
    low_trade = op.last_asof(op.asof(trade, op.min(trade_px, first_trade)), close)
    ctrdt_sampled = op.left_lim(ctrdt, close)
    ctrdt_sampled_lagged = op.tick_lag(ctrdt_sampled, 1)
    ctrdt_diff = op.diff(ctrdt_sampled, ctrdt_sampled_lagged)
    return op.combine(
                open_trade,
                (("price", "open_px"),
                 ("qty", "open_sz"),
                 ("market", "open_exch")),
                close_trade,
                (("price", "close_px"),
                 ("qty", "close_sz"),
                 ("market", "close_exch")),
                high_trade,
                (("price", "high_px"),
                 ("qty", "high_sz"),
                 ("market", "high_exch")),
                low_trade,
                (("price", "low_px"),
                 ("qty", "low_sz"),
                 ("market", "low_exch")),
                open_quote,
                (("bidprice", "open_bidpx"),
                 ("askprice", "open_askpx"),
                 ("bidqty", "open_bidsz"),
                 ("askqty", "open_asksz")),
                close_quote,
                (("bidprice", "close_bidpx"),
                 ("askprice", "close_askpx"),
                 ("bidqty", "close_bidsz"),
                 ("askqty", "close_asksz")),
                high_quote,
                (("bidprice", "high_bidpx"),
                 ("askprice", "high_askpx"),
                 ("bidqty", "high_bidsz"),
                 ("askqty", "high_asksz")),
                low_quote,
                (("bidprice", "low_bidpx"),
                 ("askprice", "low_askpx"),
                 ("bidqty", "low_bidsz"),
                 ("askqty", "low_asksz")),
                tw_quote,
                (("bidprice", "tw_bidpx"),
                 ("askprice", "tw_askpx"),
                 ("bidqty", "tw_bidsz"),
                 ("askqty", "tw_asksz")),
                ctrdt_diff, tuple(),
                close, (("actual", "close_time"),))

if __name__ == "__main__":
        graph = extr.system.comp_graph()
        op = graph.features
        bbo_file =  "../test/sip_quotes_20171018.base.mp"
        trade_file = "../test/sip_trades_20171018.base.mp"
        markets = ["NYSEMKT", "NASDAQOMX", "NYSEArca"]
        tickers = [
          {"NYSEMKT": "A", "NASDAQOMX": "A", "NYSEArca": "A"},
          {"NYSEMKT": "AA", "NASDAQOMX": "AA", "NYSEArca": "AA"},
          {"NYSEMKT": "BA", "NASDAQOMX": "BA", "NYSEArca": "BA"}
        ]
        bbos_in = op.mp_play(
              bbo_file,
              (("receive", extr.Time64, ""),
               ("ticker", extr.Array(extr.Char, 16), ""),
               ("market", extr.Array(extr.Char, 32), ""),
               ("bidprice", extr.Decimal64, ""),
               ("askprice", extr.Decimal64, ""),
               ("bidqty", extr.Int32, ""),
               ("askqty", extr.Int32, "")))
        trades_in = op.mp_play(
              trade_file,
              (("receive", extr.Time64, ""),
               ("ticker", extr.Array(extr.Char, 16), ""),
               ("market", extr.Array(extr.Char, 32), ""),
               ("price", extr.Decimal64, ""),
               ("qty", extr.Int32, ""),
               ("side", extr.Int32, "")))
        bbo_split = op.split(bbos_in, "market", tuple(markets))
        trade_split = op.split(trades_in, "market", tuple(markets))
        bbos = []
        ctrds = []
        mkt_idx = 0;
        for mkt in markets:
            mkt_tickers = [x[mkt] for x in tickers]
            mkt_bbo_split = op.split(bbo_split[mkt_idx], "ticker", tuple(mkt_tickers))
            mkt_trade_split = op.split(trade_split[mkt_idx], "ticker", tuple(mkt_tickers))
            mkt_bbos = []
            mkt_ctrds = []
            ticker_idx = 0
            for _ in tickers:
                bbo = mkt_bbo_split[ticker_idx]
                trade = mkt_trade_split[ticker_idx]
                cum_trade = op.cum_trade(trade)
                mkt_bbos.append(bbo)
                mkt_ctrds.append(cum_trade)
                ticker_idx = ticker_idx + 1
            bbos.append(mkt_bbos)
            ctrds.append(mkt_ctrds)
            mkt_idx = mkt_idx + 1
        nbbos = [op.bbo_aggr(*x) for x in zip(*bbos)]
        ctrdts = [op.cum_trade_total(*x) for x in zip(*ctrds)]
        trade_imnt_split = op.split(trades_in, "ticker", tuple([x["NASDAQOMX"] for x in tickers]))
        bars = [compute_bar(nbbo, trd, ctrdt) for nbbo, trd, ctrdt in zip(nbbos, trade_imnt_split, ctrdts)]
        out_stream = op.join(*bars, "ticker", extr.Array(extr.Char, 16),
          tuple([x["NASDAQOMX"] for x in tickers]))
        val_aggr = op.accumulate(out_stream)
        graph.stream_ctx().run_to(New_York_time(2017, 10, 18, 16))
        as_pd = extr.result_as_pandas(val_aggr)
        as_pd.to_csv("../test/bar_20171018.test.csv", index=False)
```
