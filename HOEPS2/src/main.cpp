#include <tyra>
#include "demo_game.hpp"
#include "game_settings.hpp"

int main() {
  Tyra::EngineOptions options;

  if (Demo::IS_REAL_PS2_VIA_USB) {
    options.writeLogsToFile = true;
    options.loadUsbDriver = true;
  }

  Tyra::Engine engine(options);
  Demo::DemoGame game(&engine);
  engine.run(&game);
  SleepThread();
  return 0;
}
