#include <exception>
#include <iostream>

#include "Game.h"

int main()
{
    try
    {
        gol::Game game {};
        game.Begin();
    }
    catch (std::exception e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}