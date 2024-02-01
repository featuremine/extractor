import os
import extractor as extr
import msgpack as mp
from yamal import ytp
try:
    from time import time_ns
except ImportError:
    from time import time
    def time_ns() -> int:
        return int(time() * 1000000000)

if __name__ == "__main__":
    graph = extr.system.comp_graph()
    op = graph.features

    # Generate File

    try:
        os.remove("synth.ytp.0001")
    except:
        pass

    sequence = ytp.sequence("synth.ytp.0001")
    peer = sequence.peer("peer")
    channel = peer.channel(time_ns(), "channel")
    stream = peer.stream(channel)

    # Generate messages that would not be batched
    first = mp.packb([1, time_ns(), 0, 0, 0, channel.id(), channel.id(), "10", "100", True])
    second = mp.packb([1, time_ns(), 0, 1, 0, channel.id(), channel.id() + 1, "11", "100", False])

    data = first + second

    stream.write(time_ns(), data)

    first = mp.packb([6, time_ns(), 0, 0, 0, channel.id(), channel.id(), channel.id(), "10", "101", True])
    second = mp.packb([6, time_ns(), 0, 1, 1, channel.id(), channel.id() + 1, channel.id() + 1, "11", "101", False])

    data = first + second

    stream.write(time_ns(), data)

    first = mp.packb([6, time_ns(), 0, 0, 1, channel.id(), channel.id(), channel.id(), "10", "102", True])
    second = mp.packb([6, time_ns(), 0, 1, 1, channel.id(), channel.id() + 1, channel.id() + 1, "11", "102", False])

    data = first + second

    stream.write(time_ns(), data)

    first = mp.packb([6, time_ns(), 0, 0, 0, channel.id(), channel.id(), channel.id(), "10", "103", True])
    second = mp.packb([6, time_ns(), 0, 1, 0, channel.id(), channel.id() + 1, channel.id() + 1, "11", "103", False])

    data = first + second

    stream.write(time_ns(), data)

    first = mp.packb([6, time_ns(), 0, 0, 1, channel.id(), channel.id(), channel.id(), "10", "104", True])
    second = mp.packb([6, time_ns(), 0, 1, 0, channel.id(), channel.id() + 1, channel.id() + 1, "11", "104", False])

    data = first + second

    stream.write(time_ns(), data)

    # Run context
    message_count = 0

    def clbck(frame):
        global message_count
        message_count += 1
        print(frame)

    for data in op.seq_ore_sim_split("synth.ytp", ("channel",)):
        book_data = op.book_build(data, 1)  # level = 1
        graph.callback(book_data, clbck)

    graph.stream_ctx().run()

    assert message_count == 3, f"message count is {message_count}, expected 3"
