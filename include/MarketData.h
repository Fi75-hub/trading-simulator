// Holds market orders and generates candlestick summaries for a product across a timeframe (Task 1).
#pragma once
#include <string>
#include <vector>
#include "Order.h"
#include "Candlestick.h"

// Timeframe controls how orders are grouped into candlesticks.
enum class Timeframe { Daily, Monthly, Yearly };

// MarketData holds orders and offers candlestick summaries.
class MarketData
{
public:
    // Loads orders from the market CSV and builds the product list.
    bool load(const std::string& csvFile, std::string& error);
    // Returns the available product pairs.
    const std::vector<std::string>& products() const;

    // Returns a recent price for the product (used as a baseline for simulation).
    double referencePrice(const std::string& product) const;

    // Builds OHLC candles for the chosen product, side and timeframe.
    std::vector<Candlestick> candlesticks(const std::string& product, OrderSide side, Timeframe tf) const;

    void clearSimulated();
    void addSimulatedOrder(const Order& o);

private:
    std::vector<Order> orders_;
    std::vector<Order> simulated_;
    std::vector<std::string> products_;

    // Extracts the grouping key from a timestamp.
    static std::string periodKey(const std::string& timestamp, Timeframe tf);
};
