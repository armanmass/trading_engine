#pragma once
#include "Order.h"
#include "OrderModify.h"
#include "TradeInfo.h"
#include "LevelInfo.h"
#include "MatchResult.h"

#include <mutex>
#include <map>
#include <unordered_map>
#include <cstddef>

class OrderBookVisitor;

class OrderBook 
{
    private:
        struct OrderEntry;
    public:
        OrderBook();
        OrderBook(std::string&& instrument);
        ~OrderBook();

        OrderBook(const OrderBook&)       = delete;
        void operator=(const OrderBook&)  = delete;
        OrderBook(const OrderBook&&)      = delete;
        void operator=(const OrderBook&&) = delete;

        void accept(OrderBookVisitor& visitor);

        MatchResult addOrder(OrderPointer order);
        bool addOrderCoreLogic(OrderPointer order);

        void cancelOrder(OrderId orderId);
        MatchResult modifyOrder(OrderModify modifiedOrder);

        OrderbookLevelInfos getOrderInfos() const;

        std::size_t size() const { return orders_.size(); }
        const std::string& getInstrument() const { return instrument_; }
        OrderEntry* getOrderEntry(OrderId orderId) 
        { 
            if (!orders_.contains(orderId))
                return nullptr; 
            return &orders_[orderId]; 
        }

        void pruneGoodForDayOrders();


        struct LevelData 
        {
            Quantity quantity_{ };
            Quantity count_{ };

            enum class Action { Add, Remove, Match };
        };

        bool hasMatch(Side side, Price price) const;
        MatchResult matchOrders();

        bool canFullyFill(Side side, Price price, Quantity quantity) const;

        void cancelOrders(OrderIds orderIds);
        void cancelOrderInternal(OrderId orderId);

        void onOrderAdded(OrderPointer order);
        void onOrderCancelled(OrderPointer order);
        void onOrderMatched(Price price, Quantity quantity, bool isFullyFilled);
        void updateLevelData(Price price, Quantity quantity, LevelData::Action action);
        std::unordered_map<Price, LevelData> data_;

    private:
        struct OrderEntry 
        {
            OrderPointer order_{ nullptr };
            OrderPointers::iterator location_;
        };

        std::string instrument_;

        std::map<Price, OrderPointers, std::greater<Price>> bids_;
        std::map<Price, OrderPointers, std::less<Price>> asks_;

        std::unordered_map<OrderId, OrderEntry> orders_;

        mutable std::mutex ordersMutex_;
};