#include "itchparser.h"
#include <fstream>

std::optional<std::span<const std::byte>>
ITCHParser::nextMsg()
{
    while (true)
    {
        if (buffer_pos_ == buffer_end_ && !fillBuffer())
                return std::nullopt;

        // skip null byte
        if (++buffer_pos_ == buffer_end_ && !fillBuffer())
                return std::nullopt;

        // read size byte and increment
        const size_t msgSize = static_cast<size_t>(buffer_[buffer_pos_++]);

        // now pointing at msgtype byte
        // if went off buffer we have to refill
        if (buffer_pos_ == buffer_end_ && !fillBuffer())
                return std::nullopt;

        // read msgtype but don't advance since msgtype is included in msgsize
        const char msgType = static_cast<char>(buffer_[buffer_pos_]);

        if ((buffer_pos_ + msgSize > buffer_end_) && !fillBuffer())
            return std::nullopt;

        if (!isSupportedMsgType(msgType))
        {
            buffer_pos_ += msgSize;
            continue;
        }
        
        std::span<const std::byte> msgSpan{ buffer_.data() + 
                                            static_cast<buffer_diff_t>(buffer_pos_), 
                                            msgSize };
        buffer_pos_ += msgSize;
        return msgSpan;
    }

    return std::nullopt;
}

bool ITCHParser::fillBuffer()
{
    size_t unread_bytes = buffer_end_ - buffer_pos_;
    std::move(buffer_.begin()+static_cast<buffer_diff_t>(buffer_pos_), 
              buffer_.begin()+static_cast<buffer_diff_t>(buffer_end_), 
              buffer_.begin());

    size_t to_read_bytes = size() - unread_bytes;
    itch_file_.read(reinterpret_cast<char*>(buffer_.data() + unread_bytes), to_read_bytes);


    buffer_pos_ = 0;
    buffer_end_ = unread_bytes + itch_file_.gcount();

    return itch_file_.good();
}
