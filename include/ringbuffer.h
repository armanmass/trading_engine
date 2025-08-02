#pragma once
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
        T* ring_;
        size_t push_cursor_{ };
        size_t pop_cursor_{ };
        [[no_unique_address]] Allocator alloc_;

};