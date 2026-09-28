// Implements registration, login and password reset using UserStore (Task 2).
#include "AuthService.h"
#include "Util.h"
#include <iostream>

AuthService::AuthService(UserStore& store)
: store_(store)
{
}

// Registers a new user, generates a unique 10-digit username and creates a wallet entry.
bool AuthService::registerUser(User& outUser)
{
    std::string fullName = util::readNonEmpty("Full name: ");
    std::string email = util::readNonEmpty("Email: ");
    std::string password = util::readNonEmpty("Password: ");

    User existing;
    if (store_.personExists(fullName, email, existing))
    {
        std::cout << "\nAn account already exists for this name + email.\n";
        std::cout << "Your username is: " << existing.username << "\n\n";
        return false;
    }

    std::string username;
    do
    {
        username = util::generateUsername10();
    } while (store_.usernameExists(username));

    User u;
    u.username = username;
    u.fullName = fullName;
    u.email = email;
    u.passwordHash = util::passwordHash(password);

    if (!store_.addUser(u))
    {
        std::cout << "Could not save user.\n";
        return false;
    }
    store_.createWalletFor(u.username);

    outUser = u;
    std::cout << "\nRegistration successful.\n";
    std::cout << "Your new username is: " << u.username << "\n\n";
    return true;
}

// Logs a user in by checking the stored password hash for the given username.
bool AuthService::login(User& outUser)
{
    std::string username = util::readNonEmpty("Username (10 digits): ");
    std::string password = util::readNonEmpty("Password: ");
    std::size_t h = util::passwordHash(password);

    for (const auto& u : store_.loadAll())
    {
        if (u.username == username && u.passwordHash == h)
        {
            outUser = u;
            std::cout << "\nLogin successful. Welcome, " << u.fullName << ".\n\n";
            return true;
        }
    }
    std::cout << "\nLogin failed. Check your username/password.\n\n";
    return false;
}

// Resets the password after verifying username and email, then updates users.csv with the new hash.
bool AuthService::resetPassword()
{
    std::string username = util::readNonEmpty("Username: ");
    std::string email = util::readNonEmpty("Email: ");

    User found;
    bool ok = false;
    for (const auto& u : store_.loadAll())
    {
        if (u.username == username && u.email == email)
        {
            found = u;
            ok = true;
            break;
        }
    }
    if (!ok)
    {
        std::cout << "No matching account found.\n";
        return false;
    }

    std::string newPassword = util::readNonEmpty("New password: ");
    std::size_t newHash = util::passwordHash(newPassword);

    if (!store_.updatePassword(username, newHash))
    {
        std::cout << "Could not update password.\n";
        return false;
    }

    std::cout << "Password updated.\n";
    return true;
}
