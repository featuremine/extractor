# Table of Contents

<!--TOC-->

# Format Description

An Ore 1.1.1 file is composed of three elements: The version numbers, a
header, and the message sequence.

## Version

  
The Ore 1.1.1 file includes the Ore format version numbers, they are
written as MessagePack array in the form of the major, minor and
sub-version numbers.

### Compatibility

  
When reading an Ore 1.X.Y file:

  
Sub-versions sharing the same major and minor versions are
interchangeable.

Minor versions are backward compatible.

Major versions are not compatible.

## Header

  
The file header is a MessagePack array of MessagePack maps which contain
the instrument properties.

Currently, the instrument properties include the following:

|                   |          |                                  |
|-------------------|----------|----------------------------------|
| **Property Name** | **Type** | **Description**                  |
| symbol            | String   | Instrument's symbol              |
| price_tick        | Integer  | Denominator for instrument price |

## Message Sequence

  
The book update messages are encoded as MessagePack arrays.

Each book update is composed of the common message header elements and
other elements depending on the message type.

Each field of the array is a message element of a prescribed type.

The message sequence has to start with a **Time message**.

# Message Elements

## Message Header Elements

  
To identify each message, the message type ID is set in the first
element of each message, the message types are enumerated as follows:

|        |                               |
|--------|-------------------------------|
| **ID** | **Message Type**              |
| 0      | Time                          |
| 1      | Order Add                     |
| 2      | Order Insert                  |
| 3      | Order Position                |
| 4      | Order Cancel                  |
| 5      | Order Delete                  |
| 6      | Order Modify                  |
| 7      | Complete Execution            |
| 8      | Complete Execution at a Price |
| 9      | Partial Fill                  |
| 10     | Partial Fill at a Price       |
| 11     | Off-Book Trade                |
| 12     | Status Message                |
| 13     | Book Control Message          |
| 14     | Level Set Message             |
| 15     | Product Announcement          |

  
There are also multiple elements that are used in all the messages
**except for the Time message**, such as the following:

* receive
  * The time in nanoseconds from the time in seconds set with the last '''Time message''' received.
* vendor offset
  * The difference between the receive time and vendor time in nanoseconds
* vendor seqno
  * A sequence number assigned by the vendor
  * This value should be set to Zero if no sequence number is assigned by the vendor
* batch
  * A flag used to denote if the message belongs to a batch. Depending on the batch type of the message, the value of this element can be one of the following:
|           |                |
|-----------|----------------|
| **Value** | **Batch Type** |
| 0         | No Batch       |
| 1         | Batch Begin    |
| 2         | Batch End      |
* imnt id
  * An Integer value that contains the order symbol index in the file header

## Message Specific Elements

### Time Message

    [0, receive]

* receive
  * Long
  * Time since epoch in seconds

> **_NOTE:_**  
> All the message timestamps are offsets from the time received in the
time message.
> Multiple time messages can be received throughout the session.

### Order Add Message

    [1, receive, vendor offset, vendor seqno, batch, imnt id, id, price, qty, is bid]

* id
  * Integer
  * Order Identifier
* price
  * Integer
  * Numerator part of the price of the order
* qty
  * Integer
  * Quantity of the order
* is bid
  * Boolean
  * Represents if the order corresponds to the bid side

### Order Insert Message

    [2, receive, vendor offset, vendor seqno, batch, imnt id, id, priority, price, qty, is bid]

* id
  * Integer
  * Order Identifier
* priority
  * Integer
  * Priority of the order
* price
  * Integer
  * Numerator part of the price of the order
* qty
  * Integer
  * Quantity of the order
* is bid
  * Boolean
  * Represents if the order corresponds to the bid side

### Order Position Message

    [3, receive, vendor offset, vendor seqno, batch, imnt id, id, position, price, qty, is bid]

* id
  * Integer
  * Order Identifier
* position
  * Integer
  * Position of the order in the order book, position values start from zero
