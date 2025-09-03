cmake_minimum_required(VERSION 4.1)
project(TradingEngine)

find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)

add_library(
    TradingEngineLib 
    STATIC
        src/itchparser.cpp
        src/orderbook/OrderBook.cpp
)

target_include_directories(
    TradingEngineLib
    PUBLIC
        include
        include/orderbook
)

add_executable(
    TradingEngine
    src/main.cpp
)

target_link_libraries(
    TradingEngine
    PRIVATE
        TradingEngineLib
)