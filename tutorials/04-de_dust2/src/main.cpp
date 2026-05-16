#include "engine.hpp"
#include "tutorial_04.hpp"

int main() {
  Tyra::EngineOptions options;
  options.writeLogsToFile = true;
  options.loadUsbDriver = true;

  Tyra::Engine engine(options);
  Tyra::Tutorial04 game(&engine);
  engine.run(&game);
  return 0;
}
