#pragma once

#include "OrderBookVisitor.h"
#include "OrderBook.h"

class PruningVisitor : public OrderBookVisitor 
{
public:
    void visit(OrderBook& orderBook) override 
    { orderBook.pruneGoodForDayOrders(); }
};
