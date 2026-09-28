// CSV backed storage for users, wallets and transactions.
#include "UserStore.h"
#include "Util.h"
#include <fstream>
#include <sstream>
#include <map>
#include <cmath>
#include <iomanip>
#include <limits>

UserStore::UserStore(std::string usersCsvPath, std::string walletsCsvPath, std::string transactionsCsvPath)
: usersCsv_(std::move(usersCsvPath)), walletsCsv_(std::move(walletsCsvPath)), transactionsCsv_(std::move(transactionsCsvPath))
{
}

// Ensures users/wallets/transactions CSV files exist.
bool UserStore::ensureFiles()
{
    // users.csv
    {
        std::ifstream in(usersCsv_);
        if (!in.good())
        {
            std::ofstream out(usersCsv_);
            if (!out.is_open()) return false;
            out << "username,full_name,email,password_hash\n";
        }
    }

    // wallets.csv
    {
        std::ifstream in(walletsCsv_);
        if (!in.good())
        {
            std::ofstream out(walletsCsv_);
            if (!out.is_open()) return false;
            out << "username,currency,balance\n";
        }
    }

    // transactions.csv
    {
        std::ifstream in(transactionsCsv_);
        if (!in.good())
        {
            std::ofstream out(transactionsCsv_);
            if (!out.is_open()) return false;
            out << "username,timestamp,product,type,amount,price\n";
        }
    }

return true;
}

// Splits a simple CSV line into fields.
std::vector<std::string> UserStore::splitCsvLine(const std::string& line)
{
    // Simple CSV split.
    return util::split(line, ',');
}

// Loads all users from users.csv.
std::vector<User> UserStore::loadAll() const
{
    std::vector<User> users;
    std::ifstream in(usersCsv_);
    if (!in.is_open()) return users;

    auto processLine = [&](const std::string& raw)
    {
        std::string line = util::trim(raw);
        if (line.empty()) return;

        auto parts = splitCsvLine(line);
        if (parts.size() < 4) return;

        User u;
        u.username = util::trim(parts[0]);
        u.fullName = util::trim(parts[1]);
        u.email = util::trim(parts[2]);
        try { u.passwordHash = static_cast<std::size_t>(std::stoull(util::trim(parts[3]))); }
        catch (...) { u.passwordHash = 0; }

        if (!u.username.empty()) users.push_back(u);
    };

    std::string firstLine;
    if (!std::getline(in, firstLine)) return users;

    // Support both headered and headerless CSV files.
    auto firstParts = splitCsvLine(util::trim(firstLine));
    bool isHeader = (!firstParts.empty() && util::toLower(util::trim(firstParts[0])) == "username");
    if (!isHeader) processLine(firstLine);

    std::string line;
    while (std::getline(in, line))
    {
        processLine(line);
    }

    return users;
}


// Checks whether a username already exists (used to avoid ID collisions).
bool UserStore::usernameExists(const std::string& username) const
{
    for (const auto& u : loadAll())
        if (u.username == username) return true;
    return false;
}

// Prevents duplicate accounts by checking full name + email.
bool UserStore::personExists(const std::string& fullName, const std::string& email, User& existing) const
{
    for (const auto& u : loadAll())
    {
        if (u.fullName == fullName && u.email == email)
        {
            existing = u;
            return true;
        }
    }
    return false;
}

// Appends a new user record to users.csv.
bool UserStore::addUser(const User& u)
{
    std::ofstream out(usersCsv_, std::ios::app);
    if (!out.is_open()) return false;

    out << u.username << ","
        << u.fullName << ","
        << u.email << ","
        << static_cast<unsigned long long>(u.passwordHash)
        << "\n";
    return true;
}

// Updates the password hash for a user by rewriting users.csv.
bool UserStore::updatePassword(const std::string& username, std::size_t newHash)
{
    auto users = loadAll();
    bool changed = false;

    for (auto& u : users)
    {
        if (u.username == username)
        {
            u.passwordHash = newHash;
            changed = true;
            break;
        }
    }
    if (!changed) return false;

    std::ofstream out(usersCsv_, std::ios::trunc);
    if (!out.is_open()) return false;

    out << "username,full_name,email,password_hash\n";
    for (const auto& u : users)
    {
        out << u.username << ","
            << u.fullName << ","
            << u.email << ","
            << static_cast<unsigned long long>(u.passwordHash)
            << "\n";
    }
    return true;
}

// Creates a wallet row for a user if one does not already exist.
bool UserStore::createWalletFor(const std::string& username)
{
    // Create a minimal starting wallet entry with zero balance (used in later tasks).
    std::ifstream in(walletsCsv_);
    if (!in.is_open()) return false;

    std::string line;
    std::getline(in, line); // header
    while (std::getline(in, line))
    {
        auto parts = splitCsvLine(line);
        if (parts.size() >= 1 && parts[0] == username) return true;
    }

    std::ofstream out(walletsCsv_, std::ios::app);
    if (!out.is_open()) return false;
    out << username << ",USDT,0\n";
    return true;
}


