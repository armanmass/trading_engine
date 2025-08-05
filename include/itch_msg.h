#pragma once
#include <bit>
#include <cstdint>

template <typename T>
constexpr T bs(T value) { return std::byteswap(value); }

namespace ITCH50
{
#pragma pack(push, 1)
    struct AddOrderMsg
    {
        char MessageType;
        uint16_t StockLocation;
        uint16_t TrackingNumber;
        uint64_t Timestamp;
        uint64_t OrderID;
        char BuyOrSell;
        uint32_t Shares;
        char Stock[8];
        uint32_t Price;

        void convert_network_to_host()
        {
            StockLocation = bs(StockLocation);
            TrackingNumber = bs(TrackingNumber);
            Timestamp = bs(Timestamp);
            OrderID = bs(OrderID);
            Shares = bs(Shares);
            Price = bs(Price);
        }
    };

    struct CancelOrderMsg
    {

        char MessageType;
        uint16_t StockLocation;
        uint16_t TrackingNumber;
        uint64_t Timestamp;
        uint64_t OrderID;
        uint32_t CancelledShares;

        void convert_network_to_host()
        {
            StockLocation = bs(StockLocation);
            TrackingNumber = bs(TrackingNumber);
            Timestamp = bs(Timestamp);
            OrderID = bs(OrderID);
            CancelledShares = bs(CancelledShares);
        }
    };

    struct ModifyOrderMsg
    {
        char MessageType;
        uint16_t StockLocation;
        uint16_t TrackingNumber;
        uint64_t Timestamp;
        uint64_t OldOrderID;
        uint64_t NewOrderID;
        uint32_t Shares;
        uint32_t Price;

        void convert_network_to_host()
        {
            StockLocation = bs(StockLocation);
            TrackingNumber = bs(TrackingNumber);
            Timestamp = bs(Timestamp);
            OldOrderID = bs(OldOrderID);
            NewOrderID = bs(NewOrderID);
            Shares = bs(Shares);
            Price = bs(Price);
        }

    };

#pragma pack(pop)
}