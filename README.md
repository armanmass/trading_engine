# trading_engine

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
