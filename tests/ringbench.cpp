#include "ringbuffer.h"
#include <benchmark/benchmark.h>
#include <cassert>

static void BM_RingBuffer(benchmark::State& state) {
    static RingBuffer<int> rb;

    if (state.thread_index() == 0) 
    {
        int x{};
        for (auto _ : state) 
        {
            if (rb.push(x))
            {
                ++x;
            }
        }
    }
    else if (state.thread_index() == 1) 
    {
        int value{};
        int ev{};

        for (auto _ : state) 
        {
            if (rb.pop(value))
            {
                ++ev;
                //assert(value == ev && "Error wrong value Race Condition.");
            }
        }
        state.SetItemsProcessed(state.iterations());
    }
}

BENCHMARK(BM_RingBuffer)
    ->Threads(2)
    ->MinTime(5.0)
    ->UseRealTime();

BENCHMARK_MAIN();