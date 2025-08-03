#pragma once
#include <atomic>
#include <cstddef>
#include <memory>
#include <utility>

template <typename T, typename Allocator = std::allocator<T>>
class RingBuffer
{
    public:
        RingBuffer()
            : alloc_{Allocator()},
              ring_{std::allocator_traits<Allocator>::allocate(alloc_, capacity_)}
        { }

        ~RingBuffer()
        {
            std::destroy(ring_, ring_+capacity_);
            std::allocator_traits<Allocator>::deallocate(alloc_, ring_,capacity_);
        }

        // cant access full / empty externally now
        // use return bool to notify thread of successful/failed operation
        bool pop(T& value)
        {
            // acquire relaxed cause i own it
            auto popCursor = pop_cursor_.load(std::memory_order_relaxed);

            if (empty(cached_push_cursor_, popCursor)) 
            {
                cached_push_cursor_ = push_cursor_.load(std::memory_order_acquire);
                if (empty(cached_push_cursor_, popCursor))
                    return false;
            }

            value = std::move(ring_[popCursor % capacity_]);
            pop_cursor_.store(popCursor+1, std::memory_order_release);
            return true;
        }

        template <typename U>
        bool push(U&& value)
        {
            auto pushCursor= push_cursor_.load(std::memory_order_relaxed);

            if (full(pushCursor, cached_pop_cursor_)) 
            {
                cached_pop_cursor_ = pop_cursor_.load(std::memory_order_acquire);
                if (full(pushCursor, cached_pop_cursor_)) 
                    return false;
            }

            new (&ring_[pushCursor % capacity_]) T(std::forward<U>(value));
            push_cursor_.store(pushCursor+1, std::memory_order_release);
            return true;
        }


        // capacity
        [[nodiscard]] bool empty(size_t push, size_t pop) const noexcept { return push == pop; }
        [[nodiscard]] size_t size(size_t push, size_t pop) const noexcept { return push-pop; }
        [[nodiscard]] size_t capacity() const noexcept { return capacity_; }
        [[nodiscard]] bool full(size_t push, size_t pop) const noexcept { return (push-pop) == capacity_; }

    private:
        // maybe dont lock capacity make it a ctor param so we can
        // test what sizes are most offer best performance
        static constexpr size_t capacity_{ 1000 };
        static constexpr size_t cache_line_size_{ 64 };
        T* ring_;


        alignas(cache_line_size_) std::atomic<size_t> push_cursor_{ };
        alignas(cache_line_size_) size_t cached_push_cursor_{ };

        alignas(cache_line_size_) std::atomic<size_t> pop_cursor_{ };
        alignas(cache_line_size_) size_t cached_pop_cursor_{ };
        // char padding_pop_[cache_line_size_ - sizeof(size_t)];


        static_assert(std::atomic<size_t>::is_always_lock_free);
        [[no_unique_address]] Allocator alloc_;

};