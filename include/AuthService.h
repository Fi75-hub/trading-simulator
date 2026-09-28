// Registration, login, and password changes (Task 2).
#pragma once
#include <string>
#include "User.h"
#include "UserStore.h"

// AuthService provides the Task 2 account flows.
class AuthService
{
public:
    // Uses UserStore for persistence.
    explicit AuthService(UserStore& store);

    // Creates a new account and returns the created user on success.
    bool registerUser(User& outUser);
    // Validates username/password and returns the user on success.
    bool login(User& outUser);
    // Changes the password after verifying the current password.
    bool resetPassword();

private:
    // Reference to the shared storage layer.
    UserStore& store_;
};
