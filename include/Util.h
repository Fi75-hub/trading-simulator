// Small helper functions: input validation, string helpers, username generation, hashing, timestamps.
#pragma once
#include <string>
#include <vector>

// Utility functions used across the menus and storage code.
namespace util
{

    // String helpers
    std::string trim(const std::string& s);
    std::vector<std::string> split(const std::string& s, char delim);


    // Console input helpers (Task 5)
    std::string readLine(const std::string& prompt);
    int readIntInRange(const std::string& prompt, int minValue, int maxValue);
    std::string readNonEmpty(const std::string& prompt);


    // Account helpers (Task 2)
    std::string generateUsername10();
    std::size_t passwordHash(const std::string& password);


    // Misc helpers
    std::string toLower(std::string s);
    double readDoublePositive(const std::string& prompt);
    std::string nowTimestamp();
}
