#pragma once

class OrderBook;

class OrderBookVisitor 
{
public:
    virtual ~OrderBookVisitor() = default;
    virtual void visit(OrderBook& orderBook) = 0;
};
