#include "ringbuffer.h"
#include <benchmark/benchmark.h>
#include <thread>

static void BM_RingBuffer(benchmark::State& state) {
    static RingBuffer<int> rb;

    if (state.thread_index() == 0) 
    {
        uint64_t x{};
        for (auto _ : state) 
        {
            while (rb.full()) 
            {
                std::this_thread::yield();
            }
            rb.push(++x);
        }
    }
    else if (state.thread_index() == 1) 
    {
        int value;
        for (auto _ : state) 
        {
            while (rb.empty()) 
            {
                std::this_thread::yield();
            }
            rb.pop(value);
        }
        state.SetItemsProcessed(state.iterations());
    }
}

BENCHMARK(BM_RingBuffer)
    ->Threads(2)
    ->MinTime(5.0)
    ->UseRealTime();

BENCHMARK_MAIN();