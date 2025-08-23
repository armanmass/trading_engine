#pragma once
#include "Order.h"
#include <unordered_map>

class OrderDirectory
{
public:
    void addOrderID(OrderId orderId, const std::string& instrument) { idToInstrument[orderId] = instrument; }

    void delOrderID(OrderId orderId) { idToInstrument.erase(orderId); }

    const std::string* getInstrument(OrderId orderId) 
    {
        auto it = idToInstrument.find(orderId);
        if (it == idToInstrument.end()) return nullptr;
        return &it->second;
    }

private:
    std::unordered_map<OrderId, std::string> idToInstrument;
};
