#pragma once
#include "Order.h"
#include <unordered_map>

class OrderBook;

class OrderDirectory
{
public:
    void addOrderID(OrderId orderId, OrderBook* orderBook) { idToOrderBook[orderId] = orderBook; }

    void delOrderID(OrderId orderId) { idToOrderBook.erase(orderId); }

    OrderBook* getOrderBook(OrderId orderId)
    {
        auto it = idToOrderBook.find(orderId);
        if (it == idToOrderBook.end()) return nullptr;
        return it->second;
    }

private:
    std::unordered_map<OrderId, OrderBook*> idToOrderBook;
};
