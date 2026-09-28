// Console menus and task coordination for Tasks 1 to 5.
#include "App.h"
#include "Util.h"
#include "Transaction.h"
#include <iomanip>
#include <cctype>
#include <iostream>
#include <random>
#include <chrono>
#include <stdexcept>
#include <algorithm>

App::App()
: store_("data/users.csv", "data/wallets.csv", "data/transactions.csv")
, auth_(store_)
{
    if (!store_.ensureFiles()) throw std::runtime_error("Could not create local account files. Run from a writable repository root.");
}

// Prompts for a market CSV path until it loads successfully.
bool App::ensureMarketLoaded()
{
    const std::string defaultPath = "data/20200601.csv";
    std::string defaultError;
    if (market_.load(defaultPath, defaultError))
    {
        std::cout << "Loaded market file: " << defaultPath << "\n\n";
        return true;
    }

    for (;;)
    {
        std::string path = util::readLine("Market CSV (Enter for default: data/20200601.csv): ");
        if (path.empty()) path = "data/20200601.csv";

        std::string error;
        if (market_.load(path, error))
        {
            std::cout << "Loaded market file: " << path << "\n\n";
            return true;
        }
        std::cout << "Error: " << error << "\n";
    }
}

// Handles the authentication menu (register/login/reset) and returns the logged in user.
User App::authenticate()
{
    for (;;)
    {
        std::cout << "1) Login\n2) Register\n3) Change password\n4) Exit\n";
        int c = util::readIntInRange("Choose: ", 1, 4);

        User u;
        if (c == 1)
        {
            if (auth_.login(u)) return u;
        }
        else if (c == 2)
        {
            auth_.registerUser(u);
            // After registering, user can login using the shown username
        }
        else if (c == 3)
        {
            auth_.resetPassword();
        }
        else
        {
            return User{};
        }
    }
}

// Main program loop: load market data, then authenticate and show menus until exit.
void App::run()
{
    if (!ensureMarketLoaded()) return;

    for (;;)
    {
        User user = authenticate();
        if (user.username.empty())
            break;

        menuMain(user);
    }

    std::cout << "Goodbye.\n";
}


// Logged in menu router for Task 1, Task 3 and Task 4 features.
void App::menuMain(const User& user)
{
    for (;;)
    {
        std::cout << "Logged in as: " << user.username << " (" << user.fullName << ")\n";
        std::cout << "1) Print help\n";
        std::cout << "2) Print wallet\n";
        std::cout << "3) Candlestick summary (Task 1)\n";
        std::cout << "4) Deposit/Withdraw (Task 3)\n";
        std::cout << "5) View recent transactions (Task 3)\n";
        std::cout << "6) Trading statistics (Task 3)\n";
        std::cout << "7) Simulate trading (Task 4)\n";
        std::cout << "8) Exit\n";
        std::cout << "0) Logout\n";

        std::cout << "Current time is: " << util::nowTimestamp() << "\n";

        int c = util::readIntInRange("Choose: ", 0, 8);

        if (c == 0) { std::cout << "\n"; return; }
        else if (c == 1) printHelp();
        else if (c == 2) showWallet(user);
        else if (c == 3) menuCandlesticks();
        else if (c == 4) menuFunds(user);
        else if (c == 5) showRecentTransactions(user);
        else if (c == 6) showTradingStats(user);
        else if (c == 7) simulateTrading(user);
        else if (c == 8) { std::exit(0); }
    }
}


void App::printHelp()
{
    std::cout << "\n--- Help ---\n";
    std::cout << "Use the numbered menu options to view analysis and manage your account.\n\n";
    std::cout << "Candlestick summary (Task 1):\n";
    std::cout << "- Pick a product pair (example: ETH/USDT)\n";
    std::cout << "- Choose a timeframe: Daily / Monthly / Yearly (Yearly is the default)\n";
    std::cout << "- Choose whether to view ASK, BID or BOTH\n\n";
    std::cout << "Deposit/Withdraw (Task 3):\n";
    std::cout << "- Deposit adds funds to a currency (example: USDT)\n";
    std::cout << "- Withdraw removes funds (you cannot go below zero)\n\n";
    std::cout << "Recent transactions (Task 3):\n";
    std::cout << "- Show the last 5 actions, or filter by a product/currency\n\n";
    std::cout << "Trading statistics (Task 3):\n";
    std::cout << "- Shows ask/bid counts and spending totals\n";
    std::cout << "- Timeframe prefixes use: YYYY or YYYY/MM or YYYY/MM/DD\n\n";
    std::cout << "Simulate trading (Task 4):\n";
    std::cout << "- Generates 5 bids and 5 asks for each product using the current timestamp\n\n";
    std::cout << "Logout:\n";
    std::cout << "- Type 0 to logout and return to the login menu\n\n";
}


