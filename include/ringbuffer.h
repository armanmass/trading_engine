#pragma once
#include <atomic>
#include <cstddef>
#include <memory>
#include <new>
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

        void pop(T& value)
        {
            if (!empty())
                value = std::move(ring_[pop_cursor_++ % capacity_]);
        }

        // can return shared ptr as well? more expensive?
        // std::atomic_sharedptr?
        template <typename U>
        void push(U&& value)
        {
            if (full()) return;
            new (&ring_[push_cursor_++ % capacity_]) T(std::forward<U>(value));
        }

        [[nodiscard]] bool empty() const noexcept { return pop_cursor_ == push_cursor_; }
        [[nodiscard]] size_t size() const noexcept { return push_cursor_-pop_cursor_; }
        [[nodiscard]] size_t capacity() const noexcept { return capacity_; }
        [[nodiscard]] bool full() const noexcept { return size() == capacity_; }

    private:
        static constexpr size_t capacity_{ 1000 };
        static constexpr size_t cache_line_size_{ 64 };
        T* ring_;

        alignas(cache_line_size_) std::atomic<size_t> push_cursor_{ };
        char padding_push_[cache_line_size_ - sizeof(size_t)];

        alignas(cache_line_size_) std::atomic<size_t> pop_cursor_{ };
        char padding_pop_[cache_line_size_ - sizeof(size_t)];


        static_assert(std::atomic<size_t>::is_always_lock_free);
        [[no_unique_address]] Allocator alloc_;

};