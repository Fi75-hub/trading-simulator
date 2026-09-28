// Registered user record stored in users.csv (Task 2).
#pragma once
#include <string>

// User represents an account stored in users.csv.
struct User
{
    std::string username;     // 10 digits
    std::string fullName;
    std::string email;
    std::size_t passwordHash{};
};
