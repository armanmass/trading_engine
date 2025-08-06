#pragma once

#include <cstddef>
#include <array>
#include <cstdint>

constexpr static size_t MAX_ITCH_MSG_SIZE{ 64 };

struct RawMessage
{
    std::array<std::byte, MAX_ITCH_MSG_SIZE> data_;
    uint8_t size_;
};