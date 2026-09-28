// Main application controller. Loads market data, handles authentication and runs the task menus.
#pragma once
#include <string>
#include "MarketData.h"
#include "UserStore.h"
#include "AuthService.h"

// App ties together market data, user storage and authentication.
class App
{
public:
    // Sets up services and prepares storage files.
    App();
    // Starts the main program loop.
    void run();

private:
    // Reads only market orders loaded from the provided CSV.
    MarketData market_;
    // Separate user data files (users, wallets, transactions).
    UserStore store_;
    // Task 2 authentication helper.
    AuthService auth_;

    // Prompts until a market file loads successfully.
    bool ensureMarketLoaded();
    // Register/login/reset flow; returns the logged-in user.
    User authenticate();

    // Main menu after login.
    void menuMain(const User& user);
    // Prints brief usage guidance for the logged-in menu.
    void printHelp();
    // Task 1: candlestick summaries.
    void menuCandlesticks();
    // Task 3: wallet and transaction options.
    void menuFunds(const User& user);
    // Task 3: show current wallet balances.
    void showWallet(const User& user);
    // Task 3: show recent transactions (and filtering).
    void showRecentTransactions(const User& user);
    // Task 3: statistics and timeframe spending.
    void showTradingStats(const User& user);
    // Task 4: generate sample asks/bids using the current timestamp.
    void simulateTrading(const User& user);

    // Reads Daily/Monthly/Yearly.
    static Timeframe pickTimeframe();
    // Prints a compact OHLC table.
    static void printCandles(const std::string& title, const std::vector<Candlestick>& v);
};