// Loads wallet balances for a user into a currency->amount map.
std::map<std::string,double> UserStore::loadWallet(const std::string& username) const
{
    std::map<std::string,double> res;

    std::ifstream in(walletsCsv_);
    if (!in.is_open()) return res;

    std::string firstLine;
    if (!std::getline(in, firstLine)) return res;

    // Support both headered and headerless CSV files.
    auto cols = splitCsvLine(util::trim(firstLine));
    bool isHeader = (!cols.empty() && util::toLower(util::trim(cols[0])) == "username");
    bool oldFormat = (!cols.empty() && cols.size() == 2); // username,balance (legacy)

    auto processParts = [&](const std::vector<std::string>& parts)
    {
        if (oldFormat)
        {
            if (parts.size() >= 2 && parts[0] == username)
            {
                // Legacy format: treat as USDT balance only
                try { res["USDT"] = std::stod(parts[1]); } catch (...) {}
            }
        }
        else
        {
            if (parts.size() >= 3 && parts[0] == username)
            {
                try { res[parts[1]] = std::stod(parts[2]); } catch (...) {}
            }
        }
    };

    if (!isHeader) processParts(cols);

    std::string line;
    while (std::getline(in, line))
    {
        line = util::trim(line);
        if (line.empty()) continue;
        auto parts = splitCsvLine(line);
        processParts(parts);
    }

    return res;
}


// Applies a deposit/withdraw delta with negative balance protection, then writes the updated wallet file.
bool UserStore::adjustWallet(const std::string& username, const std::string& currency, double delta, double& newBalance)
{
    if (!std::isfinite(delta)) return false;
    // Load all rows and rewrite the file.
    struct Row { std::string u; std::string c; double b; };
    std::vector<Row> rows;

    std::ifstream in(walletsCsv_);
    if (!in.is_open()) return false;

    std::string header;
    std::getline(in, header);

    bool oldFormat = false;
    {
        auto cols = splitCsvLine(header);
        oldFormat = (cols.size() == 2);
    }

    std::string line;
    while (std::getline(in, line))
    {
        auto parts = splitCsvLine(line);
        if (parts.empty()) continue;

        if (oldFormat)
        {
            if (parts.size() >= 2)
            {
                double b = 0;
                try { b = std::stod(parts[1]); } catch (...) {}
                rows.push_back({parts[0], "USDT", b});
            }
        }
        else
        {
            if (parts.size() >= 3)
            {
                double b = 0;
                try { b = std::stod(parts[2]); } catch (...) {}
                rows.push_back({parts[0], parts[1], b});
            }
        }
    }
    in.close();

    // Find or create entry
    bool found = false;
    for (auto& r : rows)
    {
        if (r.u == username && r.c == currency)
        {
            if (!std::isfinite(r.b + delta) || r.b + delta < 0) return false;
            r.b += delta;
            newBalance = r.b;
            found = true;
            break;
        }
    }
    if (!found)
    {
        if (delta < 0) return false;
        rows.push_back({username, currency, delta});
        newBalance = delta;
    }

    std::ofstream out(walletsCsv_);
    if (!out.is_open()) return false;
    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    out << "username,currency,balance\n";
    for (const auto& r : rows)
    {
        out << r.u << "," << r.c << "," << r.b << "\n";
    }
    return true;
}

// Appends a transaction row to transactions.csv.
bool UserStore::appendTransaction(const std::string& username, const Transaction& t)
{
    std::ofstream out(transactionsCsv_, std::ios::app);
    if (!out.is_open()) return false;

    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    out << username << ","
        << t.timestamp << ","
        << t.product << ","
        << t.type << ","
        << t.amount << ","
        << t.price << "\n";
    return true;
}

// Loads all transactions for a given user from transactions.csv.
std::vector<Transaction> UserStore::loadTransactions(const std::string& username) const
{
    std::vector<Transaction> res;
    std::ifstream in(transactionsCsv_);
    if (!in.is_open()) return res;

    auto processLine = [&](const std::string& raw)
    {
        std::string line = util::trim(raw);
        if (line.empty()) return;

        auto p = splitCsvLine(line);
        if (p.size() < 6) return;
        if (p[0] != username) return;

        Transaction t;
        t.timestamp = p[1];
        t.product = p[2];
        t.type = p[3];
        try { t.amount = std::stod(p[4]); } catch (...) { t.amount = 0; }
        try { t.price = std::stod(p[5]); } catch (...) { t.price = 0; }
        res.push_back(t);
    };

    std::string firstLine;
    if (!std::getline(in, firstLine)) return res;

    // Support both headered and headerless CSV files.
    auto firstParts = splitCsvLine(util::trim(firstLine));
    bool isHeader = (!firstParts.empty() && util::toLower(util::trim(firstParts[0])) == "username");
    if (!isHeader) processLine(firstLine);

    std::string line;
    while (std::getline(in, line))
    {
        processLine(line);
    }

    return res;
}
