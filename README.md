# Example
![example](https://i.imgur.com/p98v3kw.gif)

# Step 1: Implementing and optimizing SPSC FIFO
## No Atomics (Data Race)
    items_per_second=10.6608M/s

## atomic push_cursor_ and pop_cursor_ (Data Race)
    items_per_second=4.53887M/s

## atomic push_cursor_ and pop_cursor_ + cache alighnment (Data Race)
### Note: tested padding as well yielded similar peformace
    items_per_second=5.34539M/s

## atomic push_cursor_ and pop_cursor_ + cache alighnment + memory_ordering (No Race)
    items_per_second=8.33712M/s

## atomic push_cursor_ and pop_cursor_ + cache alighnment + memory_ordering + cursor caching (No Race)
    items_per_second=22.2152M/s

## atomic push_cursor_ and pop_cursor_ + cache alighnment + memory_ordering + cursor caching + increased buffer size (No Race)
    items_per_second=34.3127M/s

## atomic push_cursor_ and pop_cursor_ + cache alighnment + memory_ordering + cursor caching + increased buffer size + compiler optimizations (No Race)
    items_per_second=218.212M/s


Takeaways:
- Test with controlled variables
- Test different compilers
- Test with controlled compiler options
- Build pipe line permutating variables / compiler options

# Step 2: NASDAQ ITCH Parser
Following [Nasdaq ITCH 5.0 Specification](https://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHSpecification.pdf) I code parse raw ITCH binary stream. I couldn't find any free ITCH data so I queried an LLM. Funnily it found a hidden subdomain that doesn't show up on any regular searches. [emi.nasdaq.com](https://emi.nasdaq.com/ITCH/Nasdaq%20ITCH/) where I manged to download 8GB of ITCH data for free.
