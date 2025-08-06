#include "itchparser.h"
#include <optional>
// #include <iostream>

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

// std::cout << "msgSize: " << static_cast<int>(buffer_[buffer_pos_]) << std::endl;
        // read size byte and increment
        const size_t msgSize = static_cast<int>(buffer_[buffer_pos_++]);

        // now pointing at msgtype byte
        // if went off buffer we have to refill
        if (buffer_pos_ == buffer_end_ && !fillBuffer())
                return std::nullopt;
        // read msgtype but don't advance since msgtype is included in msgsize
// std::cout << "msgType: " << static_cast<char>(buffer_[buffer_pos_]) << std::endl;
        const char msgType = static_cast<char>(buffer_[buffer_pos_]);


        if ((buffer_pos_ + msgSize > buffer_end_) && !fillBuffer())
            return std::nullopt;

        if (!isSupportedMsgType(msgType))
        {
            buffer_pos_ += msgSize;
            continue;
        }
        
        std::span<const std::byte> msgSpan{ buffer_.data()+buffer_pos_, msgSize };
        buffer_pos_ += msgSize;
        return msgSpan;
    }

    return std::nullopt;
}

bool ITCHParser::fillBuffer()
{
    // if unread bytes in buffer
        // get length of unread bytes
        // move to front of buffer
        // set start read index to length
        // read in buffer_size - length of unread bytes
    // else 
        //just read full buffer
        //we could bypass if else and just do all the compute
        //avoiding branching is probably best practice 
    
    size_t unread_bytes = buffer_end_ - buffer_pos_;
    std::move(buffer_.begin()+buffer_pos_, buffer_.begin()+buffer_end_, buffer_.begin());

    size_t to_read_bytes = size() - unread_bytes;
    itch_file_.read(reinterpret_cast<char*>(buffer_.data() + unread_bytes), to_read_bytes);


    buffer_pos_ = 0;
    buffer_end_ = unread_bytes + itch_file_.gcount();

    return itch_file_.good();
}

size_t ITCHParser::getMsgSize(char msgType) const
{
    switch (msgType)
    {
    // supported message types add cancel modify
    case 'A':
        return 36;
    case 'D':
        return 19;
    case 'U':
        return 35;

    // unsupported message types
    // kept for knowing how many bytes to skip in input stream
    case 'F':
        return 40;
    case 'E':
        return 31;
    case 'C':
        return 36;
    case 'X':
        return 19;
    case 'P':
        return 44;
    case 'Q':
        return 40;
    case 'B':
        return 19;
    case 'S':
        return 12;
    case 'R':
        return 39;
    case 'H':
        return 25;
    case 'Y':
        return 20;
    case 'L':
        return 26;
    case 'V':
        return 35;
    case 'W':
        return 12;
    case 'K':
        return 28;
    case 'J':
        return 35;
    case 'h':
        return 21;
    case 'I':
        return 50;
    case 'N':
        return 20;
    case 'O':
        return 48;
    default:
        return 0;
    }
}