// Reads a timeframe choice.
Timeframe App::pickTimeframe()
{
    std::cout << "Timeframe:\n1) Daily\n2) Monthly\n3) Yearly (default)\n";
    for (;;)
    {
        std::string s = util::readLine("Choose (Enter=3): ");
        if (s.empty()) return Timeframe::Yearly;

        try
        {
            int c = std::stoi(s);
            if (c == 1) return Timeframe::Daily;
            if (c == 2) return Timeframe::Monthly;
            if (c == 3) return Timeframe::Yearly;
        }
        catch (...) {}

        std::cout << "Invalid input. Choose 1-3.\n";
    }
}


// Prints a compact OHLC table for a set of candlesticks.
void App::printCandles(const std::string& title, const std::vector<Candlestick>& v)
{
    std::cout << "\n" << title << "\n";

    if (v.empty())
    {
        std::cout << "(no data)\n\n";
        return;
    }

    // Column widths
    const int noW = 3;
    int periodW = 6;
    for (const auto& c : v)
    {
        if ((int)c.period.size() > periodW) periodW = (int)c.period.size();
    }
    if (periodW > 16) periodW = 16;

    const int numW = 14;

    auto hr = [&]()
    {
        std::cout << "+"
                  << std::string(noW + 2, '-') << "+"
                  << std::string(periodW + 2, '-') << "+"
                  << std::string(numW + 2, '-') << "+"
                  << std::string(numW + 2, '-') << "+"
                  << std::string(numW + 2, '-') << "+"
                  << std::string(numW + 2, '-') << "+\n";
    };

    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(8);

    hr();
    std::cout << "| " << std::right << std::setw(noW) << "No"
              << " | " << std::left  << std::setw(periodW) << "Period"
              << " | " << std::right << std::setw(numW) << "Open"
              << " | " << std::right << std::setw(numW) << "High"
              << " | " << std::right << std::setw(numW) << "Low"
              << " | " << std::right << std::setw(numW) << "Close"
              << " |\n";
    hr();

    int row = 1;
    for (const auto& c : v)
    {
        std::string period = c.period;
        if ((int)period.size() > periodW) period = period.substr(0, periodW);

        std::cout << "| " << std::right << std::setw(noW) << row++
                  << " | " << std::left  << std::setw(periodW) << period
                  << " | " << std::right << std::setw(numW) << c.open
                  << " | " << std::right << std::setw(numW) << c.high
                  << " | " << std::right << std::setw(numW) << c.low
                  << " | " << std::right << std::setw(numW) << c.close
                  << " |\n";
    }

    hr();
    std::cout << "\n";
}



// Task 1: asks for product/timeframe/side and displays candlestick summaries.
void App::menuCandlesticks()
{
    const auto& products = market_.products();
    if (products.empty())
    {
        std::cout << "No products found in market file.\n";
        return;
    }

    std::cout << "\n--- Candlestick Summary (Task 1) ---\n";
    std::cout << "1) Choose product from list\n2) Enter product manually\n3) Back\n";
    int mode = util::readIntInRange("Choose: ", 1, 3);
    if (mode == 3) { std::cout << "\n"; return; }

    std::string product;
    if (mode == 1)
    {
        std::cout << "\nSelect product:\n";
        for (std::size_t i = 0; i < products.size(); ++i)
            std::cout << (i + 1) << ") " << products[i] << "\n";

        int idx = util::readIntInRange("Choose: ", 1, static_cast<int>(products.size()));
        product = products[static_cast<std::size_t>(idx - 1)];
    }
    else
    {
        product = util::readNonEmpty("Enter product (e.g., ETH/USDT): ");

        // Normalise and validate.
        std::string want = util::toLower(product);
        bool ok = false;
        for (const auto& p : products)
        {
            if (util::toLower(p) == want)
            {
                product = p;
                ok = true;
                break;
            }
        }
        if (!ok)
        {
            std::cout << "Unknown product. Please enter one of the available pairs.\n\n";
            return;
        }
    }

    // By default the yearly summary is shown.
    Timeframe tf = pickTimeframe();

    std::cout << "Order side:\n1) Both\n2) Ask only\n3) Bid only\n";
    int sideChoice = util::readIntInRange("Choose: ", 1, 3);

    if (sideChoice == 1 || sideChoice == 2)
    {
        auto ask = market_.candlesticks(product, OrderSide::Ask, tf);
        printCandles("ASK candlesticks for " + product, ask);
    }

    if (sideChoice == 1 || sideChoice == 3)
    {
        auto bid = market_.candlesticks(product, OrderSide::Bid, tf);
        printCandles("BID candlesticks for " + product, bid);
    }
}



