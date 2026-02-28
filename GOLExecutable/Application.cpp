#include <exception>
#include <filesystem>
#include <iostream>

#include "Game.h"

int main() {
  const auto configPath = std::filesystem::path{"config"} / "style.yaml";
  gol::Game game{configPath};
  game.Begin();
}