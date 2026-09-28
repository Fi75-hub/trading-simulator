// Represents one row from the market order CSV (timestamp, product, side, price, amount).
#pragma once
#include <string>

// OrderSide indicates whether the order is an ask or a bid.
enum class OrderSide { Ask, Bid };

// Order represents one row from the market CSV.
struct Order
{
    // Original timestamp from the market file
    std::string timestamp;
    // Product pair, e.g. ETH/USDT
    std::string product;
    // Ask or Bid
    OrderSide side;
    // Unit price
    double price;
    // Quantity
    double amount;
};