// Task 3: deposit/withdraw/view wallet with validation and transaction logging.
void App::menuFunds(const User& user)
{
    for (;;)
    {
        std::cout << "\n--- Manage Funds (Task 3) ---\n";
        std::cout << "1) Deposit\n2) Withdraw\n3) View wallet\n4) Back\n";
        int c = util::readIntInRange("Choose: ", 1, 4);

        if (c == 4) { std::cout << "\n"; return; }
        if (c == 3) { showWallet(user); continue; }

        std::string cur = util::readNonEmpty("Currency (e.g., USDT, BTC): ");
        for (char& ch : cur) ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));

        if (cur.size() > 12 || !std::all_of(cur.begin(), cur.end(), [](unsigned char ch) { return std::isalnum(ch); }))
        {
            std::cout << "Use a currency code of up to 12 letters or digits.\n";
            continue;
        }
        double amt = util::readDoublePositive("Amount: ");
        double delta = (c == 1) ? amt : -amt;

        double newBal{};
        if (!store_.adjustWallet(user.username, cur, delta, newBal))
        {
            std::cout << "Not enough balance to withdraw that amount.\n\n";
            continue;
        }

        Transaction t;
        t.timestamp = util::nowTimestamp();
        t.product = cur;
        t.type = (c == 1) ? "deposit" : "withdraw";
        t.amount = amt;
        t.price = 1.0;

        store_.appendTransaction(user.username, t);

        std::cout << "Updated balance: " << cur << " = " << newBal << "\n";

        if (c == 1) std::cout << "Amount deposited successfully.\n\n";
        else std::cout << "Withdraw was successful.\n\n";
    }
}



// Task 3: displays wallet balances for the current user.
void App::showWallet(const User& user)
{
    auto wallet = store_.loadWallet(user.username);

    std::cout << "\n--- Wallet ---\n";
    std::cout << "User: " << user.fullName << "\n";
    if (wallet.empty())
    {
        std::cout << "(wallet is empty)\n\n";
        return;
    }

    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(8);

    for (const auto& kv : wallet)
    {
        std::cout << kv.first << " : " << kv.second << "\n";
    }
    std::cout << "\n";
}

// Task 3: shows recent transactions (last 5) and optional product filtering.
void App::showRecentTransactions(const User& user)
{
    auto tx = store_.loadTransactions(user.username);

    std::cout << "\n--- Recent Transactions (Task 3) ---\n";
    if (tx.empty())
    {
        std::cout << "(no transactions yet)\n\n";
        return;
    }

    std::cout << "1) Show last 5\n2) Filter by product/currency\n3) Back\n";
    int c = util::readIntInRange("Choose: ", 1, 3);
    if (c == 3) { std::cout << "\n"; return; }

    std::string filter;
    if (c == 2)
    {
        filter = util::readNonEmpty("Enter product/currency (e.g., USDT or ETH/BTC): ");
    }

    std::vector<Transaction> out;
    if (c == 1)
    {
        int start = (int)tx.size() - 5;
        if (start < 0) start = 0;
        for (int i = start; i < (int)tx.size(); ++i) out.push_back(tx[i]);
    }
    else
    {
        for (const auto& t : tx)
        {
            if (t.product == filter) out.push_back(t);
        }
    }

    if (out.empty())
    {
        std::cout << "(no matching transactions)\n\n";
        return;
    }

    std::cout << "Time                 | Type      | Product     | Amount        | Price\n";
    std::cout << "----------------------------------------------------------------------------\n";
    std::cout.setf(std::ios::fixed);
    std::cout << std::setprecision(8);

    for (const auto& t : out)
    {
        std::cout << std::left  << std::setw(19) << t.timestamp
                  << " | " << std::left  << std::setw(9)  << t.type
                  << " | " << std::left  << std::setw(11) << t.product
                  << " | " << std::right << std::setw(12) << t.amount
                  << " | " << std::right << std::setw(11) << t.price
                  << "\n";
    }
    std::cout << "\n";
}

