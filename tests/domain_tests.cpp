#include "AuthService.h"
#include "UserStore.h"
#include "MarketData.h"
#include "Util.h"
#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>

struct Input {
    std::istringstream source;
    std::streambuf* previous;
    explicit Input(const std::string& value) : source(value), previous(std::cin.rdbuf(source.rdbuf())) { std::cin.clear(); }
    ~Input() { std::cin.rdbuf(previous); std::cin.clear(); }
};

int main()
{
    const auto temp = std::filesystem::temp_directory_path() / ("trading-tests-" + util::generateUsername10());
    std::filesystem::create_directory(temp);
    UserStore store((temp / "users.csv").string(), (temp / "wallets.csv").string(), (temp / "transactions.csv").string());
    assert(store.ensureFiles());
    User user{"1234567890", "Test User", "user@example.invalid", util::passwordHash("test-password")};
    assert(store.addUser(user));
    assert(store.createWalletFor(user.username));
    double balance = 0;
    const double precise = 0.123456789012345;
    assert(store.adjustWallet(user.username, "BTC", precise, balance));
    assert(store.loadWallet(user.username).at("BTC") == precise);
    assert(!store.adjustWallet(user.username, "BTC", -1, balance));
    assert(!store.adjustWallet(user.username, "BTC", std::numeric_limits<double>::infinity(), balance));
    assert(!store.adjustWallet(user.username, "BTC", std::numeric_limits<double>::quiet_NaN(), balance));
    assert(store.loadWallet(user.username).at("BTC") == precise);
    Transaction tx{"2030/01/01 00:00:00", "BTC/USDT", "bid", precise, 12345.6789012345};
    assert(store.appendTransaction(user.username, tx));
    const auto saved = store.loadTransactions(user.username);
    assert(saved.size() == 1 && saved[0].amount == tx.amount && saved[0].price == tx.price);

    AuthService auth(store);
    User loggedIn;
    { Input input("1234567890\nwrong\n"); assert(!auth.login(loggedIn)); }
    { Input input("1234567890\ntest-password\n"); assert(auth.login(loggedIn)); }
    { Input input("1234567890\nwrong\n"); assert(!auth.resetPassword()); }
    { Input input("1234567890\ntest-password\nchanged-password\n"); assert(auth.resetPassword()); }
    { Input input("1234567890\nchanged-password\n"); assert(auth.login(loggedIn)); }
    { Input input("Bad,Name\nother@example.invalid\npassword\n"); assert(!auth.registerUser(loggedIn)); }
    assert(store.loadAll().size() == 1);

    { Input input("10oops\nnan\ninf\n-2\n0\n1.25\n"); assert(util::readDoublePositive("") == 1.25); }
    { Input input(""); bool closed = false; try { util::readDoublePositive(""); } catch (const std::ios_base::failure&) { closed = true; } assert(closed); }
    { Input input("2junk\n99\n2\n"); assert(util::readIntInRange("", 1, 3) == 2); }

    const auto csv = temp / "market.csv";
    {
        std::ofstream out(csv);
        out << "2030/01/01 00:00:00,BTC/USDT,ask,10,1\n"
               "2030/01/01 00:00:00,BTC/USDT,ask,15,1\n"
               "2030/01/02 00:00:00,BTC/USDT,ask,5,1\n"
               "2030/01/02 00:00:00,BTC/USDT,bid,6,1\n"
               "2030/01/02 00:00:00,BTC/USDT,ask,nan,1\n"
               "2030/01/02 00:00:00,BTC/USDT,ask,5junk,1\n";
    }
    MarketData market;
    std::string error;
    assert(market.load(csv.string(), error));
    const auto daily = market.candlesticks("BTC/USDT", OrderSide::Ask, Timeframe::Daily);
    assert(daily.size() == 2 && daily[0].period == "2030/01/01");
    assert(daily[0].open == 10 && daily[0].close == 15 && daily[0].high == 15 && daily[0].low == 10);
    const auto monthly = market.candlesticks("BTC/USDT", OrderSide::Ask, Timeframe::Monthly);
    assert(monthly.size() == 1 && monthly[0].period == "2030/01");
    assert(monthly[0].open == 10 && monthly[0].close == 5 && monthly[0].high == 15 && monthly[0].low == 5);
    assert(market.candlesticks("BTC/USDT", OrderSide::Ask, Timeframe::Yearly)[0].period == "2030");
    assert(!market.load((temp / "missing.csv").string(), error));
    std::ofstream(temp / "empty.csv");
    assert(!market.load((temp / "empty.csv").string(), error));
    std::filesystem::remove_all(temp);
    std::cout << "Domain tests passed.\n";
}
