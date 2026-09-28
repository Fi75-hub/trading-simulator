// Registration, login, and password reset logic (Task 2).
#pragma once
#include <string>
#include "User.h"
#include "UserStore.h"

// AuthService provides the Task 2 flows: register, login and reset.
class AuthService
{
public:
    // Uses UserStore for persistence.
    explicit AuthService(UserStore& store);

    // Creates a new account and returns the created user on success.
    bool registerUser(User& outUser);
    // Validates username/password and returns the user on success.
    bool login(User& outUser);
    // Resets the password after verifying the account details.
    bool resetPassword();

private:
    // Reference to the shared storage layer.
    UserStore& store_;
};
