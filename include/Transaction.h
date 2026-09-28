// Represents a user transaction stored in transactions.csv (Tasks 3 and 4).
#pragma once
#include <string>

// Transaction is appended to transactions.csv to keep an audit trail of user actions.
struct Transaction
{
    std::string timestamp;   // YYYY-MM-DD HH:MM:SS
    std::string product;     // currency or pair
    std::string type;        // deposit, withdraw, ask, bid
    double amount{};
    double price{};          // for deposit/withdraw can be 1.0
};
