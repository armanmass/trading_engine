#include "Order.h"
#include "OrderBook.h"
#include "itch_msg.h"
#include "itchparser.h"
#include "ringbuffer.h"
#include "rawmessage.h"

#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <thread>
#include <atomic>

void submit_order(OrderBook& orderBook, RawMessage& rawMsg);

void consumer_worker(
    OrderBook& orderBook,
    RingBuffer<RawMessage>& eventQueue,
    std::atomic<bool>& producerDone
)
{
std::cout << "Consumer thread running..." << std::endl;
    RawMessage rawMsg;
    while (true)
    {
        if (eventQueue.pop(rawMsg))
        {
            submit_order(orderBook, rawMsg);
        }
        else
        {
            if (producerDone)
                break;
            std::this_thread::yield();
        }
    }
}

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <itch_file path>" << std::endl;
        return -1;
    }
std::cout << "Producer thread running..." << std::endl;

    std::string itchFilePath = argv[1];
    ITCHParser itchParser(itchFilePath);
    OrderBook orderBook;
    RingBuffer<RawMessage> eventQueue;
    std::atomic<bool> producerDone{ false };

    std::thread Consumer{
        consumer_worker, std::ref(orderBook), std::ref(eventQueue), std::ref(producerDone)
    };

    // exits loop if nextMsg returns std::nullopt
    size_t cnt{};
    while (auto optionalMsgSpan = itchParser.nextMsg())
    {
        std::span<const std::byte> itchMsg = optionalMsgSpan.value();

        RawMessage rawMsg;
        std::memcpy(rawMsg.data_.data(),itchMsg.data(),itchMsg.size());
        rawMsg.size_ = itchMsg.size();
std::cout << "Message type: " << static_cast<char>(rawMsg.data_.data()[0]) << " Message number: " << ++cnt << std::endl;
        while (!eventQueue.push(rawMsg))
            std::this_thread::yield();
    }

    producerDone.store(true, std::memory_order_release);
    Consumer.join();

    return 0;
}

void submit_order(OrderBook& orderBook, RawMessage& rawMsg)
{
    const auto msgType = static_cast<char>(rawMsg.data_[0]);
    // A add D cancel U update/modify (cancel old order id/add new order)
    switch (msgType) 
    {
        case 'A':
        {
            auto* addRaw= reinterpret_cast<ITCH50::AddOrderMsg*>(rawMsg.data_.data());
            auto newOrder = std::make_shared<Order>(
                OrderType::GoodTillCancel,
                (OrderId)addRaw->OrderID,
                addRaw->BuyOrSell == 'B' ? Side::Buy : Side::Sell,
                (Price)addRaw->Price,
                (Quantity)addRaw->Shares
            );
            orderBook.addOrder(newOrder);
            break;
        }
        case 'D':
        {
            auto* delRaw = reinterpret_cast<ITCH50::CancelOrderMsg*>(rawMsg.data_.data());
            orderBook.cancelOrder(delRaw->OrderID);
            break;
        }
        case 'U':
        {
            auto* modRaw = reinterpret_cast<ITCH50::ModifyOrderMsg*>(rawMsg.data_.data());
            auto orderEntry = orderBook.getOrderEntry(modRaw->OldOrderID);
            if (orderEntry == nullptr)
                break;

            auto oldSide = orderEntry->order_->getSide();
            orderBook.cancelOrder(modRaw->OldOrderID);

            auto newOrder = std::make_shared<Order>(
                OrderType::GoodTillCancel,
                (OrderId)modRaw->NewOrderID,
                oldSide,
                (Price)modRaw->Price,
                (Quantity)modRaw->Shares
            );

            orderBook.addOrder(newOrder);
        }
        default:
            throw std::logic_error("Unsupported message type.");
    }
}