#pragma once
#include "TradeInfo.h"
#include "Order.h"
#include <vector>

struct MatchResult {
    Trades trades;
    OrderIds filledOrders;
};
