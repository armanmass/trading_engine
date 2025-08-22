cmake_minimum_required(VERSION 4.1)
project(TradingEngine LANGUAGES CXX)

find_package(GTest REQUIRED)
find_package(benchmark REQUIRED)

file(GLOB_RECURSE HEADERS include/*.h include/*.hpp)
file(GLOB_RECURSE SOURCES src/*.cpp)
list(FILTER SOURCES EXCLUDE REGEX ".*main\\.cpp$")

add_library(
    TradingEngineLib
    ${HEADERS}
    ${SOURCES}
)

target_include_directories(
    TradingEngineLib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/include/orderbook
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/orderbook
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