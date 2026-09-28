#include "App.h"
#include <iostream>

int main()
{
    try
    {
        App app;
        app.run();
    }
    catch (const std::ios_base::failure&)
    {
        std::cout << "\nGoodbye.\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << "\n";
        return 1;
    }
    return 0;
}