static bool tsStartsWith(const std::string& ts, const std::string& prefix)
{
    return ts.size() >= prefix.size() && ts.compare(0, prefix.size(), prefix) == 0;
}

// Task 3: prints summary stats, including totals and timeframe based spending.
void App::showTradingStats(const User& user)
{
    auto tx = store_.loadTransactions(user.username);

    std::cout << "\n--- Trading Statistics (Task 3) ---\n";
    if (tx.empty())
    {
        std::cout << "(no transactions yet)\n\n";
        return;
    }

    std::cout << "1) All products\n2) Specific product\n3) Back\n";
    int mode = util::readIntInRange("Choose: ", 1, 3);
    if (mode == 3) { std::cout << "\n"; return; }

    std::string productFilter;
    if (mode == 2)
    {
        productFilter = util::readNonEmpty("Enter product (e.g., BTC/USDT): ");
        productFilter = util::trim(productFilter);
    }

    auto matchesProduct = [&](const Transaction& t)
    {
        if (productFilter.empty()) return true;
        // Compare case insensitive and normalise.
        std::string a = util::toLower(t.product);
        std::string b = util::toLower(productFilter);
        return a == b;
    };

    int asks = 0, bids = 0;
    double spentAll = 0;

    for (const auto& t : tx)
    {
        if (!matchesProduct(t)) continue;

        if (t.type == "ask") asks++;
        if (t.type == "bid")
        {
            bids++;
            spentAll += (t.amount * t.price);
        }
    }

    std::cout << "Asks: " << asks << "\n";
    std::cout << "Bids: " << bids << "\n";
    std::cout << "Total spent (all-time bids): " << spentAll << "\n\n";

    std::cout << "Total spent in timeframe (bids only)\n";
    std::cout << "1) Daily\n2) Monthly\n3) Yearly (default)\n4) Skip\n";

    // Allow Enter as default yearly by reading a line.
    int choice = 0;
    for (;;)
    {
        std::string s = util::readLine("Choose (Enter=3): ");
        if (s.empty()) { choice = 3; break; }
        try { choice = std::stoi(s); } catch (...) { choice = 0; }
        if (choice >= 1 && choice <= 4) break;
        std::cout << "Invalid input. Choose 1-4.\n";
    }

    if (choice == 4) { std::cout << "\n"; return; }

    std::string prefix;
    if (choice == 1) prefix = util::readNonEmpty("Enter day (YYYY/MM/DD): ");
    else if (choice == 2) prefix = util::readNonEmpty("Enter month (YYYY/MM): ");
    else prefix = util::readNonEmpty("Enter year (YYYY): ");

    // Normalise prefix.
    for (char& ch : prefix) if (ch == '-') ch = '/';

    double spent = 0;
    int bidsInRange = 0;

    for (const auto& t : tx)
    {
        if (!matchesProduct(t)) continue;
        if (t.type != "bid") continue;

        std::string ts = t.timestamp;
        for (char& ch : ts) if (ch == '-') ch = '/';

        if (tsStartsWith(ts, prefix))
        {
            bidsInRange++;
            spent += (t.amount * t.price);
        }
    }

    std::cout << "Bids in range: " << bidsInRange << "\n";
    std::cout << "Total spent in range: " << spent << "\n\n";
}



static bool splitPair(const std::string& product, std::string& base, std::string& quote)
{
    auto pos = product.find('/');
    if (pos == std::string::npos) return false;
    base = product.substr(0, pos);
    quote = product.substr(pos + 1);
    return !base.empty() && !quote.empty();
}

static double suggestedAmount(double refPrice)
{
    if (refPrice >= 10000.0) return 0.0001;
    if (refPrice >= 1000.0)  return 0.001;
    if (refPrice >= 100.0)   return 0.01;
    if (refPrice >= 1.0)     return 0.1;
    return 1.0;
}

