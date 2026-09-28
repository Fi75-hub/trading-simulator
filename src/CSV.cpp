// Reads the provided market CSV file and converts each valid row into an Order struct.
#include "CSV.h"
#include "Util.h"
#include <fstream>
#include <stdexcept>

static OrderSide parseSide(const std::string& s)
{
    std::string t = util::toLower(s);
    if (t == "ask") return OrderSide::Ask;
    if (t == "bid") return OrderSide::Bid;
    throw std::runtime_error("Unknown order side: " + s);
}


// Reads the market CSV file and parses each valid row into an Order object.
std::vector<Order> CSV::readMarketFile(const std::string& filename)
{
    std::ifstream in(filename);
    if (!in.is_open())
        throw std::runtime_error("Could not open file: " + filename);

    std::vector<Order> out;
    std::string line;

    while (std::getline(in, line))
    {
        line = util::trim(line);
        if (line.empty()) continue;

        auto parts = util::split(line, ',');
        if (parts.size() < 5) continue;

        try
        {
            Order o;
            o.timestamp = parts[0];
            o.product = parts[1];
            o.side = parseSide(parts[2]);
            o.price = std::stod(parts[3]);
            o.amount = std::stod(parts[4]);
            out.push_back(o);
        }
        catch (...)
        {
            // Skip malformed rows instead of terminating.
            continue;
        }
    }

    return out;
}
