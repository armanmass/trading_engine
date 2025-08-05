#include "itchparser.h"

std::optional<std::span<const std::byte>>
ITCHParser::nextMsg()
{
    if (buffer_pos_ == buffer_end_ && !fillBuffer())
        return std::nullopt;

    while (true)
    {
        const char msg_type = static_cast<char>(buffer_[buffer_pos_]);
        const size_t msg_size = getMsgSize(msg_type);


        if ((buffer_pos_ + msg_size > buffer_end_) && !fillBuffer())
            return std::nullopt;

        if (!isSupportedMsgType(msg_type))
        {
            buffer_pos_ += msg_size;
            continue;
        }
        
        std::span<const std::byte> msg_span{ buffer_.data()+buffer_pos_, msg_size };
        buffer_pos_ += msg_size;
        return msg_span;
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

size_t ITCHParser::getMsgSize(char msg_type) const
{
    switch (msg_type)
    {
    // supported message types add cancel modify
    case 'A':
        return 36;
    case 'X':
        return 23;
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
    case 'D':
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
        throw std::logic_error("Unknown message type: " + std::string(1,msg_type) + "\n");
    }
}
