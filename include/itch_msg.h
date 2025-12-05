#pragma once
#include <bit>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

template <typename T>
[[nodiscard]] constexpr T bs(T value) noexcept 
{ return std::byteswap(value); }

namespace ITCH50
{
#pragma pack(push, 1)
    struct AddOrderMsg
    {
        char MessageType;
        uint16_t StockLocation;
        uint16_t TrackingNumber;
        std::byte Timestamp[6];
        uint64_t OrderID;
        char BuyOrSell;
        uint32_t Shares;
        char Stock[8];
        uint32_t Price;

        void convertNetworkToHost()
        {
            StockLocation = bs(StockLocation);
            TrackingNumber = bs(TrackingNumber);
            OrderID = bs(OrderID);
            Shares = bs(Shares);
            Price = bs(Price);
        }

        std::string getInstrumentName()
        {
            std::string_view instrument(Stock, sizeof(Stock));
            auto end = instrument.find_last_not_of(' ');
            return std::string{ instrument.substr(0,end+1) };
        }
    };

    struct CancelOrderMsg
    {
        char MessageType;
        uint16_t StockLocation;
        uint16_t TrackingNumber;
        std::byte Timestamp[6];
        uint64_t OrderID;

        void convertNetworkToHost()
        {
            StockLocation = bs(StockLocation);
            TrackingNumber = bs(TrackingNumber);
            OrderID = bs(OrderID);
        }
    };

    struct ModifyOrderMsg
    {
        char MessageType;
        uint16_t StockLocation;
        uint16_t TrackingNumber;
        std::byte Timestamp[6];
        uint64_t OldOrderID;
        uint64_t NewOrderID;
        uint32_t Shares;
        uint32_t Price;

        void convertNetworkToHost()
        {
            StockLocation = bs(StockLocation);
            TrackingNumber = bs(TrackingNumber);
            OldOrderID = bs(OldOrderID);
            NewOrderID = bs(NewOrderID);
            Shares = bs(Shares);
            Price = bs(Price);
        }

    };

#pragma pack(pop)
}