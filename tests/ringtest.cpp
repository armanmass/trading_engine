#include "ringbuffer.h"
#include <gtest/gtest.h>


TEST(OriginalRingBufferTest, Initialization) {
    RingBuffer<int> rb;
    EXPECT_EQ(rb.capacity(), 1000);
}

TEST(OriginalRingBufferTest, PushAndPop) {
    RingBuffer<int> rb;
    rb.push(10);

    rb.push(20);

    int val = 0;
    rb.pop(val);
    EXPECT_EQ(val, 10);

    rb.pop(val);
    EXPECT_EQ(val, 20);
}

TEST(OriginalRingBufferTest, PopFromEmpty) {
    RingBuffer<int> rb;
    int original_value = 123;
    int val = original_value;
    
    rb.pop(val);
    
}

TEST(OriginalRingBufferTest, PushToFull) {
    RingBuffer<int> rb;
    for (int i = 0; i < 1000; ++i) {
        rb.push(i);
    }

    rb.push(9999);

    int last_val = 0;
    for (int i = 0; i < 999; ++i) {
        rb.pop(last_val);
    }
    rb.pop(last_val);
    EXPECT_EQ(last_val, 999);
}

TEST(OriginalRingBufferTest, WrapAround) {
    RingBuffer<int> rb;
    for (int i = 0; i < 1000; ++i) {
        rb.push(i);
    }

    int val = 0;
    for (int i = 0; i < 500; ++i) {
        rb.pop(val);
        ASSERT_EQ(val, i);
    }

    for (int i = 1000; i < 1500; ++i) {
        rb.push(i);
    }

    for (int i = 500; i < 1500; ++i) {
        rb.pop(val);
        ASSERT_EQ(val, i);
    }
}