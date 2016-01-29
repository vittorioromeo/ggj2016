#include <iostream>
#include "base.hpp"
#include "game.hpp"

int main()
{
    using namespace ggj16;

    std::cout << "Hello world!\n";
    boilerplate::app_runner<game_app>{"ggj2016 temp", 320, 240};

    return 0;
}
