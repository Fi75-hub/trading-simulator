// OHLC candlestick summary for a single time period (Task 1).
#pragma once
#include <string>

// Candlestick stores OHLC values for one time period.
struct Candlestick
{
    // Group key: YYYY, YYYY/MM or YYYY/MM/DD
    std::string period;
    // First price in the period
    double open{};
    // Highest price in the period
    double high{};
    // Lowest price in the period
    double low{};
    // Last price in the period
    double close{};
};
