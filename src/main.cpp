#include "OrderBook.h"
#include "OrderDirectory.h"
#include "PruningVisitor.h"

#include "ringbuffer.h"
#include "rawmessage.h"

#include "itchparser.h"
#include "itch_msg.h"
#include "debug.h"

#include <atomic>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <chrono>
#include <thread>
#include <stop_token>

std::atomic<uint64_t> processed_message_count{0};
std::atomic<bool> shutdown_flag{false};

void signal_handler(int /*signal*/)
{
    shutdown_flag.store(true, std::memory_order_relaxed);
}

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
    std::signal(SIGINT, signal_handler);

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <itch_file path>" << std::endl;
        return EXIT_FAILURE;
    }

    LOG_DEBUG("Producer thread running...");

    std::string itchFilePath = argv[1];
    ITCHParser itchParser(itchFilePath);

    std::unordered_map<std::string, OrderBook> orderBooks;
    OrderDirectory orderDirectory;
    RingBuffer<RawMessage> eventQueue;

    std::atomic<bool> producerDone{ false };

    auto start_time = std::chrono::high_resolution_clock::now();

    std::jthread consumer{
        consumerWorker, std::ref(orderBooks), std::ref(orderDirectory),
        std::ref(eventQueue), std::ref(producerDone)
    };

    std::jthread pruner{
        prunerWorker, std::ref(orderBooks)
    };

    // exits loop if nextMsg returns std::nullopt or shutdown is signaled
    while (!shutdown_flag.load(std::memory_order_relaxed))
    {
        auto optionalMsgSpan = itchParser.nextMsg();
        if (!optionalMsgSpan)
            break;

        std::span<const std::byte> itchMsg = optionalMsgSpan.value();

        RawMessage rawMsg;
        std::memcpy(rawMsg.data_.data(),itchMsg.data(),itchMsg.size());

        while (!shutdown_flag.load(std::memory_order_relaxed) && !eventQueue.push(rawMsg))
            std::this_thread::yield();
    }

    producerDone.store(true, std::memory_order_release);
    
    // jthread destructor joins the thread
    // Wait for consumer to finish
    consumer.join();

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end_time - start_time);

    const double messages_per_second = processed_message_count.load() / duration.count();

    std::cout << "\n--- BENCHMARK " << (shutdown_flag.load() ? "INTERRUPTED" : "COMPLETE") << " ---" << std::endl;
    std::cout << "Processed " << processed_message_count.load() << " messages." << std::endl;
    std::cout << "Elapsed Time: " << std::fixed << std::setprecision(2) << duration.count() << " seconds." << std::endl;
    std::cout << "Messages/Second: " << std::fixed << std::setprecision(0) << messages_per_second << std::endl;


    return EXIT_SUCCESS;
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

        while (system_clock::now() < next_prune_time && !stoken.stop_requested())
        {
            std::this_thread::sleep_for(seconds(1));
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
    LOG_DEBUG("Consumer thread running...");

    RawMessage rawMsg;
    while (true)
    {
        if (eventQueue.pop(rawMsg))
        {
            submitOrder(orderBooks, orderDirectory, rawMsg);
            processed_message_count.fetch_add(1, std::memory_order_relaxed);
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
            auto* addRaw = reinterpret_cast<ITCH50::AddOrderMsg*>(rawMsg.data_.data());
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
            LOG_DEBUG("Received message type: " << msgType);
            throw std::logic_error("Unsupported message type.");
    }
}
