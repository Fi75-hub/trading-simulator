// Lightweight CSV reader used to load the provided market order file (read-only).
#pragma once
#include <string>
#include <vector>
#include "Order.h"

// CSV provides a helper to load market orders from a file.
class CSV
{
public:
    // Reads all valid rows from the market CSV.
    static std::vector<Order> readMarketFile(const std::string& filename);
};
