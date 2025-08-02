# trading_engine

## No Atomics (Data Race)
-------------------------------------------------------------------------------
Benchmark                                                 Time             CPU 
-------------------------------------------------------------------------------
BM_RingBuffer/min_time:5.000/real_time/threads:2       46.9 ns         42.6 ns
  147923740 items_per_second=10.6608M/s

## std::atomic on push_cursor_ and pop_cursor_ (Data Race)

--------------------------------------------------------------------------------
Benchmark                                                 Time             CPU  
--------------------------------------------------------------------------------
BM_RingBuffer/min_time:5.000/real_time/threads:2        110 ns          100 ns
   65826096 items_per_second=4.53887M/s

## std::atomic on push_cursor_ and pop_cursor_ + cache alighnment (Data Race)
### Note: tested padding as well yielded similar peformace

--------------------------------------------------------------------------------
Benchmark                                                 Time             CPU  
--------------------------------------------------------------------------------
BM_RingBuffer/min_time:5.000/real_time/threads:2       93.5 ns         85.2 ns
   75825240 items_per_second=5.34539M/s