// Task 4: generates sample asks/bids (5 each per product) using the current system timestamp.
void App::simulateTrading(const User& user)
{
    std::cout << "\n--- Simulate Trading (Task 4) ---\n";
    std::cout << "This will create 5 bids and 5 asks for each available product.\n";
    std::cout << "Timestamps are based on the current system time.\n";
    std::cout << "Note: Market data is historical; simulated orders use today's timestamp.\n\n";

    const auto& prods = market_.products();
    if (prods.empty())
    {
        std::cout << "No products loaded.\n\n";
        return;
    }

    market_.clearSimulated();

    // Small random jitter to avoid identical prices.
    std::mt19937 rng(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<double> jitter(-0.0015, 0.0015); // +/- 0.15%

    int createdBids = 0;
    int createdAsks = 0;

    for (const auto& product : prods)
    {
        std::string base, quote;
        if (!splitPair(product, base, quote))
            continue;

        double ref = market_.referencePrice(product);
        if (ref <= 0.0)
            continue;

        double amt = suggestedAmount(ref);

        std::cout << "Product: " << product << " (ref=" << std::fixed << std::setprecision(8) << ref << ")\n";

        // First create bids (buy base with quote), then asks (sell base for quote).
        for (int i = 0; i < 5; ++i)
        {
            double pct = 0.006 + (0.001 * i) + jitter(rng); // ~0.6% to 1.0%
            double price = ref * (1.0 - pct);
            if (price <= 0.0) price = ref;

            double cost = price * amt;

            // Ensure enough quote funds.
            auto wallet = store_.loadWallet(user.username);
            double haveQuote = 0.0;
            auto itQ = wallet.find(quote);
            if (itQ != wallet.end()) haveQuote = itQ->second;

            if (haveQuote + 1e-12 < cost)
            {
                double add = (cost - haveQuote) + (0.10 * cost);
                double nb{};
                store_.adjustWallet(user.username, quote, add, nb);

                Transaction dep;
                dep.timestamp = util::nowTimestamp();
                dep.product = quote;
                dep.type = "deposit";
                dep.amount = add;
                dep.price = 1.0;
                store_.appendTransaction(user.username, dep);
            }

            // Execute buy: quote decreases, base increases.
            double nb1{}, nb2{};
            store_.adjustWallet(user.username, quote, -cost, nb1);
            store_.adjustWallet(user.username, base, +amt, nb2);

            Transaction t;
            t.timestamp = util::nowTimestamp();
            t.product = product;
            t.type = "bid";
            t.amount = amt;
            t.price = price;
            store_.appendTransaction(user.username, t);

            Order o;
            o.timestamp = t.timestamp;
            o.product = product;
            o.side = OrderSide::Bid;
            o.price = price;
            o.amount = amt;
            market_.addSimulatedOrder(o);

            createdBids++;
            std::cout << "[SIM] Generated Bid: " << product << " | Price: " << price << " | Time: " << t.timestamp << "\n";
        }

        for (int i = 0; i < 5; ++i)
        {
            double pct = 0.006 + (0.001 * i) + jitter(rng); // ~0.6% to 1.0%
            double price = ref * (1.0 + pct);
            if (price <= 0.0) price = ref;

            double proceeds = price * amt;

            // Ensure enough base to sell.
            auto wallet = store_.loadWallet(user.username);
            double haveBase = 0.0;
            auto itB = wallet.find(base);
            if (itB != wallet.end()) haveBase = itB->second;

            if (haveBase + 1e-12 < amt)
            {
                double add = (amt - haveBase) + (0.10 * amt);
                double nb{};
                store_.adjustWallet(user.username, base, add, nb);

                Transaction dep;
                dep.timestamp = util::nowTimestamp();
                dep.product = base;
                dep.type = "deposit";
                dep.amount = add;
                dep.price = 1.0;
                store_.appendTransaction(user.username, dep);
            }

            // Execute sell: base decreases, quote increases.
            double nb1{}, nb2{};
            store_.adjustWallet(user.username, base, -amt, nb1);
            store_.adjustWallet(user.username, quote, +proceeds, nb2);

            Transaction t;
            t.timestamp = util::nowTimestamp();
            t.product = product;
            t.type = "ask";
            t.amount = amt;
            t.price = price;
            store_.appendTransaction(user.username, t);


            Order o;
            o.timestamp = t.timestamp;
            o.product = product;
            o.side = OrderSide::Ask;
            o.price = price;
            o.amount = amt;
            market_.addSimulatedOrder(o);

            createdAsks++;
            std::cout << "[SIM] Generated Ask: " << product << " | Price: " << price << " | Time: " << t.timestamp << "\n";
        }

        std::cout << "\n";
    }

    std::cout << "Simulation complete.\n";
    std::cout << "Created bids: " << createdBids << "\n";
    std::cout << "Created asks: " << createdAsks << "\n\n";
}
