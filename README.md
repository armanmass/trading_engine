# Demo
![example](https://i.imgur.com/p98v3kw.gif)

# Step 1: Implementing and optimizing SPSC FIFO
## No Atomics (Data Race)
    items_per_second=10.6608M/s

## atomic push_cursor_ and pop_cursor_ (Data Race)
    items_per_second=4.53887M/s

## add cache alighnment (Data Race)
    items_per_second=5.34539M/s

## add acquire/release memory_ordering (No Race)
    items_per_second=8.33712M/s

## add cursor caching (No Race)
    items_per_second=22.2152M/s

## increased buffer size at power of 2 to perform & modulous (No Race)
    items_per_second=34.3127M/s

## add compiler optimizations (No Race)
    items_per_second=218.212M/s


Takeaways:
- Test with controlled variables
- Test different compilers
- Test with controlled compiler options
- Build pipe line permutating variables / compiler options

# Step 2: NASDAQ ITCH Parser
Following [Nasdaq ITCH 5.0 Specification](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf) I created structs of the message size with the appropriate fields using packing to avoid padding. I couldn't find any free ITCH data so I queried an LLM. It found a hidden subdomain that doesn't show up on regular searches. [emi.nasdaq.com](https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/) where I managed to download 8GB of ITCH data for free. I parsed a few bytes at a time initally reading the message size and message type. If I was not supporting the type I would jump ahead message size bytes. Else I would wrap the next message size raw bytes into a RawMessage wrapper which stored the bytes and the size and is reintepreted when consumed.

# Step 3: Producer and Consumer
Main thread creates the orderbook, parser, event queue, and atomic done flag. It then launches the consumer thread passing it reference to the orderbook, queue, and done flag. The main thread handles the input stream and packs the messages into the FIFO using the parser. The consumer gets the RawMessage from the event queue and checks the first byte of the message to learn the message type. It then reinterpret_casts the raw bytes into the correct message type and converts from network to host. It them add the order to the orderbook which performs the matching logic.
