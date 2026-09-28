// Shared utilities for input validation, parsing, hashing and timestamps.
#include "Util.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iostream>
#include <limits>
#include <random>

namespace util
{
    // Trims leading and trailing whitespace.
    std::string trim(const std::string& s)
    {
        std::size_t a = 0;
        while (a < s.size() && std::isspace(static_cast<unsigned char>(s[a]))) ++a;

        std::size_t b = s.size();
        while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;

        return s.substr(a, b - a);
    }

    // Splits a string by a delimiter character.
    std::vector<std::string> split(const std::string& s, char delim)
    {
        std::vector<std::string> out;
        std::string cur;
        for (char ch : s)
        {
            if (ch == delim)
            {
                out.push_back(cur);
                cur.clear();
            }
            else cur.push_back(ch);
        }
        out.push_back(cur);
        return out;
    }

    // Prompts the user and reads a full line from stdin.
    std::string readLine(const std::string& prompt)
    {
        std::cout << prompt;
        std::string line;
        std::getline(std::cin, line);
        return trim(line);
    }

    // Prompts until a non empty string is entered.
    std::string readNonEmpty(const std::string& prompt)
    {
        for (;;)
        {
            std::string s = readLine(prompt);
            if (!s.empty()) return s;
            std::cout << "Please enter a value.\n";
        }
    }

    // Prompts until a valid integer within a range is entered (prevents input crashes).
    int readIntInRange(const std::string& prompt, int minValue, int maxValue)
    {
        for (;;)
        {
            std::cout << prompt;

            std::string line;
            if (!std::getline(std::cin >> std::ws, line))
            {
                std::cin.clear();
                std::cout << "Invalid input. Try again.\n";
                continue;
            }

            line = trim(line);
            if (line.empty())
            {
                std::cout << "Invalid input. Try again.\n";
                continue;
            }

            try
            {
                std::size_t idx = 0;
                int value = std::stoi(line, &idx);

                if (idx != line.size())
                {
                    std::cout << "Invalid input. Try again.\n";
                    continue;
                }

                if (value >= minValue && value <= maxValue) return value;
                std::cout << "Choose a number between " << minValue << " and " << maxValue << ".\n";
            }
            catch (...)
            {
                std::cout << "Invalid input. Try again.\n";
            }
        }
    }

    // Prompts until a valid positive number is entered.
    double readDoublePositive(const std::string& prompt)
    {
        for (;;)
        {
            std::cout << prompt;
            double v{};
            if (std::cin >> v)
            {
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                if (v > 0) return v;
                std::cout << "Enter a positive amount.\n";
            }
            else
            {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid number. Try again.\n";
            }
        }
    }

    // Generates a random 10 digit username string.
    std::string generateUsername10()
    {
        static std::mt19937_64 rng{ std::random_device{}() };
        std::uniform_int_distribution<unsigned long long> dist(1000000000ULL, 9999999999ULL);
        return std::to_string(dist(rng));
    }

    // Returns a hashed password value.
    std::size_t passwordHash(const std::string& password)
    {
        const std::string salt = "CM2005";
        return std::hash<std::string>{}(salt + password);
    }

    // Converts a string to lower-case for case-insensitive matching.
    std::string toLower(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return s;
    }

    // Returns the current local timestamp in a consistent format.
    std::string nowTimestamp()
    {
        using namespace std::chrono;
        auto now = system_clock::now();
        std::time_t t = system_clock::to_time_t(now);

        std::tm tm{};
#if defined(_WIN32)
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif

        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y/%m/%d %H:%M:%S", &tm);
        return std::string(buf);
    }
}
