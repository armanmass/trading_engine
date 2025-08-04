#pragma  once
#include <fstream>
#include <stdexcept>
#include <vector>
#include "itch_msg.h"

constexpr static size_t BUFFER_SIZE{ 64 * 1024 };


class ITCHParser
{
public:
    ITCHParser(const std::string& filename)
        : itch_file_(filename, std::ios::binary),
          buffer_(BUFFER_SIZE)
    { 
        if (!itch_file_.is_open())
            throw std::runtime_error("Failed to open ITCH file: " + filename);
    }

    ~ITCHParser()
    {
        if (!itch_file_.is_open())
            itch_file_.close();
    }

    ITCHParser(const ITCHParser&) = delete;
    ITCHParser& operator=(const ITCHParser&) = delete;
    ITCHParser(ITCHParser&&) = delete;
    ITCHParser& operator=(ITCHParser&&) = delete;

private:
std::ifstream itch_file_;
std::vector<std::byte> buffer_;

size_t buffer_pos_{};
size_t buffer_end_{};
size_t msg_count_{};
size_t bytes_processed{};
};