* price
  * Integer
  * Numerator part of the price of the order
* qty
  * Integer
  * Quantity of the order
* is bid
  * Boolean
  * Represents if the order corresponds to the bid side

> **_NOTE:_**  
> The difference between inserting and positioning the orders is as
follows:
> Positioning the order places the order at the prescribed position in the
order queue.
> Inserting orders is based on comparing a priority property of the order
which does not change with the lifetime of the order, while position of
the order will change.

### Order Cancel Message

    [4, receive, vendor offset, vendor seqno, batch, imnt id, id, qty]

* id
  * Integer
  * Order Identifier
* qty
  * Integer
  * Quantity to be canceled in the order

### Order Delete Message

    [5, receive, vendor offset, vendor seqno, batch, imnt id, id]

* id
  * Integer
  * Order Identifier

### Order Modify Message

    [6, receive, vendor offset, vendor seqno, batch, imnt id, id, new id, new price, new qty]

* id
  * Integer
  * Order Identifier of modified order
* new id
  * Integer
  * New Order Identifier of modified order
* new price
  * Integer
  * Numerator part of the new price of the order
* new qty
  * Integer
  * New quantity of the order

### Order Executed Whole Message

    [7, receive, vendor offset, vendor seqno, batch, imnt id, id]

* id
  * Integer
  * Order Identifier

### Order Executed Whole at a Price Message

    [8, receive, vendor offset, vendor seqno, batch, imnt id, id, trade price]

* id
  * Integer
  * Order Identifier
* trade price
  * Integer
  * Numerator part of the traded price of the execution

### Order Fill Message

    [9, receive, vendor offset, vendor seqno, batch, imnt id, id, qty]

* id
  * Integer
  * Order Identifier
* qty
  * Integer
  * Quantity filled in the order

> **_NOTE:_**  
> This message contains a partial execution of a given order

### Order Fill at a Price Message

    [10, receive, vendor offset, vendor seqno, batch, imnt id, id, trade price, qty]

* id
  * Integer
  * Order Identifier
* trade price
  * Integer
  * Numerator part of the traded price of the execution
* qty
  * Integer
  * Quantity filled in the execution

> **_NOTE:_**  
> This message contains a partial execution of a given order at a given
price

### Off Book Trade Message

    [11, receive, vendor offset, vendor seqno, batch, imnt id, trade price, qty, decorator]

* trade price
  * Integer
  * Numerator part of the traded price
* qty
  * Integer
  * Quantity filled in the trade
* decorator
  * 8 Character String
  * Optional element to store custom decorator

### Status Message

    [12, receive, vendor offset, vendor seqno, batch, imnt id, id, price, state id, is bid]

* id
  * Integer
  * Order Identifier, the value could be ignored for some states
* price
  * Integer
  * Numerator part of the traded price of the execution, the value could be ignored for some states
* state id
  * Integer
  * Stores a custom state determined by the user
* is bid
  * Boolean
  * Represents if the order corresponds to the bid side, the value could be ignored for some states

### Book Control Message

    [13, receive, vendor offset, vendor seqno, batch, imnt id, uncross, command]

* uncross
  * Integer, zero represents uncrossed as false.
  * Book uncrossed status
  * Command as character

<!-- -->

  
> **_NOTE:_**  
> When the message is written from python and the command is provided,
ord(command_character) should be used instead of a string object of one
character directly.

### Level Set Message

    [14, receive, vendor offset, vendor seqno, batch, imnt id, price, qty, is bid]

* price
  * Integer
  * Numerator part of the traded price
* qty
  * Integer
  * Quantity in level
* is bid
  * Boolean
  * Represents if the order corresponds to the bid side

### Product Announcement Message

price_tick Integer Denominator for symbol price

    [15, receive, vendor offset, vendor seqno, batch, imnt id, symbol, price_tick]

* symbol
  * String
  * Instrument's symbol
* price_tick
  * Integer
  * Denominator for instrument price
