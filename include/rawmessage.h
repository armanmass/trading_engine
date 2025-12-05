#pragma once

#include <cstddef>
#include <array>

constexpr static size_t MAX_ITCH_MSG_SIZE{ 64 };

struct RawMessage
{
    std::array<std::byte, MAX_ITCH_MSG_SIZE> data_;
};