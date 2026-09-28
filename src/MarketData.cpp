// Loads market orders and builds candlestick summaries by grouping orders into daily/monthly/yearly periods (Task 1).
#include "MarketData.h"
#include "CSV.h"
#include <algorithm>
#include <map>
#include <stdexcept>

// Loads the market order file and builds a list of available product pairs.
bool MarketData::load(const std::string& csvFile, std::string& error)
{
    try
    {
        orders_ = CSV::readMarketFile(csvFile);
        simulated_.clear();

        products_.clear();
        for (const auto& o : orders_)
            products_.push_back(o.product);

        std::sort(products_.begin(), products_.end());
        products_.erase(std::unique(products_.begin(), products_.end()), products_.end());

        return true;
    }
    catch (const std::exception& ex)
    {
        error = ex.what();
        return false;
    }
}

// Returns the distinct product pairs found in the market file.
const std::vector<std::string>& MarketData::products() const
{
    return products_;
}


void MarketData::clearSimulated()
{
    simulated_.clear();
}

void MarketData::addSimulatedOrder(const Order& o)
{
    simulated_.push_back(o);
}

// Converts a timestamp into a grouping key for the selected timeframe.
std::string MarketData::periodKey(const std::string& timestamp, Timeframe tf)
{
    // Market file format: "YYYY/MM/DD HH:MM:SS.micro"
    // Grouping by substring is safe because the format is fixed and lexicographically sortable.
    if (timestamp.size() < 10) return timestamp;

    if (tf == Timeframe::Daily)
        return timestamp.substr(0, 10);   // YYYY/MM/DD
    if (tf == Timeframe::Monthly)
        return timestamp.substr(0, 7);    // YYYY/MM
    return timestamp.substr(0, 4);        // YYYY
}


// Finds a recent price for a product (used as a baseline for Task 4 pricing).
double MarketData::referencePrice(const std::string& product) const
{
    // Use the most recent order price for the given product.
    bool found = false;
    std::string bestTs;
    double price = 0.0;

    for (const auto& o : orders_)
    {
        if (o.product != product) continue;
        if (!found || o.timestamp > bestTs)
        {
            found = true;
            bestTs = o.timestamp;
            price = o.price;
        }
    }
    return found ? price : 0.0;
}

// Groups orders by period and computes OHLC values for the chosen side.
std::vector<Candlestick> MarketData::candlesticks(const std::string& product, OrderSide side, Timeframe tf) const
{
    std::map<std::string, std::vector<Order>> groups;

    for (const auto& o : orders_)
    {
        if (o.product != product) continue;
        if (o.side != side) continue;

        groups[periodKey(o.timestamp, tf)].push_back(o);
    }

    for (const auto& o : simulated_)
    {
        if (o.product != product) continue;
        if (o.side != side) continue;

        groups[periodKey(o.timestamp, tf)].push_back(o);
    }

    std::vector<Candlestick> out;
    out.reserve(groups.size());

    for (auto& kv : groups)
    {
        auto& v = kv.second;
        std::sort(v.begin(), v.end(), [](const Order& a, const Order& b){
            return a.timestamp < b.timestamp;
        });

        Candlestick c;
        std::string label = kv.first;
        if (!v.empty() && v.front().timestamp.size() >= 10)
        {
            label = v.front().timestamp.substr(0, 10);
        }
        c.period = label;
        c.open = v.front().price;
        c.close = v.back().price;

        c.high = v.front().price;
        c.low = v.front().price;
        for (const auto& x : v)
        {
            if (x.price > c.high) c.high = x.price;
            if (x.price < c.low) c.low = x.price;
        }
        out.push_back(c);
    }

    return out;
}
