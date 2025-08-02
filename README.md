# trading_engine

## No Atomics (Data Race)
    items_per_second=10.6608M/s

## std::atomic on push_cursor_ and pop_cursor_ (Data Race)
    items_per_second=4.53887M/s

## std::atomic on push_cursor_ and pop_cursor_ + cache alighnment (Data Race)
### Note: tested padding as well yielded similar peformace
    items_per_second=5.34539M/s


