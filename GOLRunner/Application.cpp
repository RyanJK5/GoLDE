#include <exception>
#include <filesystem>
#include <iostream>

#include "Game.h"

int main()
{
    try
    {
        auto configPath = std::filesystem::path { "config" } / "style.yaml";
        gol::Game game { configPath };
        game.Begin();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Fatal Error:\n" << e.what() << '\n';
        throw;
    }
}