#include "ringbuffer.h"
#include <gtest/gtest.h>


TEST(OriginalRingBufferTest, Initialization) {
    RingBuffer<int> rb;
    EXPECT_TRUE(rb.empty());
    EXPECT_FALSE(rb.full());
    EXPECT_EQ(rb.size(), 0);
    EXPECT_EQ(rb.capacity(), 1000);
}

TEST(OriginalRingBufferTest, PushAndPop) {
    RingBuffer<int> rb;
    rb.push(10);
    EXPECT_EQ(rb.size(), 1);
    EXPECT_FALSE(rb.empty());

    rb.push(20);
    EXPECT_EQ(rb.size(), 2);

    int val = 0;
    rb.pop(val);
    EXPECT_EQ(val, 10);
    EXPECT_EQ(rb.size(), 1);

    rb.pop(val);
    EXPECT_EQ(val, 20);
    EXPECT_EQ(rb.size(), 0);
    EXPECT_TRUE(rb.empty());
}

TEST(OriginalRingBufferTest, PopFromEmpty) {
    RingBuffer<int> rb;
    int original_value = 123;
    int val = original_value;
    
    rb.pop(val);
    
    EXPECT_TRUE(rb.empty());
}

TEST(OriginalRingBufferTest, PushToFull) {
    RingBuffer<int> rb;
    for (int i = 0; i < 1000; ++i) {
        rb.push(i);
    }

    EXPECT_TRUE(rb.full());
    EXPECT_EQ(rb.size(), 1000);

    rb.push(9999);
    EXPECT_TRUE(rb.full());
    EXPECT_EQ(rb.size(), 1000);

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
    EXPECT_EQ(rb.size(), 500);

    for (int i = 1000; i < 1500; ++i) {
        rb.push(i);
    }
    EXPECT_TRUE(rb.full());

    for (int i = 500; i < 1500; ++i) {
        rb.pop(val);
        ASSERT_EQ(val, i);
    }
    EXPECT_TRUE(rb.empty());
}