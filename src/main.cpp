#include "OrderBook.h"
#include "OrderDirectory.h"
#include "PruningVisitor.h"

#include "ringbuffer.h"
#include "rawmessage.h"

#include "itchparser.h"
#include "itch_msg.h"

#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <stop_token>

void submitOrder(std::unordered_map<std::string, OrderBook>& orderBooks, 
                 OrderDirectory& orderDirectory,
                 RawMessage& rawMsg);

void consumerWorker(std::unordered_map<std::string, OrderBook>& orderBooks, 
                    OrderDirectory& orderDirectory,
                    RingBuffer<RawMessage>& eventQueue, 
                    std::atomic<bool>& producerDone);

void prunerWorker(std::stop_token stoken,
                  std::unordered_map<std::string, OrderBook>& orderBooks);

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

    std::unordered_map<std::string, OrderBook> orderBooks;
    OrderDirectory orderDirectory;
    RingBuffer<RawMessage> eventQueue;

    std::atomic<bool> producerDone{ false };

    std::jthread consumer{
        consumerWorker, std::ref(orderBooks), std::ref(orderDirectory),
        std::ref(eventQueue), std::ref(producerDone)
    };

    std::jthread pruner{
        prunerWorker, std::ref(orderBooks)
    };

    // exits loop if nextMsg returns std::nullopt
    while (auto optionalMsgSpan = itchParser.nextMsg())
    {
        std::span<const std::byte> itchMsg = optionalMsgSpan.value();

        RawMessage rawMsg;
        std::memcpy(rawMsg.data_.data(),itchMsg.data(),itchMsg.size());
        rawMsg.size_ = static_cast<decltype(RawMessage::size_)>(itchMsg.size());

        while (!eventQueue.push(rawMsg))
            std::this_thread::yield();
    }

    producerDone.store(true, std::memory_order_release);

    return 0;
}

void prunerWorker(std::stop_token stoken,
                  std::unordered_map<std::string, OrderBook>& orderBooks)
{
    using namespace std::chrono;
    static constexpr auto MarketClose = hours(13);

    while (!stoken.stop_requested())
    {
        auto now = system_clock::now();
        time_t now_t = system_clock::to_time_t(now);
        std::tm time_info{};
        localtime_r(&now_t, &time_info);

        if (time_info.tm_hour >= MarketClose.count())
            time_info.tm_mday++;

        time_info.tm_hour = MarketClose.count();
        time_info.tm_min = 0;
        time_info.tm_sec = 0;

        auto next_prune_time = system_clock::from_time_t(mktime(&time_info));

        auto time_to_wait = next_prune_time - now;

        while (time_to_wait > seconds(0) && !stoken.stop_requested()) 
        {
            std::this_thread::sleep_for(time_to_wait);
            time_to_wait = next_prune_time - system_clock::now();
        }

        if (stoken.stop_requested())
            break;

        PruningVisitor visitor;
        for (auto& [instrument, orderBook] : orderBooks)
            orderBook.accept(visitor);
    }
}


void consumerWorker(
    std::unordered_map<std::string, OrderBook>& orderBooks,
    OrderDirectory& orderDirectory,
    RingBuffer<RawMessage>& eventQueue,
    std::atomic<bool>& producerDone)
{
std::cout << "Consumer thread running..." << std::endl;
    RawMessage rawMsg;
    while (true)
    {
        if (eventQueue.pop(rawMsg))
        {
            submitOrder(orderBooks, orderDirectory, rawMsg);
        }
        else
        {
            if (producerDone) break;
            std::this_thread::yield();
        }
    }
}


void submitOrder(std::unordered_map<std::string, OrderBook>& orderBooks, 
                 OrderDirectory& orderDirectory,
                 RawMessage& rawMsg)
{
    const auto msgType = static_cast<char>(rawMsg.data_[0]);

    // A add, D cancel, U update/modify
    switch (msgType) 
    {
        case 'A':
        {
            auto* addRaw= reinterpret_cast<ITCH50::AddOrderMsg*>(rawMsg.data_.data());
            addRaw->convertNetworkToHost();

            auto newOrder = std::make_shared<Order>(
                OrderType::GoodTillCancel,
                (OrderId)addRaw->OrderID,
                addRaw->BuyOrSell == 'B' ? Side::Buy : Side::Sell,
                (Price)addRaw->Price,
                (Quantity)addRaw->Shares
            );
            
            auto instrument = addRaw->getInstrumentName();
            orderDirectory.addOrderID(addRaw->OrderID, instrument);

            auto [orderbook_it, inserted] = orderBooks.try_emplace(instrument, std::move(instrument));
            orderbook_it->second.addOrder(newOrder);
        }
            break;

        case 'D':
        {
            auto* delRaw = reinterpret_cast<ITCH50::CancelOrderMsg*>(rawMsg.data_.data());
            delRaw->convertNetworkToHost();

            auto instrument = orderDirectory.getInstrument(delRaw->OrderID);
            if (instrument == nullptr)
                break;

            orderBooks[*instrument].cancelOrder(delRaw->OrderID);

            orderDirectory.delOrderID(delRaw->OrderID);
        }
            break;

        case 'U':
        {
            auto* modRaw = reinterpret_cast<ITCH50::ModifyOrderMsg*>(rawMsg.data_.data());
            modRaw->convertNetworkToHost();

            auto instrument = orderDirectory.getInstrument(modRaw->OldOrderID);
            if (instrument == nullptr)
                break;

            auto orderEntry = orderBooks[*instrument].getOrderEntry(modRaw->OldOrderID);
            if (orderEntry == nullptr)
                break;

            auto oldSide = orderEntry->order_->getSide();
            orderBooks[*instrument].cancelOrder(modRaw->OldOrderID);

            auto newOrder = std::make_shared<Order>(
                OrderType::GoodTillCancel,
                (OrderId)modRaw->NewOrderID,
                oldSide,
                (Price)modRaw->Price,
                (Quantity)modRaw->Shares
            );

            orderDirectory.addOrderID(modRaw->NewOrderID, *instrument);
            orderBooks[*instrument].addOrder(newOrder);

            orderDirectory.delOrderID(modRaw->OldOrderID);
        }
            break;

        default:
        std::cout << "Received message type: " << msgType << std::endl;
            throw std::logic_error("Unsupported message type.");
    }
}
