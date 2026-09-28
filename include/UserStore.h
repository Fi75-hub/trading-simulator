// File backed storage for users, wallets and transactions (Tasks 2 and 3 and used by Task 4).
#pragma once
#include <string>
#include <vector>
#include <map>
#include "User.h"
#include "Transaction.h"

// UserStore reads and writes the separate user CSV files.
class UserStore
{
public:
    // Constructor takes file paths so storage stays configurable.
    explicit UserStore(std::string usersCsvPath, std::string walletsCsvPath, std::string transactionsCsvPath);

    // Creates CSV files with headers if they are missing.
    bool ensureFiles();
    // Loads all users from users.csv.
    std::vector<User> loadAll() const;

    // Checks whether a username already exists.
    bool usernameExists(const std::string& username) const;
    // Prevents duplicates using full name + email.
    bool personExists(const std::string& fullName, const std::string& email, User& existing) const;

    // Appends a new user record.
    bool addUser(const User& u);
    // Updates the stored password hash.
    bool updatePassword(const std::string& username, std::size_t newHash);

    // Ensures the user has a wallet row.
    bool createWalletFor(const std::string& username);

    // Task 3
    // Loads wallet balances for a user.
    std::map<std::string,double> loadWallet(const std::string& username) const;
    // Applies a balance change and prevents going below zero.
    bool adjustWallet(const std::string& username, const std::string& currency, double delta, double& newBalance);

    // Adds a new transaction log entry.
    bool appendTransaction(const std::string& username, const Transaction& t);
    // Loads the transaction history for a user.
    std::vector<Transaction> loadTransactions(const std::string& username) const;

private:
    std::string usersCsv_;
    std::string walletsCsv_;
    std::string transactionsCsv_;

    // Splits a simple CSV line into fields.
    static std::vector<std::string> splitCsvLine(const std::string& line);
};
